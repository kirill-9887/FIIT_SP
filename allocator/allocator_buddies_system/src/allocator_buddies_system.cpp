#include <not_implemented.h>
#include <cstddef>
#include "../include/allocator_buddies_system.h"
#include <iostream>

allocator_buddies_system::~allocator_buddies_system()
{
    if (!_trusted_memory) {
        return;
    }
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    if (allocator_metadata->ref_counter.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        allocator_metadata->~allocator_metadata_struct();
        if (allocator_metadata->parent_allocator) {
            size_t total_size = allocator_metadata_size + (1ULL << allocator_metadata->order) +
                                (static_cast<int>(allocator_metadata->order) + 1) * sizeof(struct free_block_metadata*);
            allocator_metadata->parent_allocator->deallocate(_trusted_memory, total_size);
        } else {
            ::operator delete(_trusted_memory);
        }
    }
    _trusted_memory = nullptr;
}

allocator_buddies_system::allocator_buddies_system(
    allocator_buddies_system &&other) noexcept
    : _trusted_memory(std::exchange(other._trusted_memory, nullptr))
{
}

allocator_buddies_system &allocator_buddies_system::operator=(
    allocator_buddies_system &&other) noexcept
{
    if (this != &other) {
        allocator_buddies_system temp(std::move(other));
        std::swap(_trusted_memory, temp._trusted_memory);
    }
    return *this;
}

void* allocator_buddies_system::pool_ptr() const noexcept {
    return reinterpret_cast<char*>(_trusted_memory) + allocator_metadata_size;
}

allocator_buddies_system::allocator_buddies_system(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    unsigned char order = __detail::nearest_greater_k_of_2(space_size);
    if (order <= min_k) {
        throw std::logic_error("order < min_k");
    }

    int array_length = static_cast<int>(order) + 1;
    size_t total_size = allocator_metadata_size + (1ULL << order) +
                        array_length * sizeof(struct free_block_metadata*);
    _trusted_memory = parent_allocator ? parent_allocator->allocate(total_size) : ::operator new(total_size);
    struct allocator_metadata_struct* allocator_metadata = new(_trusted_memory) allocator_metadata_struct();
    allocator_metadata->parent_allocator = parent_allocator;
    allocator_metadata->order = order;
    allocator_metadata->mode = allocate_fit_mode;
    
    void* pool_start_addr = static_cast<char*>(_trusted_memory) + allocator_metadata_size;
    auto* free_block = reinterpret_cast<struct free_block_metadata*>(pool_start_addr);
    free_block->occupied = false;
    free_block->size = order;
    free_block->next = nullptr;
    
    auto* array = reinterpret_cast<struct free_block_metadata**>(static_cast<char*>(pool_start_addr) + (1ULL << order));
    for (int i = 0; i < array_length; ++i) {
        array[i] = nullptr;
    }
    array[static_cast<int>(order)] = free_block;
}

[[nodiscard]] void *allocator_buddies_system::do_allocate_sm(size_t size)
{
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);

    int alloc_order = static_cast<int>(allocator_metadata->order);
    size_t max_align = alignof(std::max_align_t);
    size += max_align * ((free_block_metadata_size + max_align - 1) / max_align);
    size_t order = __detail::nearest_greater_k_of_2(size);
    auto* array = reinterpret_cast<struct free_block_metadata**>(static_cast<char*>(pool_ptr()) + (1ULL << alloc_order));
    size_t idx = order;
    struct free_block_metadata* free_block = nullptr;
    while (!free_block) {
        if (idx > alloc_order) {
            throw std::bad_alloc();
        }
        free_block = array[idx];
        ++idx;
    }
    --idx;
    array[idx] = free_block->next;
    while (idx != order) {
        --idx;
        uintptr_t relative_addr = reinterpret_cast<uintptr_t>(free_block) - reinterpret_cast<uintptr_t>(pool_ptr());
        uintptr_t buddy_relative_addr = relative_addr ^ (1ULL << idx);
        void* buddy_address = static_cast<char*>(pool_ptr()) + buddy_relative_addr;

        auto* new_free_block = reinterpret_cast<struct free_block_metadata*>(buddy_address);
        new_free_block->occupied = false;
        new_free_block->size = idx;
        new_free_block->next = array[idx];
        array[idx] = new_free_block;
    }
    free_block->occupied = true;
    free_block->size = order;
    size_t offset = (sizeof(struct free_block_metadata*) + max_align - 1) & ~(max_align - 1);
    return reinterpret_cast<char*>(free_block) + offset;
}

void allocator_buddies_system::do_deallocate_sm(void *at)
{
    if (!at) return;
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);

    int alloc_order = static_cast<int>(allocator_metadata->order);
    size_t max_align = alignof(std::max_align_t);
    size_t offset = (sizeof(struct free_block_metadata) + max_align - 1) & ~(max_align - 1);
    auto* block = reinterpret_cast<struct free_block_metadata*>(static_cast<char*>(at) - offset);
    char* pool_start = static_cast<char*>(pool_ptr());
    if (reinterpret_cast<char*>(block) < pool_start || pool_start + (1ULL << alloc_order) <= reinterpret_cast<char*>(block)) {
        return;
    }
    auto* array = reinterpret_cast<struct free_block_metadata**>(static_cast<char*>(pool_ptr()) + (1ULL << alloc_order));
    while (block->size < alloc_order) {
        uintptr_t relative_addr = reinterpret_cast<uintptr_t>(block) - reinterpret_cast<uintptr_t>(pool_ptr());
        uintptr_t buddy_relative_addr = relative_addr ^ (1ULL << block->size);
        auto* buddy = reinterpret_cast<struct free_block_metadata*>(static_cast<char*>(pool_ptr()) + buddy_relative_addr);
        if (!buddy->occupied && buddy->size == block->size) {
            if (array[buddy->size] == buddy) {
                array[buddy->size] = buddy->next;
            } else {
                auto* prev_block = array[buddy->size];
                for (; prev_block->next != buddy; prev_block = prev_block->next);
                prev_block->next = buddy->next;
            }
            block = block < buddy ? block : buddy;
            block->occupied = false;
            ++block->size;
        } else {
            break;
        }
    }
    block->occupied = false;
    block->next = array[block->size];
    array[block->size] = block;
}

allocator_buddies_system::allocator_buddies_system(const allocator_buddies_system &other)
    : _trusted_memory(other._trusted_memory)
{
    if (_trusted_memory) {
        auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
        ++allocator_metadata->ref_counter;
    }
}

allocator_buddies_system &allocator_buddies_system::operator=(const allocator_buddies_system &other)
{
    if (this != &other) {
        allocator_buddies_system temp(other);
        std::swap(_trusted_memory, temp._trusted_memory);
    }
    return *this;
}

bool allocator_buddies_system::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    auto* other_ptr = dynamic_cast<const allocator_buddies_system*>(&other);
    if (!other_ptr) {
        return false;
    }
    return _trusted_memory == other_ptr->_trusted_memory;
}

inline void allocator_buddies_system::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    auto* allocator_metadata = reinterpret_cast<allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    allocator_metadata->mode = mode;
}


std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    auto* allocator_metadata = reinterpret_cast<allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    try {
        return get_blocks_info_inner();
    } catch (...) {
        return std::vector<allocator_test_utils::block_info>();
    }
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> v;
    for (auto it = begin(); it != end(); ++it) {
        v.push_back({ it.size(), it.occupied() });
    }
    return v;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::begin() const noexcept
{
    return buddy_iterator(pool_ptr());
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::end() const noexcept
{
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    void* end_ptr = static_cast<char*>(pool_ptr()) + (1ULL << allocator_metadata->order);
    return buddy_iterator(end_ptr);
}

bool allocator_buddies_system::buddy_iterator::operator==(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return _block == other._block;
}

bool allocator_buddies_system::buddy_iterator::operator!=(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_buddies_system::buddy_iterator &allocator_buddies_system::buddy_iterator::operator++() & noexcept
{
    if (!_block) {
        return *this;
    }
    _block = static_cast<char*>(_block) + size();
    return *this;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::buddy_iterator::operator++(int n)
{
    buddy_iterator tmp = *this;
    ++*this;
    return tmp;
}

size_t allocator_buddies_system::buddy_iterator::size() const noexcept
{
    return 1ULL << reinterpret_cast<occupied_block_metadata*>(_block)->size;
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept
{
    return reinterpret_cast<occupied_block_metadata*>(_block)->occupied;
}

void *allocator_buddies_system::buddy_iterator::operator*() const noexcept
{
    return _block;
}

allocator_buddies_system::buddy_iterator::buddy_iterator(void *start)
{
    _block = start;
}

allocator_buddies_system::buddy_iterator::buddy_iterator()
{
    _block = nullptr;
}

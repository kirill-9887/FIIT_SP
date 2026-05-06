#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

// size_t allocator_boundary_tags::align_size(size_t size) {
//     return (size + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);
// }

void* allocator_boundary_tags::pool_ptr(void* trusted) {
    return reinterpret_cast<char*>(trusted) + allocator_metadata_size;
}

allocator_boundary_tags::~allocator_boundary_tags()
{
    if (!_trusted_memory) {
        return;
    }
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    if (allocator_metadata->ref_counter.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        allocator_metadata->~allocator_metadata_struct();
        size_t total_size = allocator_metadata_size + allocator_metadata->managered_mem_size;
        allocator_metadata->parent_allocator->deallocate(_trusted_memory, total_size);
    }
    _trusted_memory = nullptr;
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
    : _trusted_memory(std::exchange(other._trusted_memory, nullptr))
{
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    if (this != &other) {
        allocator_boundary_tags temp(std::move(other));
        std::swap(_trusted_memory, temp._trusted_memory);
    }
    return *this;
}


/** If parent_allocator* == nullptr you should use std::pmr::get_default_resource()
 */
allocator_boundary_tags::allocator_boundary_tags(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    // space_size = align_size(space_size);
    if (space_size < occupied_block_metadata_size) {
        throw std::logic_error("Too small space_size.");
    }
    if (!parent_allocator) {
        parent_allocator = std::pmr::get_default_resource();
    }
    size_t total_size = allocator_metadata_size + space_size;
    _trusted_memory = parent_allocator->allocate(total_size);
    struct allocator_metadata_struct* allocator_metadata = new(_trusted_memory) allocator_metadata_struct();
    allocator_metadata->parent_allocator = parent_allocator;
    allocator_metadata->mode = allocate_fit_mode;
    allocator_metadata->managered_mem_size = space_size;
    
    auto* free_block = reinterpret_cast<struct block_metadata_struct*>(pool_ptr(_trusted_memory));
    free_block->set_occupied(false);
    free_block->set_size(space_size - occupied_block_metadata_size);
    free_block->prev = free_block->next = free_block->directly_prev = nullptr;
    
    allocator_metadata->first_free_block = free_block;
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(
    size_t size)
{
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);

    // size = align_size(size);
    struct block_metadata_struct* found_block = nullptr;
    auto* free_block = allocator_metadata->first_free_block;
    switch (allocator_metadata->mode) {
        case allocator_with_fit_mode::fit_mode::first_fit:
            for (; free_block; free_block = free_block->next) {
                if (free_block->size() >= size) {
                    found_block = free_block;
                    break;
                }
            }
            break;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            for (; free_block; free_block = free_block->next) {
                if (free_block->size() >= size && (!found_block || free_block->size() < found_block->size())) {
                    found_block = free_block;
                }
            }
            break;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            for (; free_block; free_block = free_block->next) {
                if (free_block->size() >= size && (!found_block || free_block->size() > found_block->size())) {
                    found_block = free_block;
                }
            }
            break;
    }
    if (!found_block) {
        throw std::bad_alloc();
    }
    if (found_block->size() - size >= occupied_block_metadata_size) {
        auto* new_free_block = reinterpret_cast<struct block_metadata_struct*>(
            reinterpret_cast<char*>(found_block) + occupied_block_metadata_size + size);
        new_free_block->next = found_block->next;
        new_free_block->prev = found_block->prev;
        if (new_free_block->next) new_free_block->next->prev = new_free_block;
        if (new_free_block->prev) new_free_block->prev->next = new_free_block;
        new_free_block->directly_prev = found_block;
        new_free_block->set_size(found_block->size() - size - occupied_block_metadata_size);
        new_free_block->set_occupied(false);
        if (!new_free_block->prev) allocator_metadata->first_free_block = new_free_block;
        found_block->set_size(size);
    } else {
        if (found_block->prev) {
            found_block->prev->next = found_block->next;
        }
        if (found_block->next) {
            free_block->next->prev = found_block->prev;
        }
        if (!found_block->prev) allocator_metadata->first_free_block = found_block->next;
    }
    found_block->set_occupied(true);
    return static_cast<void*>(found_block + 1);
}

void allocator_boundary_tags::do_deallocate_sm(
    void *at)
{
    if (!at) return;
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);

    auto* block = reinterpret_cast<struct block_metadata_struct*>(static_cast<char*>(at) - occupied_block_metadata_size);
    char* pool_start = static_cast<char*>(pool_ptr(_trusted_memory));
    char* end_ptr = pool_start + allocator_metadata->managered_mem_size;
    if (reinterpret_cast<char*>(block) < pool_start || end_ptr <= reinterpret_cast<char*>(block)) {
        return;
    }
    if (block->directly_prev && !block->directly_prev->occupied()) {
        block->directly_prev->set_size(block->directly_prev->size() + occupied_block_metadata_size + block->size());
        block = block->directly_prev;
    }
    char* next_block_addr = reinterpret_cast<char*>(block) + occupied_block_metadata_size + block->size();
    if (next_block_addr < end_ptr) {
        auto* next_block = reinterpret_cast<struct block_metadata_struct*>(next_block_addr);
        if (!next_block->occupied()) {
            block->set_size(block->size() + occupied_block_metadata_size + next_block->size());
            if (next_block->prev != block) {
                block->prev = next_block->prev;
                block->next = next_block->next;
                if (block->prev) block->prev->next = block;
                if (block->next) block->next->prev = block;
            } else {
                block->next = next_block->next;
                if (block->next) block->next->prev = block;
            }
            char* double_next_block_addr = reinterpret_cast<char*>(next_block) + occupied_block_metadata_size + next_block->size();
            if (double_next_block_addr < end_ptr) {
                auto* double_next_block = reinterpret_cast<struct block_metadata_struct*>(double_next_block_addr);
                double_next_block->directly_prev = block;
            }
        } else {
            next_block->directly_prev = block;
        }
    }
    block->set_occupied(false);
    if (!block->prev) allocator_metadata->first_free_block = block;
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    allocator_metadata->mode = mode;
}


std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    try {
        return get_blocks_info_inner();
    } catch (...) {
        return std::vector<allocator_test_utils::block_info>();
    }
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{
    return boundary_iterator(_trusted_memory);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
    return boundary_iterator();
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> v;
    for (auto it = begin(); it != end(); ++it) {
        v.push_back({ it.size(), it.occupied() });
    }
    return v;    
}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)
    : _trusted_memory(other._trusted_memory)
{
    if (_trusted_memory) {
        auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
        ++allocator_metadata->ref_counter;
    }
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    if (this != &other) {
        allocator_boundary_tags temp(other);
        std::swap(_trusted_memory, temp._trusted_memory);
    }
    return *this;
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    auto* other_ptr = dynamic_cast<const allocator_boundary_tags*>(&other);
    if (!other_ptr) {
        return false;
    }
    return _trusted_memory == other_ptr->_trusted_memory;
}

bool allocator_boundary_tags::boundary_iterator::operator==(
        const allocator_boundary_tags::boundary_iterator &other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
        const allocator_boundary_tags::boundary_iterator & other) const noexcept
{
    return !(*this == other);
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    if (!_occupied_ptr) {
        return *this;
    }
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    auto* out_ptr = reinterpret_cast<struct block_metadata_struct*>(
                    reinterpret_cast<char*>(allocator_boundary_tags::pool_ptr(_trusted_memory)) + allocator_metadata->managered_mem_size);
    auto* current_block = reinterpret_cast<struct block_metadata_struct*>(_occupied_ptr);
    size_t full_current_block_size = occupied_block_metadata_size + current_block->size();
    auto* next_block = reinterpret_cast<struct block_metadata_struct*>(
                       reinterpret_cast<char*>(current_block) + full_current_block_size);
    if (next_block >= out_ptr) {
        _occupied_ptr = nullptr;
        _occupied = false;
        return *this;
    }
    _occupied_ptr = next_block;
    _occupied = next_block->occupied();
    return *this;
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    if (!_occupied_ptr) {
        return *this;
    }
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    auto* current_block = reinterpret_cast<struct block_metadata_struct*>(_occupied_ptr);
    auto* prev_block = current_block->directly_prev;
    if (prev_block == nullptr) {
        _occupied_ptr = nullptr;
        _occupied = false;
        return *this;
    }
    _occupied_ptr = prev_block;
    _occupied = prev_block->occupied();
    return *this;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int n)
{
    boundary_iterator tmp = *this;
    ++*this;
    return tmp;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int n)
{
    boundary_iterator tmp = *this;
    --*this;
    return tmp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    return reinterpret_cast<struct block_metadata_struct*>(_occupied_ptr)->size() + occupied_block_metadata_size;
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    return _occupied;
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    return get_ptr();
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
    : _occupied_ptr(nullptr),
      _trusted_memory(nullptr),
      _occupied(false)
{
}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted)
    : _occupied_ptr(reinterpret_cast<struct allocator_metadata_struct*>(trusted) + 1),
      _trusted_memory(trusted)
{
    _occupied = reinterpret_cast<struct block_metadata_struct*>(_occupied_ptr)->occupied();
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}

#include <not_implemented.h>
#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list()
{
    if (!_trusted_memory) {
        return;
    }
    auto allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    if (allocator_metadata->ref_counter.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        allocator_metadata->~allocator_metadata_struct();
        if (allocator_metadata->parent_allocator) {
            size_t total_size = allocator_metadata_size + allocator_metadata->managered_mem_size;
            allocator_metadata->parent_allocator->deallocate(_trusted_memory, total_size);
        } else {
            ::operator delete(_trusted_memory);
        }
    }
    _trusted_memory = nullptr;
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
    : _trusted_memory(std::exchange(other._trusted_memory, nullptr))
{
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    if (this != &other) {
        allocator_sorted_list temp(std::move(other));
        std::swap(_trusted_memory, temp._trusted_memory);
    }
    return *this;
}

allocator_sorted_list::allocator_sorted_list(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (block_metadata_size > space_size) {
        throw std::invalid_argument("space_size must be at least block_metadata_size");
    }
    size_t total_size = allocator_metadata_size + space_size;
    _trusted_memory = parent_allocator ? parent_allocator->allocate(total_size) : ::operator new(total_size);
    struct allocator_metadata_struct* allocator_metadata = new(_trusted_memory) allocator_metadata_struct();

    allocator_metadata->parent_allocator = parent_allocator;
    allocator_metadata->mode = allocate_fit_mode;
    allocator_metadata->managered_mem_size = space_size;
    
    auto block_metadata = reinterpret_cast<struct block_metadata_struct*>(allocator_metadata + 1);
    block_metadata->next_block = nullptr;
    block_metadata->managered_mem_size = space_size - block_metadata_size;
    
    allocator_metadata->list_head = block_metadata;
}

[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(
    size_t size)
{   
    auto allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    struct block_metadata_struct** prev = &allocator_metadata->list_head;
    struct block_metadata_struct** saved_prev = nullptr;
    sorted_free_iterator iterator;
    switch (allocator_metadata->mode) {
        case allocator_with_fit_mode::fit_mode::first_fit:
            for (auto it = free_begin(); it != free_end(); prev = &((*it)->next_block), ++it) {
                if (it.size() >= size) {
                    saved_prev = prev;
                    iterator = it;
                    break;
                }
            }
            break;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            for (auto it = free_begin(); it != free_end(); prev = &((*it)->next_block), ++it) {
                if (it.size() >= size && (!saved_prev || it.size() < iterator.size())) {
                    saved_prev = prev;
                    iterator = it;
                }
            }
            break;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            for (auto it = free_begin(); it != free_end(); prev = &((*it)->next_block), ++it) {
                if (it.size() >= size && (!saved_prev || it.size() > iterator.size())) {
                    saved_prev = prev;
                    iterator = it;
                }
            }
            break;
    }
    if (!saved_prev) {
        throw std::bad_alloc();
    }
    auto found_block = *iterator;
    size_t cur_managered_mem_size = found_block->managered_mem_size;
    struct block_metadata_struct* new_free_block = nullptr;
    if (cur_managered_mem_size - size > block_metadata_size) {
        new_free_block = reinterpret_cast<struct block_metadata_struct*>(
            reinterpret_cast<char*>(found_block) + block_metadata_size + size);
        new_free_block->next_block = found_block->next_block;
        new_free_block->managered_mem_size = cur_managered_mem_size - size - block_metadata_size;
        found_block->managered_mem_size = size;
    } else {
        new_free_block = found_block->next_block;
    }
    *saved_prev = new_free_block;
    return static_cast<void*>(found_block + 1);
}

allocator_sorted_list::allocator_sorted_list(const allocator_sorted_list &other)
    : _trusted_memory(other._trusted_memory)
{
    if (_trusted_memory) {
        auto allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
        ++allocator_metadata->ref_counter;
    }
}

allocator_sorted_list &allocator_sorted_list::operator=(const allocator_sorted_list &other)
{
    if (this != &other) {
        allocator_sorted_list temp(other);
        std::swap(_trusted_memory, temp._trusted_memory);
    }
    return *this;
}

bool allocator_sorted_list::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    auto other_ptr = dynamic_cast<const allocator_sorted_list*>(&other);
    if (!other_ptr) {
        return false;
    }
    return _trusted_memory == other_ptr->_trusted_memory;
}

void allocator_sorted_list::do_deallocate_sm(
    void *at)
{
    auto allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    auto cur_block = reinterpret_cast<struct block_metadata_struct*>(static_cast<char*>(at) - block_metadata_size);
    auto head_ptr_ptr = &(allocator_metadata->list_head);
    if (auto it = free_begin(); it != free_end()) {
        auto free_block = *it;
        if (cur_block < free_block) {
            if (reinterpret_cast<char*>(cur_block) + block_metadata_size + 
                    cur_block->managered_mem_size == reinterpret_cast<char*>(free_block)) {
                cur_block->next_block = free_block->next_block;
                size_t united_size = cur_block->managered_mem_size + 
                    block_metadata_size + free_block->managered_mem_size;
                cur_block->managered_mem_size = united_size;
            } else {
                cur_block->next_block = free_block;
            }
            *head_ptr_ptr = cur_block;
            return;
        }
    }
    for (auto it = free_begin(); it != free_end(); ++it) {
        auto some_free_block = *it;
        auto next_free_block = some_free_block->next_block;
        if (some_free_block < cur_block && (!next_free_block || cur_block < next_free_block)) {
            if (reinterpret_cast<char*>(some_free_block) + block_metadata_size + 
                    some_free_block->managered_mem_size == reinterpret_cast<char*>(cur_block)) {
                size_t united_size = some_free_block->managered_mem_size + block_metadata_size + 
                    cur_block->managered_mem_size;
                some_free_block->managered_mem_size = united_size;
                cur_block = some_free_block;
            } else {
                some_free_block->next_block = cur_block;
            }
            if (next_free_block && reinterpret_cast<char*>(cur_block) + block_metadata_size + 
                    cur_block->managered_mem_size == reinterpret_cast<char*>(next_free_block)) {
                cur_block->next_block = next_free_block->next_block;
                size_t united_size = cur_block->managered_mem_size + block_metadata_size + 
                    next_free_block->managered_mem_size;
                cur_block->managered_mem_size = united_size;
            } else {
                cur_block->next_block = next_free_block;
            }
            return;
        }
    }
    *head_ptr_ptr = cur_block;
    cur_block->next_block = nullptr;
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    auto allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    allocator_metadata->mode = mode;
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
    auto allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    try {
        return get_blocks_info_inner();
    } catch (...) {
        return std::vector<allocator_test_utils::block_info>();
    }
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> v;
    for (auto it = begin(); it != end(); ++it) {
        v.push_back({ it.size(), it.occupied() });
    }
    return v;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_begin() const noexcept
{
    return sorted_free_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_end() const noexcept
{
    return sorted_free_iterator();
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::begin() const noexcept
{
    return sorted_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::end() const noexcept
{
    return sorted_iterator();
}


bool allocator_sorted_list::sorted_free_iterator::operator==(
        const allocator_sorted_list::sorted_free_iterator & other) const noexcept
{
    return _free_ptr == other._free_ptr;
}

bool allocator_sorted_list::sorted_free_iterator::operator!=(
        const allocator_sorted_list::sorted_free_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_sorted_list::sorted_free_iterator &allocator_sorted_list::sorted_free_iterator::operator++() & noexcept
{
    if (!_free_ptr) {
        return *this;
    }
    _free_ptr = _free_ptr->next_block;
    return *this;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::sorted_free_iterator::operator++(int n)
{
    sorted_free_iterator tmp = *this;
    ++*this;
    return tmp;
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept
{
    if (_free_ptr == nullptr) {
        return 0;
    }
    return _free_ptr->managered_mem_size;
}

allocator_sorted_list::block_metadata_struct *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept
{
    return _free_ptr;
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator()
    : _free_ptr(nullptr)
{
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(void *trusted)
    : sorted_free_iterator()
{
    if (trusted == nullptr) {
        return;
    }
    auto allocator_metadata = static_cast<struct allocator_metadata_struct*>(trusted);
    _free_ptr = allocator_metadata->list_head;
}

bool allocator_sorted_list::sorted_iterator::operator==(const allocator_sorted_list::sorted_iterator & other) const noexcept
{
    return _current_ptr == other._current_ptr;
}

bool allocator_sorted_list::sorted_iterator::operator!=(const allocator_sorted_list::sorted_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_sorted_list::sorted_iterator &allocator_sorted_list::sorted_iterator::operator++() & noexcept
{
    if (!_current_ptr) {
        return *this;
    }
    auto out_ptr = reinterpret_cast<struct block_metadata_struct*>(
        reinterpret_cast<char*>(_trusted_memory + 1) + _trusted_memory->managered_mem_size);
    _current_ptr = reinterpret_cast<struct block_metadata_struct*>(
        reinterpret_cast<char*>(_current_ptr + 1) + _current_ptr->managered_mem_size);
    if (_current_ptr == out_ptr) {
        _current_ptr = nullptr;
        _free_ptr = nullptr;
        return *this;
    }
    while (_free_ptr && _free_ptr < _current_ptr) {
        _free_ptr = _free_ptr->next_block;
    }
    return *this;
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::sorted_iterator::operator++(int n)
{
    allocator_sorted_list::sorted_iterator tmp = *this;
    ++*this;
    return tmp;
}

size_t allocator_sorted_list::sorted_iterator::size() const noexcept
{
    if (_current_ptr == nullptr) {
        return 0;
    }
    return _current_ptr->managered_mem_size;
}

allocator_sorted_list::block_metadata_struct *allocator_sorted_list::sorted_iterator::operator*() const noexcept
{
    return _current_ptr;
}

allocator_sorted_list::sorted_iterator::sorted_iterator()
    : _free_ptr(nullptr),
    _current_ptr(nullptr),
    _trusted_memory(nullptr)
{
}

allocator_sorted_list::sorted_iterator::sorted_iterator(void *trusted)
    : sorted_iterator()
{
    if (trusted == nullptr) {
        return;
    }
    _trusted_memory = reinterpret_cast<struct allocator_metadata_struct*>(trusted);
    _free_ptr = _trusted_memory->list_head;
    _current_ptr = reinterpret_cast<struct block_metadata_struct*>(_trusted_memory + 1);
}

bool allocator_sorted_list::sorted_iterator::occupied() const noexcept
{
    return _current_ptr && _current_ptr != _free_ptr;
}
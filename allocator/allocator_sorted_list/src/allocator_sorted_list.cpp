#include <not_implemented.h>
#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list()
{
    if (!_trusted_memory) {
        return;
    }
    auto ref_count_ptr = reinterpret_cast<std::atomic<size_t>*>(_trusted_memory);
    size_t ref_count = --(*ref_count_ptr);
    if (ref_count == 0) {
        char* mem_ptr = static_cast<char*>(_trusted_memory);
        auto mutex_ptr = reinterpret_cast<std::mutex*>(
            mem_ptr +
            sizeof(std::atomic<size_t>) +
            sizeof(std::pmr::memory_resource *) +
            sizeof(fit_mode) +
            sizeof(size_t)
        );
        mutex_ptr->~mutex();
        ref_count_ptr->~atomic();
        ::operator delete(_trusted_memory);
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
    _trusted_memory = ::operator new(allocator_metadata_size + space_size);
    char *_mem_ptr = static_cast<char*>(_trusted_memory);
    new (_mem_ptr) std::atomic<size_t>(1);
    size_t offset = sizeof(std::atomic<size_t>);
    *reinterpret_cast<std::pmr::memory_resource**>(_mem_ptr + offset) = parent_allocator;
    set_fit_mode(allocate_fit_mode);
    offset += sizeof(std::pmr::memory_resource*) + sizeof(fit_mode);
    *reinterpret_cast<size_t*>(_mem_ptr + offset) = space_size;
    offset += sizeof(size_t);
    new (_mem_ptr + offset) std::mutex();
    offset += sizeof(std::mutex);
    void** sorted_list_head_ptr = reinterpret_cast<void**>(_mem_ptr + offset);
    
    char* sorted_list_head = _mem_ptr + allocator_metadata_size;
    *reinterpret_cast<void**>(sorted_list_head) = nullptr;
    *reinterpret_cast<size_t*>(sorted_list_head + sizeof(void*)) = space_size - block_metadata_size;

    *sorted_list_head_ptr = static_cast<void*>(sorted_list_head);
}

[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(
    size_t size)
{   
    char* _mem_ptr = static_cast<char*>(_trusted_memory);
    auto mutex_ptr = reinterpret_cast<std::mutex*>(
        _mem_ptr +
        sizeof(std::atomic<size_t>) +
        sizeof(std::pmr::memory_resource *) +
        sizeof(fit_mode) +
        sizeof(size_t)
    );
    std::lock_guard<std::mutex> lock(*mutex_ptr);
    allocator_with_fit_mode::fit_mode mode = *reinterpret_cast<fit_mode*>(
        _mem_ptr +
        sizeof(std::atomic<size_t>) +
        sizeof(std::pmr::memory_resource *)
    );
    void** prev = reinterpret_cast<void**>(
        _mem_ptr +
        sizeof(std::atomic<size_t>) +
        sizeof(std::pmr::memory_resource *) +
        sizeof(fit_mode) +
        sizeof(size_t) +
        sizeof(std::mutex)
    );
    allocator_sorted_list::sorted_free_iterator iterator;
    void** saved_prev;
    bool iter_is_free = true;
    switch (mode) {
        case allocator_with_fit_mode::fit_mode::first_fit:
            for (auto it = free_begin(); it != free_end(); prev = static_cast<void**>(*it), ++it) {
                if (it.size() >= size) {
                    saved_prev = prev;
                    iterator = it;
                    iter_is_free = false;
                    break;
                }
            }
            break;
        case allocator_with_fit_mode::fit_mode::the_best_fit:
            for (auto it = free_begin(); it != free_end(); prev = static_cast<void**>(*it), ++it) {
                if (it.size() >= size && (iter_is_free || it.size() < iterator.size())) {
                    saved_prev = prev;
                    iterator = it;
                    iter_is_free = false;
                }
            }
            break;
        case allocator_with_fit_mode::fit_mode::the_worst_fit:
            for (auto it = free_begin(); it != free_end(); prev = static_cast<void**>(*it), ++it) {
                if (it.size() >= size && (iter_is_free || it.size() > iterator.size())) {
                    saved_prev = prev;
                    iterator = it;
                    iter_is_free = false;
                }
            }
            break;
    }
    if (iter_is_free) {
        throw std::bad_alloc();
    }
    char* cur_ptr = static_cast<char*>(*iterator);
    char* new_free_ptr = nullptr;
    if (iterator.size() - size > block_metadata_size) {
        new_free_ptr = cur_ptr + block_metadata_size + size;
        *reinterpret_cast<void**>(new_free_ptr) = *reinterpret_cast<void**>(cur_ptr);
        *reinterpret_cast<size_t*>(new_free_ptr + sizeof(void*)) = iterator.size() - size - block_metadata_size;
        *reinterpret_cast<size_t*>(cur_ptr + sizeof(void*)) = size;
    } else {
        new_free_ptr = static_cast<char*>(*reinterpret_cast<void**>(*iterator));
    }
    *saved_prev = static_cast<void*>(new_free_ptr);
    return static_cast<void*>(cur_ptr + block_metadata_size);
}

allocator_sorted_list::allocator_sorted_list(const allocator_sorted_list &other)
    : _trusted_memory(other._trusted_memory)
{
    if (_trusted_memory) {
        auto ref_count_ptr = reinterpret_cast<std::atomic<size_t>*>(_trusted_memory);
        ++(*ref_count_ptr);
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
    char* _mem_ptr = static_cast<char*>(_trusted_memory);
    auto mutex_ptr = reinterpret_cast<std::mutex*>(
        _mem_ptr +
        sizeof(std::atomic<size_t>) +
        sizeof(std::pmr::memory_resource *) +
        sizeof(fit_mode) +
        sizeof(size_t)
    );
    std::lock_guard<std::mutex> lock(*mutex_ptr);
    char* cur_block_ptr = static_cast<char*>(at) - block_metadata_size;
    size_t cur_block_size = *reinterpret_cast<size_t*>(cur_block_ptr + sizeof(void*));
    void** head_ptr_ptr = reinterpret_cast<void**>(
        _mem_ptr +
        sizeof(std::atomic<size_t>) +
        sizeof(std::pmr::memory_resource *) +
        sizeof(fit_mode) +
        sizeof(size_t) +
        sizeof(std::mutex)
    );
    if (auto it = free_begin(); it != free_end()) {
        char* free_ptr = static_cast<char*>(*it);
        size_t free_size = it.size();
        if (cur_block_ptr < free_ptr) {
            if (cur_block_ptr + block_metadata_size + cur_block_size == free_ptr) {
                *reinterpret_cast<void**>(cur_block_ptr) = *reinterpret_cast<void**>(free_ptr);
                size_t united_size = cur_block_size + block_metadata_size + free_size;
                *reinterpret_cast<size_t*>(cur_block_ptr + sizeof(void*)) = united_size;
            } else {
                *reinterpret_cast<void**>(cur_block_ptr) = static_cast<void*>(free_ptr);
            }
            *head_ptr_ptr = static_cast<void*>(cur_block_ptr);
            return;
        }
    }
    for (auto it = free_begin(); it != free_end(); ++it) {
        char* some_free_ptr = static_cast<char*>(*it);
        char* next_free_ptr = static_cast<char*>(*reinterpret_cast<void**>(*it));
        size_t some_free_size = it.size();
        if (some_free_ptr < cur_block_ptr && (!next_free_ptr || cur_block_ptr < next_free_ptr)) {
            if (some_free_ptr + block_metadata_size + some_free_size == cur_block_ptr) {
                size_t united_size = some_free_size + block_metadata_size + cur_block_size;
                *reinterpret_cast<size_t*>(some_free_ptr + sizeof(void*)) = united_size;
                cur_block_ptr = some_free_ptr;
                cur_block_size = united_size;
            } else {
                *reinterpret_cast<void**>(some_free_ptr) = static_cast<void*>(cur_block_ptr);
            }
            if (next_free_ptr && cur_block_ptr + block_metadata_size + cur_block_size == next_free_ptr) {
                *reinterpret_cast<void**>(cur_block_ptr) = *reinterpret_cast<void**>(next_free_ptr);
                size_t united_size = cur_block_size + block_metadata_size + *reinterpret_cast<size_t*>(next_free_ptr + sizeof(void*));
                *reinterpret_cast<size_t*>(cur_block_ptr + sizeof(void*)) = united_size;
            } else {
                *reinterpret_cast<void**>(cur_block_ptr) = static_cast<void*>(next_free_ptr);
            }
            return;
        }
    }
    *head_ptr_ptr = static_cast<void*>(cur_block_ptr);
    *reinterpret_cast<void**>(cur_block_ptr) = nullptr;
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    char *_mem_ptr = static_cast<char*>(_trusted_memory);
    size_t offset = (
        sizeof(std::atomic<size_t>) +
        sizeof(std::pmr::memory_resource *)
    );
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(_mem_ptr + offset) = mode;
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
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
    return allocator_sorted_list::sorted_free_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_end() const noexcept
{
    return allocator_sorted_list::sorted_free_iterator();
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::begin() const noexcept
{
    return allocator_sorted_list::sorted_iterator(_trusted_memory);
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::end() const noexcept
{
    return allocator_sorted_list::sorted_iterator();
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
    _free_ptr = *reinterpret_cast<void**>(_free_ptr);
    return *this;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::sorted_free_iterator::operator++(int n)
{
    allocator_sorted_list::sorted_free_iterator tmp = *this;
    ++*this;
    return tmp;
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept
{
    if (_free_ptr == nullptr) {
        return 0;
    }
    return *reinterpret_cast<size_t*>(static_cast<char*>(_free_ptr) + sizeof(void*));
}

void *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept
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
    char *_mem_ptr = static_cast<char*>(trusted);
    _free_ptr = *reinterpret_cast<void**>(
        _mem_ptr +
        sizeof(std::atomic<size_t>) +
        sizeof(std::pmr::memory_resource *) +
        sizeof(fit_mode) +
        sizeof(size_t) +
        sizeof(std::mutex)
    );
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
    size_t all_memory_size = *reinterpret_cast<size_t*>(static_cast<char*>(_trusted_memory) +
                                                        sizeof(std::atomic<size_t>) +
                                                        sizeof(std::pmr::memory_resource *) +
                                                        sizeof(fit_mode));
    char* out_ptr = static_cast<char*>(_trusted_memory) + allocator_metadata_size + all_memory_size;
    size_t current_size = *reinterpret_cast<size_t*>(static_cast<char*>(_current_ptr) + sizeof(void*));
    _current_ptr = static_cast<void*>(static_cast<char*>(_current_ptr) + block_metadata_size + current_size);
    if (_current_ptr == out_ptr) {
        _current_ptr = nullptr;
        _free_ptr = nullptr;
        return *this;
    }
    while (_free_ptr && _free_ptr < _current_ptr) {
        _free_ptr = *reinterpret_cast<void**>(_free_ptr);
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
    return *reinterpret_cast<size_t*>(static_cast<char*>(_current_ptr) + sizeof(void*));
}

void *allocator_sorted_list::sorted_iterator::operator*() const noexcept
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
    _trusted_memory = trusted;
    _free_ptr = *reinterpret_cast<void**>(static_cast<char*>(_trusted_memory) +
                                         sizeof(std::atomic<size_t>) +
                                         sizeof(std::pmr::memory_resource *) +
                                         sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex));
    _current_ptr = static_cast<void*>(static_cast<char*>(trusted) + allocator_metadata_size);
}

bool allocator_sorted_list::sorted_iterator::occupied() const noexcept
{
    return _current_ptr && _current_ptr != _free_ptr;
}
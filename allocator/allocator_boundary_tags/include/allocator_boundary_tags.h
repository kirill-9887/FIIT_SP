#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H

#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <pp_allocator.h>
#include <iterator>
#include <mutex>
#include <atomic>

class allocator_boundary_tags final :
    public smart_mem_resource,
    public allocator_test_utils,
    public allocator_with_fit_mode
{

private:

    struct block_metadata_struct {
        size_t size_and_occupied;
        struct block_metadata_struct* prev;
        struct block_metadata_struct* next;
        struct block_metadata_struct* directly_prev;

        size_t size() const {
            return size_and_occupied & ~size_t(1);
        }

        void set_size(size_t size) {
            size_and_occupied = size & ~size_t(1) | size_and_occupied & size_t(1);
        }

        bool occupied() const {
            return size_and_occupied & size_t(1);
        }

        void set_occupied(bool occupied) {
            size_t occupied_bit = occupied ? size_t(1) : 0;
            size_and_occupied = size_and_occupied & ~size_t(1) | occupied_bit;
        }
    };

    struct allocator_metadata_struct {
        std::mutex mutex;
        std::atomic<size_t> ref_counter;
        std::pmr::memory_resource* parent_allocator;
        fit_mode mode;
        size_t managered_mem_size;
        struct block_metadata_struct* first_free_block;

        allocator_metadata_struct() : ref_counter(1) {}
    };

    static constexpr const size_t allocator_metadata_size =
        (sizeof(struct allocator_metadata_struct) + alignof(std::max_align_t) - 1) & ~(alignof(std::max_align_t) - 1);
    // static constexpr const size_t allocator_metadata_size = sizeof(memory_resource*) + sizeof(allocator_with_fit_mode::fit_mode) +
    //                                                         sizeof(size_t) + sizeof(std::mutex) + sizeof(void*);

    static constexpr const size_t occupied_block_metadata_size = sizeof(block_metadata_struct);
    // static constexpr const size_t occupied_block_metadata_size = sizeof(size_t) + sizeof(void*) + sizeof(void*) + sizeof(void*);

    static constexpr const size_t free_block_metadata_size = 0;

    void *_trusted_memory;

    static size_t align_size(size_t size);

    static void* pool_ptr(void* trusted);

public:
    
    ~allocator_boundary_tags() override;
    
    allocator_boundary_tags(allocator_boundary_tags const &other);
    
    allocator_boundary_tags &operator=(allocator_boundary_tags const &other);
    
    allocator_boundary_tags(
        allocator_boundary_tags &&other) noexcept;
    
    allocator_boundary_tags &operator=(
        allocator_boundary_tags &&other) noexcept;

public:
    
    explicit allocator_boundary_tags(
            size_t space_size,
            std::pmr::memory_resource *parent_allocator = nullptr,
            allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

private:
    
    [[nodiscard]] void *do_allocate_sm(
        size_t bytes) override;
    
    void do_deallocate_sm(
        void *at) override;

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override;

public:
    
    inline void set_fit_mode(
        allocator_with_fit_mode::fit_mode mode) override;

public:
    
    std::vector<allocator_test_utils::block_info> get_blocks_info() const override;

private:

    std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const override;

/** TODO: Highly recommended for helper functions to return references */

    class boundary_iterator
    {
        void* _occupied_ptr;
        bool _occupied;
        void* _trusted_memory;

    public:

        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = void*;
        using reference = void*&;
        using pointer = void**;
        using difference_type = ptrdiff_t;

        bool operator==(const boundary_iterator&) const noexcept;

        bool operator!=(const boundary_iterator&) const noexcept;

        boundary_iterator& operator++() & noexcept;

        boundary_iterator& operator--() & noexcept;

        boundary_iterator operator++(int n);

        boundary_iterator operator--(int n);

        size_t size() const noexcept;

        bool occupied() const noexcept;

        void* operator*() const noexcept;

        void* get_ptr() const noexcept;

        boundary_iterator();

        boundary_iterator(void* trusted);
    };

    friend class boundary_iterator;

    boundary_iterator begin() const noexcept;

    boundary_iterator end() const noexcept;
};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_BOUNDARY_TAGS_H
#ifndef MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_RED_BLACK_TREE_H
#define MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_RED_BLACK_TREE_H

#include <pp_allocator.h>
#include <allocator_test_utils.h>
#include <allocator_with_fit_mode.h>
#include <mutex>
#include <atomic>
#include <cassert>

class allocator_red_black_tree final:
    public smart_mem_resource,
    public allocator_test_utils,
    public allocator_with_fit_mode
{

private:
    enum class block_color : unsigned char
    { RED, BLACK };

    struct block_data
    {
        bool occupied : 4;
        block_color color : 4;
    };

    void *_trusted_memory;

    struct list_block : block_data
    {
        list_block* prev;
        list_block* next;
    };

    struct occupied_block_metadata_struct : list_block
    {
        // void* ptr_3;
    };
    
    struct free_block_metadata_struct : list_block
    {
        free_block_metadata_struct* parent;
        free_block_metadata_struct* left;
        free_block_metadata_struct* right;

        bool is_left_child() const {
            if (!parent) {
                assert(false && "parent == nullptr.");
            }
            return this == parent->left;
        }

        bool is_right_child() const {
            if (!parent) {
                assert(false && "parent == nullptr.");
            }
            return this == parent->right;
        }
    };

    struct allocator_metadata_struct
    {
        std::mutex mutex;
        std::atomic<size_t> ref_counter;
        allocator_dbg_helper* alloc_dbg_helper_ptr = nullptr;
        std::pmr::memory_resource* parent_allocator = nullptr;
        fit_mode mode;
        size_t pool_size = 0;
        free_block_metadata_struct* root = nullptr;
    };

    // static constexpr const size_t allocator_metadata_size = sizeof(allocator_dbg_helper*) + sizeof(fit_mode) + sizeof(size_t) + sizeof(std::mutex) + sizeof(void*);
    static constexpr const size_t allocator_metadata_size = sizeof(allocator_metadata_struct);

    // static constexpr const size_t occupied_block_metadata_size = sizeof(block_data) + 3 * sizeof(void*);
    static constexpr const size_t occupied_block_metadata_size = sizeof(occupied_block_metadata_struct);

    // static constexpr const size_t free_block_metadata_size = sizeof(block_data) + 5 * sizeof(void*);
    static constexpr const size_t free_block_metadata_size = sizeof(free_block_metadata_struct);

public:
    
    ~allocator_red_black_tree() override;
    
    allocator_red_black_tree(
        allocator_red_black_tree const &other);
    
    allocator_red_black_tree &operator=(
        allocator_red_black_tree const &other);
    
    allocator_red_black_tree(
        allocator_red_black_tree &&other) noexcept;
    
    allocator_red_black_tree &operator=(
        allocator_red_black_tree &&other) noexcept;

public:
    
    explicit allocator_red_black_tree(
            size_t space_size,
            std::pmr::memory_resource *parent_allocator = nullptr,
            allocator_with_fit_mode::fit_mode allocate_fit_mode = allocator_with_fit_mode::fit_mode::first_fit);

private:
    
    [[nodiscard]] void *do_allocate_sm(
        size_t size) override;
    
    void do_deallocate_sm(
        void *at) override;

    bool do_is_equal(const std::pmr::memory_resource&) const noexcept override;

    std::vector<allocator_test_utils::block_info> get_blocks_info() const override;
    
    inline void set_fit_mode(allocator_with_fit_mode::fit_mode mode) override;

    static void* pool_ptr(void* trusted);

    static size_t block_size(void* trusted, void* block_ptr);

    static struct list_block* get_head(void* trusted);

    void transpant(free_block_metadata_struct* node, free_block_metadata_struct* child);

    void rotate_left(free_block_metadata_struct* x);

    void rotate_right(free_block_metadata_struct* y);

    void insert_in_red_black_tree(free_block_metadata_struct* node);

    void remove_from_red_black_tree(free_block_metadata_struct* node);
    
public:
    void dump_allocator_state();

private:

    std::vector<allocator_test_utils::block_info> get_blocks_info_inner() const override;

    class rb_iterator
    {
        void* _trusted;
        list_block* _block_ptr;

    public:

        using iterator_category = std::forward_iterator_tag;
        using value_type = void*;
        using reference = void*&;
        using pointer = void**;
        using difference_type = ptrdiff_t;

        bool operator==(const rb_iterator&) const noexcept;

        bool operator!=(const rb_iterator&) const noexcept;

        rb_iterator& operator++() & noexcept;

        rb_iterator operator++(int n);

        size_t size() const noexcept;

        void* operator*() const noexcept;

        bool occupied() const noexcept;

        rb_iterator();

        rb_iterator(void* trusted);
    };

    friend class rb_iterator;

    rb_iterator begin() const noexcept;
    rb_iterator end() const noexcept;

};

#endif //MATH_PRACTICE_AND_OPERATING_SYSTEMS_ALLOCATOR_ALLOCATOR_RED_BLACK_TREE_H
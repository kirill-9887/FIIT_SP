#include <not_implemented.h>

#include "../include/allocator_red_black_tree.h"
#include <iomanip>

void* allocator_red_black_tree::pool_ptr(void* trusted) {
    return reinterpret_cast<char*>(trusted) + allocator_metadata_size;
}

size_t allocator_red_black_tree::block_size(void* trusted, void* block_ptr) {
    auto* block = reinterpret_cast<list_block*>(block_ptr);
    if (block->next) {
        return reinterpret_cast<char*>(block->next) - reinterpret_cast<char*>(block);
    }
    auto* allocator_matadata = reinterpret_cast<struct allocator_metadata_struct*>(trusted);
    auto pool_size = allocator_matadata->pool_size;
    return reinterpret_cast<char*>(pool_ptr(trusted)) + pool_size - reinterpret_cast<char*>(block);
}

allocator_red_black_tree::list_block* allocator_red_black_tree::get_head(void* trusted) {
    return reinterpret_cast<struct list_block*>(reinterpret_cast<struct allocator_metadata_struct*>(trusted) + 1);
}

allocator_red_black_tree::~allocator_red_black_tree()
{
    if (!_trusted_memory) {
        return;
    }
    auto* allocator_metadata = reinterpret_cast<allocator_metadata_struct*>(_trusted_memory);
    if (allocator_metadata->ref_counter.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        allocator_metadata->~allocator_metadata_struct();
        size_t total_size = allocator_metadata_size + allocator_metadata->pool_size;
        allocator_metadata->parent_allocator->deallocate(_trusted_memory, total_size);
    }
    _trusted_memory = nullptr;
}

allocator_red_black_tree::allocator_red_black_tree(
    allocator_red_black_tree &&other) noexcept
    : _trusted_memory(std::exchange(other._trusted_memory, nullptr))
{
}

allocator_red_black_tree &allocator_red_black_tree::operator=(
    allocator_red_black_tree &&other) noexcept
{
    if (this != &other) {
        allocator_red_black_tree temp(std::move(other));
        std::swap(_trusted_memory, temp._trusted_memory);
    }
    return *this;
}

allocator_red_black_tree::allocator_red_black_tree(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size < free_block_metadata_size) {
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
    allocator_metadata->pool_size = space_size;
    
    auto* free_block = reinterpret_cast<free_block_metadata_struct*>(pool_ptr(_trusted_memory));
    free_block->occupied = false;
    free_block->color = block_color::RED;
    free_block->prev = free_block->next = nullptr;
    free_block->parent = free_block->left = free_block->right = nullptr;
    
    allocator_metadata->root = free_block;
}

allocator_red_black_tree::allocator_red_black_tree(const allocator_red_black_tree &other)
    : _trusted_memory(other._trusted_memory)
{
    if (_trusted_memory) {
        auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
        ++allocator_metadata->ref_counter;
    }
}

allocator_red_black_tree &allocator_red_black_tree::operator=(const allocator_red_black_tree &other)
{
    if (this != &other) {
        allocator_red_black_tree temp(other);
        std::swap(_trusted_memory, temp._trusted_memory);
    }
    return *this;
}

bool allocator_red_black_tree::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    auto* other_ptr = dynamic_cast<const allocator_red_black_tree*>(&other);
    if (!other_ptr) {
        return false;
    }
    return _trusted_memory == other_ptr->_trusted_memory;
}

void allocator_red_black_tree::transpant(free_block_metadata_struct* node, free_block_metadata_struct* child) {
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    if (node->parent) {
        if (node->is_left_child()) {
            node->parent->left = child;
        } else if (node->is_right_child()) {
            node->parent->right = child;
        }
    } else {
        allocator_metadata->root = child;
    }
    if (child) {
        child->parent = node->parent;
    }
    node->parent = node->right = node->left = nullptr;
}

void allocator_red_black_tree::rotate_left(free_block_metadata_struct* x) {
    if (x == nullptr) {
        return;
    }
    if (x->right == nullptr) {
        assert(false && "x->right == nullptr");
    }
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    auto* y = x->right;
    x->right = y->left;
    y->left = x;

    if (x->right) {
        x->right->parent = x;
    }
    if (!x->parent) {
        allocator_metadata->root = y;
    } else {
        if (x->is_left_child()) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
    }
    y->parent = x->parent;
    x->parent = y;
}

void allocator_red_black_tree::rotate_right(free_block_metadata_struct* y) {
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    if (y == nullptr) {
        return;
    }
    if (y->left == nullptr) {
        assert(false && "y->left == nullptr");
    }
    auto* x = y->left;
    y->left = x->right;
    x->right = y;

    if (y->left) {
        y->left->parent = y;
    }
    if (!y->parent) {
        allocator_metadata->root = x;
    } else {
        if (y->is_right_child()) {
            y->parent->right = x;
        } else {
            y->parent->left = x;
        }
    }
    x->parent = y->parent;
    y->parent = x;
}

void allocator_red_black_tree::remove_from_red_black_tree(free_block_metadata_struct* node) {
    node->occupied = true;
    free_block_metadata_struct* new_child;
    auto* parent = node->parent;
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    if (!node->left && !node->right) {
        new_child = nullptr;
    } else if (!node->left) {
        new_child = node->right;
    } else if (!node->right) {
        new_child = node->left;
    } else {
        new_child = node->right;
        while (new_child->left) {
            new_child = new_child->left;
        }
        transpant(new_child, new_child->right);
        new_child->right = node->right;
        if (new_child->right) {
            new_child->right->parent = new_child;
        }
        new_child->left = node->left;
        if (new_child->left) {
            new_child->left->parent = new_child;
        }
    }
    transpant(node, new_child);
    
    if (!new_child || new_child->color == block_color::BLACK) {
        if (!new_child) {
            new_child = parent;
        }
        while (new_child && new_child->color == block_color::BLACK && new_child->parent != nullptr) {
            auto* p = new_child->parent;
            auto* sibling = new_child->is_left_child() ? p->right : p->left;
            if (sibling == nullptr) {
                break;
            }
            if (new_child->is_left_child()) {
                if (sibling->color == block_color::RED) {
                    sibling->color = block_color::BLACK;
                    p->color = block_color::RED;
                    rotate_left(p);
                    sibling = p->right;
                }
                if (sibling == nullptr) {
                    new_child = p;
                    continue;
                }
                if ((!sibling->left || sibling->left->color == block_color::BLACK) &&
                        (!sibling->right || sibling->right->color == block_color::BLACK)) {
                    sibling->color = block_color::RED;
                    new_child = p;
                } else {
                    if (!sibling->right || sibling->right->color == block_color::BLACK) {
                        if (sibling->left) {
                            sibling->left->color = block_color::BLACK;
                        }
                        sibling->color = block_color::RED;
                        rotate_right(sibling);
                    }
                    sibling->color = p->color;
                    p->color = block_color::BLACK;
                    if (sibling->right) {
                        sibling->right->color = block_color::BLACK;
                    }
                    rotate_left(p);
                    new_child = allocator_metadata->root;
                }
            } else {
                if (sibling->color == block_color::RED) {
                    sibling->color = block_color::BLACK;
                    p->color = block_color::RED;
                    rotate_right(p);
                    sibling = p->left;
                }
                if (sibling == nullptr) {
                    new_child = p;
                    continue;
                }
                if ((!sibling->left || sibling->left->color == block_color::BLACK) &&
                (!sibling->right || sibling->right->color == block_color::BLACK)) {
                    sibling->color = block_color::RED;
                    new_child = p;
                } else {
                    if (!sibling->left || sibling->left->color == block_color::BLACK) {
                        if (sibling->right) {
                            sibling->right->color = block_color::BLACK;
                        }
                        sibling->color = block_color::RED;
                        rotate_left(sibling);
                    }
                    sibling->color = p->color;
                    p->color = block_color::BLACK;
                    if (sibling->left) {
                        sibling->left->color = block_color::BLACK;
                    }
                    rotate_right(p);
                    new_child = allocator_metadata->root;
                }
            }
        }
        if (new_child) {
            new_child->color = block_color::BLACK;
        }
        if (allocator_metadata->root) {
            allocator_metadata->root->color = block_color::BLACK;
        }
    }
}

void allocator_red_black_tree::insert_in_red_black_tree(free_block_metadata_struct* node) {
    if (!node) {
        return;
    }
    node->occupied = false;
    node->parent = node->left = node->right = nullptr;
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    if (!allocator_metadata->root) {
        allocator_metadata->root = node;
    } else {
        auto* x = allocator_metadata->root;
        while (x) {
            if (block_size(_trusted_memory, node) >= block_size(_trusted_memory, x)) {
                if (x->right != nullptr) {
                    x = x->right;
                } else {
                    node->parent = x;
                    x->right = node;
                    break;
                }
            } else {
                if (x->left != nullptr) {
                    x = x->left;
                } else {
                    node->parent = x;
                    x->left = node;
                    break;
                }
            }
        }
    }
    while (node->parent && node->parent->color == block_color::RED) {
        auto* grand = node->parent->parent;
        if (!grand) {
            break;
        }
        auto* uncle = node->parent->is_left_child() ? grand->right : grand->left;
        if (uncle && uncle->color == block_color::RED) {
            node->parent->color = uncle->color = block_color::BLACK;
            grand->color = block_color::RED;
            node = grand;
        } else if (node->parent->is_left_child()) {
            if (node->is_right_child()) {
                node = node->parent;
                rotate_left(node);
            }
            node->parent->color = block_color::BLACK;
            grand->color = block_color::RED;
            rotate_right(grand);
        } else {
            if (node->is_left_child()) {
                node = node->parent;
                rotate_right(node);
            }
            node->parent->color = block_color::BLACK;
            grand->color = block_color::RED;
            rotate_left(grand);
        }
    }
    allocator_metadata->root->color = block_color::BLACK;
}

[[nodiscard]] void *allocator_red_black_tree::do_allocate_sm(
    size_t size)
{
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    if (!allocator_metadata->root) {
        throw std::bad_alloc();
    }
    auto* node = reinterpret_cast<struct free_block_metadata_struct*>(allocator_metadata->root);
    free_block_metadata_struct* best_node = nullptr;
    if (allocator_metadata->mode == fit_mode::the_best_fit) {
        while (node) {
            if (block_size(_trusted_memory, node) - occupied_block_metadata_size >= size) {
                best_node = node;
                node = node->left;
            } else {
                node = node->right;
            }
        }
    } else if (allocator_metadata->mode == fit_mode::first_fit) {
        while (node) {
            if (block_size(_trusted_memory, node) - occupied_block_metadata_size >= size) {
                best_node = node;
                break;
            } else {
                node = node->right;
            }
        }
    } else if (allocator_metadata->mode == fit_mode::the_worst_fit) {
        while (node && node->right) {
            node = node->right;
        }
        if (node && block_size(_trusted_memory, node) - occupied_block_metadata_size >= size) {
            best_node = node;
        }
    }
    if (!best_node) {
        throw std::bad_alloc();
    }
    free_block_metadata_struct* new_free_block = nullptr;
    if (block_size(_trusted_memory, best_node) - occupied_block_metadata_size - size > free_block_metadata_size) {
        new_free_block = reinterpret_cast<struct free_block_metadata_struct*>(
            reinterpret_cast<char*>(best_node) + occupied_block_metadata_size + size);
        new_free_block->occupied = false;
        new_free_block->color = block_color::RED;
        if (best_node->next) {
            best_node->next->prev = new_free_block;
        }
        new_free_block->next = best_node->next;
        new_free_block->prev = best_node;
        best_node->next = new_free_block;
    }
    remove_from_red_black_tree(best_node);
    insert_in_red_black_tree(new_free_block);
    return reinterpret_cast<char*>(best_node) + occupied_block_metadata_size;
}

void allocator_red_black_tree::do_deallocate_sm(
    void *at)
{
    if (!at) return;
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    auto* block = reinterpret_cast<struct free_block_metadata_struct*>(static_cast<char*>(at) - occupied_block_metadata_size);
    char* pool_start = static_cast<char*>(pool_ptr(_trusted_memory));
    char* end_ptr = pool_start + allocator_metadata->pool_size;
    if (reinterpret_cast<char*>(block) < pool_start || end_ptr <= reinterpret_cast<char*>(block)) {
        return;
    }
    if (block->next && !block->next->occupied) {
        remove_from_red_black_tree(reinterpret_cast<struct free_block_metadata_struct*>(block->next));
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    if (block->prev && !block->prev->occupied) {
        remove_from_red_black_tree(reinterpret_cast<struct free_block_metadata_struct*>(block->prev));
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
        block = reinterpret_cast<struct free_block_metadata_struct*>(block->prev);
    }
    insert_in_red_black_tree(block);
}

void allocator_red_black_tree::set_fit_mode(allocator_with_fit_mode::fit_mode mode)
{
    auto* allocator_metadata = reinterpret_cast<struct allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    allocator_metadata->mode = mode;
}


std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info() const
{
    auto* allocator_metadata = reinterpret_cast<allocator_metadata_struct*>(_trusted_memory);
    std::lock_guard<std::mutex> lock(allocator_metadata->mutex);
    try {
        return get_blocks_info_inner();
    } catch (...) {
        return std::vector<allocator_test_utils::block_info>();
    }
}

std::vector<allocator_test_utils::block_info> allocator_red_black_tree::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> v;
    for (auto it = begin(); it != end(); ++it) {
        v.push_back({ it.size(), it.occupied() });
    }
    return v;
}


allocator_red_black_tree::rb_iterator allocator_red_black_tree::begin() const noexcept
{
    return rb_iterator(_trusted_memory);
}

allocator_red_black_tree::rb_iterator allocator_red_black_tree::end() const noexcept
{
    return rb_iterator();
}


bool allocator_red_black_tree::rb_iterator::operator==(const allocator_red_black_tree::rb_iterator &other) const noexcept
{
    return _block_ptr == other._block_ptr;
}

bool allocator_red_black_tree::rb_iterator::operator!=(const allocator_red_black_tree::rb_iterator &other) const noexcept
{
    return !(*this == other);
}

allocator_red_black_tree::rb_iterator &allocator_red_black_tree::rb_iterator::operator++() & noexcept
{
    if (_block_ptr) {
        _block_ptr = _block_ptr->next;
    }
    return *this;
}

allocator_red_black_tree::rb_iterator allocator_red_black_tree::rb_iterator::operator++(int n)
{
    auto tmp = *this;
    ++*this;
    return tmp;
}

size_t allocator_red_black_tree::rb_iterator::size() const noexcept
{
    return block_size(_trusted, _block_ptr);
}

void *allocator_red_black_tree::rb_iterator::operator*() const noexcept
{
    return _block_ptr;
}

allocator_red_black_tree::rb_iterator::rb_iterator()
    : _trusted(nullptr),
      _block_ptr(nullptr)
{
}

allocator_red_black_tree::rb_iterator::rb_iterator(void *trusted)
    : _trusted(trusted),
      _block_ptr(get_head(_trusted))
{
}

bool allocator_red_black_tree::rb_iterator::occupied() const noexcept
{
    return _block_ptr->occupied;
}

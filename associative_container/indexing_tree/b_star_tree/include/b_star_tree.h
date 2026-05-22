#ifndef SYS_PROG_BS_TREE_H
#define SYS_PROG_BS_TREE_H

#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BS_tree final : private compare
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    static constexpr const size_t minimum_keys_in_node = 2 * t - 1;
    static constexpr const size_t maximum_keys_in_node = 3 * t - 2;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration

    struct bstree_node
    {
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bstree_node*, maximum_keys_in_node + 2> _pointers;
        bstree_node() noexcept;

        bool is_leaf() {
            return _pointers.empty();
        }

        bool is_internal() {
            return !is_leaf();
        }
    };

    pp_allocator<value_type> _allocator;
    bstree_node* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit BS_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BS_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BS_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BS_tree(const BS_tree& other);

    BS_tree(BS_tree&& other) noexcept;

    BS_tree& operator=(const BS_tree& other);

    BS_tree& operator=(BS_tree&& other) noexcept;

    ~BS_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bstree_iterator;
    class bstree_reverse_iterator;
    class bstree_const_iterator;
    class bstree_const_reverse_iterator;

    class bstree_iterator final
    {
        std::stack<std::pair<bstree_node**, size_t>> _path;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_iterator;

        friend class BS_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_const_iterator;
        friend class bstree_const_reverse_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_iterator(const std::stack<std::pair<bstree_node**, size_t>>& path = std::stack<std::pair<bstree_node**, size_t>>(), size_t index = 0);

    };

    class bstree_const_iterator final
    {
        std::stack<std::pair<bstree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_const_iterator;

        friend class BS_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_iterator;
        friend class bstree_const_reverse_iterator;

        bstree_const_iterator(const bstree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_const_iterator(const std::stack<std::pair<bstree_node* const*, size_t>>& path = std::stack<std::pair<bstree_node* const*, size_t>>(), size_t index = 0);
    };

    class bstree_reverse_iterator final
    {
        std::stack<std::pair<bstree_node**, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_reverse_iterator;

        friend class BS_tree;
        friend class bstree_iterator;
        friend class bstree_const_iterator;
        friend class bstree_const_reverse_iterator;

        bstree_reverse_iterator(const bstree_iterator& it) noexcept;
        operator bstree_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_reverse_iterator(const std::stack<std::pair<bstree_node**, size_t>>& path = std::stack<std::pair<bstree_node**, size_t>>(), size_t index = 0);
    };

    class bstree_const_reverse_iterator final
    {
        std::stack<std::pair<bstree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bstree_const_reverse_iterator;

        friend class BS_tree;
        friend class bstree_reverse_iterator;
        friend class bstree_const_iterator;
        friend class bstree_iterator;

        bstree_const_reverse_iterator(const bstree_reverse_iterator& it) noexcept;
        operator bstree_const_iterator() const noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        self& operator--();
        self operator--(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t depth() const noexcept;
        size_t current_node_keys_count() const noexcept;
        bool is_terminate_node() const noexcept;
        size_t index() const noexcept;

        explicit bstree_const_reverse_iterator(const std::stack<std::pair<bstree_node* const*, size_t>>& path = std::stack<std::pair<bstree_node* const*, size_t>>(), size_t index = 0);
    };

    friend class bstree_iterator;
    friend class bstree_const_iterator;
    friend class bstree_reverse_iterator;
    friend class bstree_const_reverse_iterator;

    // endregion iterators declaration

    // region element access declaration

    /*
     * Returns a reference to the mapped value of the element with specified key. If no such element exists, an exception of type std::out_of_range is thrown.
     */
    tvalue& at(const tkey&);
    const tvalue& at(const tkey&) const;

    /*
     * If key not exists, makes default initialization of value
     */
    tvalue& operator[](const tkey& key);
    tvalue& operator[](tkey&& key);

    // endregion element access declaration
    // region iterator begins declaration

    bstree_iterator begin();
    bstree_iterator end();

    bstree_const_iterator begin() const;
    bstree_const_iterator end() const;

    bstree_const_iterator cbegin() const;
    bstree_const_iterator cend() const;

    bstree_reverse_iterator rbegin();
    bstree_reverse_iterator rend();

    bstree_const_reverse_iterator rbegin() const;
    bstree_const_reverse_iterator rend() const;

    bstree_const_reverse_iterator crbegin() const;
    bstree_const_reverse_iterator crend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bstree_iterator find(const tkey& key);
    bstree_const_iterator find(const tkey& key) const;

    bstree_iterator lower_bound(const tkey& key);
    bstree_const_iterator lower_bound(const tkey& key) const;

    bstree_iterator upper_bound(const tkey& key);
    bstree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bstree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bstree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bstree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bstree_iterator insert_or_assign(const tree_data_type& data);
    bstree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bstree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bstree_iterator erase(bstree_iterator pos);
    bstree_iterator erase(bstree_const_iterator pos);

    bstree_iterator erase(bstree_iterator beg, bstree_iterator en);
    bstree_iterator erase(bstree_const_iterator beg, bstree_const_iterator en);


    bstree_iterator erase(const tkey& key);

    // endregion modifiers declaration

    // region bstree_balancing declaration

    void split_1to2(bstree_node* parent, bstree_node* full_node, size_t idx);
    
    void split_2to3(bstree_node* parent, bstree_node* left, bstree_node* right, size_t left_idx);
    
    void rotate_left(bstree_node* left, bstree_node* right, bstree_node* parent, int left_idx);
    
    void rotate_right(bstree_node* left, bstree_node* right,bstree_node* parent, int left_idx);

    void merge_2to1(bstree_node* left, bstree_node* right, bstree_node* parent, int left_idx);
    
    void merge_3to2(bstree_node* left, bstree_node* middle, bstree_node* right, bstree_node* parent, int left_idx);

    void redistribute_2to2(bstree_node* left, bstree_node* right, bstree_node* parent, int left_idx);

    // endregion bstree_balancing declaration

    void print_recursive(bstree_node* node, int depth = 0);

    void print_bstree();
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BS_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BS_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BS_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::compare_pairs(const BS_tree::tree_data_type &lhs,
                                                     const BS_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_node::bstree_node() noexcept
{
    _keys.reserve(maximum_keys_in_node + 1);
    _pointers.reserve(maximum_keys_in_node + 2);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BS_tree<tkey, tvalue, compare, t>::value_type> BS_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return _allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_iterator::operator*() const noexcept
{
    bstree_node* current_node = *_path.top().first;
    return reinterpret_cast<reference&>(current_node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::pointer BS_tree<tkey, tvalue, compare, t>::bstree_iterator
::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self & BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator++()
{
    if (_path.empty()) {
        return *this;
    }
    bstree_node* cur_node = *_path.top().first;
    if (cur_node->is_internal()) {
        size_t right_child_idx = _index + 1;
        _path.push({&(cur_node->_pointers[right_child_idx]), right_child_idx});
        cur_node = cur_node->_pointers[right_child_idx];
        while (cur_node->is_internal()) {
            _path.push({&(cur_node->_pointers[0]), 0});
            cur_node = cur_node->_pointers[0];
        }
        _index = 0;
    } else if (_index + 1 < cur_node->_keys.size()) {
        ++_index;
    } else {
        size_t child_idx_in_parent;
        bstree_node* parent;
        do {
            child_idx_in_parent = _path.top().second;
            _path.pop();
            parent = *_path.top().first;
        } while (!_path.empty() && child_idx_in_parent + 1 == parent->_pointers.size());
        if (_path.empty()) {
            _index = 0; 
        } else if (child_idx_in_parent + 1 < parent->_pointers.size()) {
            _index = child_idx_in_parent;
        } else {
            assert(false && "Unreachable.");
        }
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self & BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator--()
{
    if (_path.empty()) {
        return *this; 
    }
    bstree_node* cur_node = *_path.top().first;
    if (cur_node->is_internal()) {
        size_t left_child_idx = _index;
        _path.push({&(cur_node->_pointers[left_child_idx]), left_child_idx});
        cur_node = cur_node->_pointers[left_child_idx];
        while (cur_node->is_internal()) {
            size_t last_child_idx = cur_node->_pointers.size() - 1;
            _path.push({&(cur_node->_pointers[last_child_idx]), last_child_idx});
            cur_node = cur_node->_pointers[last_child_idx];
        }
        _index = cur_node->_keys.size() - 1;
    } else if (_index > 0) {
        --_index;
    } else {
        size_t child_idx_in_parent;
        do {
            child_idx_in_parent = _path.top().second;
            _path.pop();
        } while (!_path.empty() && child_idx_in_parent == 0);
        if (_path.empty()) {
            _index = 0; 
        } else if (child_idx_in_parent > 0) {
            _index = child_idx_in_parent - 1;
        } else {
            assert(false && "Unreachable.");
        }
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator::self BS_tree<tkey, tvalue, compare, t>::bstree_iterator::
operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator==(const self &other) const noexcept
{
    if (_path.empty() && other._path.empty()) {
        return true;
    }
    if (_path.empty() || other._path.empty()) {
        return false;
    }
    return *_path.top().first == *other._path.top().first && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::depth() const noexcept
{
    return _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) {
        return 0;
    }
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) {
        return false;
    }
    return (*_path.top().first)->is_leaf();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_iterator::bstree_iterator(
    const std::stack<std::pair<bstree_node **, size_t>> &path, size_t index)
    : _path(path),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::bstree_const_iterator(const bstree_iterator &it) noexcept
    : _index(it._index)
{
    auto path_copy = it._path;
    std::vector<std::pair<bstree_node**, size_t>> elements;
    while (!path_copy.empty()) {
        elements.emplace_back(path_copy.top());
        path_copy.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        _path.emplace(std::make_pair(const_cast<bstree_node**>(it_el->first), it_el->second));
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator*() const noexcept
{
    bstree_node* const current_node = *_path.top().first;
    return reinterpret_cast<const reference&>(current_node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::pointer BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator++()
{
    bstree_iterator it;
    it._index = _index;
    std::vector<std::pair<bstree_node* const*, size_t>> elements;
    while (!_path.empty()) {
        elements.emplace_back(_path.top());
        _path.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        it._path.push(std::make_pair(const_cast<bstree_node**>(it_el->first), it_el->second));
    }
    ++it;
    *this = bstree_const_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator--()
{
    bstree_iterator it;
    it._index = _index;
    std::vector<std::pair<bstree_node* const*, size_t>> elements;
    while (!_path.empty()) {
        elements.emplace_back(_path.top());
        _path.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        it._path.emplace({it_el->first, it_el->second});
    }
    --it;
    *this = bstree_const_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator==(const self &other) const noexcept
{
    if (_path.empty() && other._path.empty()) {
        return true;
    }
    if (_path.empty() || other._path.empty()) {
        return false;
    }
    return *_path.top().first == *other._path.top().first && _index == other._index;

}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::depth() const noexcept
{
    return _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) {
        return 0;
    }
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) {
        return false;
    }
    return (*_path.top().first)->is_leaf();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator::bstree_const_iterator(
    const std::stack<std::pair<bstree_node * const*, size_t>> &path, size_t index)
    : _path(path),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::bstree_reverse_iterator(const bstree_iterator &it) noexcept
    : _index(it._index),
      _path(it._path)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator BS_tree<tkey, tvalue, compare, t>::bstree_iterator() const noexcept
{
    return bstree_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator*() const noexcept
{
    return *static_cast<bstree_iterator>(*this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::pointer BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator++()
{
    bstree_iterator it = static_cast<bstree_iterator>(*this);
    --it;
    *this = bstree_reverse_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator--()
{
    bstree_iterator it = static_cast<bstree_iterator>(*this);
    ++it;
    *this = bstree_reverse_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_reverse_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator==(const self &other) const noexcept
{
    return static_cast<bstree_iterator>(*this) == static_cast<bstree_iterator>(other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::depth() const noexcept
{
    return _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) {
        return 0;
    }
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) {
        return false;
    }
    return (*_path.top().first)->is_leaf();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator::bstree_reverse_iterator(
    const std::stack<std::pair<bstree_node **, size_t>> &path, size_t index)
    : _path(path),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::bstree_const_reverse_iterator(
    const bstree_reverse_iterator &it) noexcept
{
    auto path_copy = it._path;
    std::vector<std::pair<bstree_node**, size_t>> elements;
    while (!path_copy.empty()) {
        elements.emplace_back(path_copy.top());
        path_copy.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        _path.emplace({it_el->first, it_el->second});
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator() const noexcept
{
    return bstree_const_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::reference BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator*() const noexcept
{
    return *static_cast<bstree_const_iterator>(*this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::pointer BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator++()
{
    bstree_reverse_iterator it;
    it._index = _index;
    std::vector<std::pair<bstree_node* const*, size_t>> elements;
    while (!_path.empty()) {
        elements.emplace_back(_path.top());
        _path.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        it._path.emplace({it_el->first, it_el->second});
    }
    ++it;
    *this = bstree_const_reverse_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self & BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator--()
{
    bstree_reverse_iterator it;
    it._index = _index;
    std::vector<std::pair<bstree_node* const*, size_t>> elements;
    while (!_path.empty()) {
        elements.emplace_back(_path.top());
        _path.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        it._path.emplace({it_el->first, it_el->second});
    }
    --it;
    *this = bstree_const_reverse_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::self BS_tree<tkey, tvalue, compare, t>::
bstree_const_reverse_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator==(const self &other) const noexcept
{
    return static_cast<bstree_const_iterator>(*this) == static_cast<bstree_const_iterator>(other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::depth() const noexcept
{
    return _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) {
        return 0;
    }
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) {
        return false;
    }
    return (*_path.top().first)->is_leaf();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator::bstree_const_reverse_iterator(
    const std::stack<std::pair<bstree_node * const*, size_t>> &path, size_t index)
    : _path(path),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(const compare& cmp, pp_allocator<value_type> alloc)
    : compare(cmp),
      _allocator(alloc),
      _root(nullptr),
      _size(0)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(pp_allocator<value_type> alloc, const compare& comp)
    : BS_tree(comp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BS_tree<tkey, tvalue, compare, t>::BS_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc)
    : BS_tree(cmp, alloc)
{
    for (auto it = begin; it != end; ++it) {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc)
    : BS_tree(data.begin(), data.end(), cmp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(const BS_tree& other)
{
    if (other._root == nullptr) {
        return;
    }
    _size = other._size;
    std::stack<bstree_node**> copy_to;
    copy_to.push(&_root);
    std::stack<bstree_node*> copy_from;
    copy_from.push(other._root);
    while (copy_from.size() > 0) {
        bstree_node* copying_from = copy_from.top();
        copy_from.pop();
        for (size_t i = 0; i < copying_from->_pointers.size(); ++i) {
            copy_from.push(copying_from->_pointers[i]);
        }
        bstree_node** copying_to = copy_to.top();
        copy_to.pop();
        *copying_to = _allocator.template new_object<bstree_node>();
        *copying_to->_keys = copying_from->_keys;
        *copying_to->_pointers.resize(copying_from->_pointers.size());
        for (size_t i = 0; i < copying_from->_pointers.size(); ++i) {
            copy_to.push(&copying_to->_pointers[i]);
        }
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::BS_tree(BS_tree&& other) noexcept
    : compare(std::move(static_cast<compare&>(other))),
      _allocator(std::move(other._allocator)),
      _root(other._root),
      _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>& BS_tree<tkey, tvalue, compare, t>::operator=(const BS_tree& other)
{
    if (this != &other) {
        BS_tree tmp(other);
        std::swap(static_cast<compare&>(*this), static_cast<compare&>(tmp));
        std::swap(_allocator, tmp._allocator);
        std::swap(_root, tmp._root);
        std::swap(_size, tmp._size);
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>& BS_tree<tkey, tvalue, compare, t>::operator=(BS_tree&& other) noexcept
{
    if (this != &other) {
        BS_tree tmp(std::move(other));
        std::swap(static_cast<compare&>(*this), static_cast<compare&>(tmp));
        std::swap(_allocator, tmp._allocator);
        std::swap(_root, tmp._root);
        std::swap(_size, tmp._size);
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BS_tree<tkey, tvalue, compare, t>::~BS_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end()) {
        throw std::out_of_range("Key not found.");
    }
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& BS_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end()) {
        throw std::out_of_range("Key not found.");
    }
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    return emplace(std::make_pair(key, tvalue())).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BS_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    return emplace(std::make_pair(std::move(key), tvalue())).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::begin()
{
    if (_root == nullptr || _size == 0) {
        return end();
    }
    std::stack<std::pair<bstree_node**, size_t>> path;
    bstree_node** current_ptr = &_root;
    bstree_node* current_node = _root;
    path.push({current_ptr, 0});
    while (current_node->is_internal()) {
        current_ptr = &(current_node->_pointers[0]);
        current_node = *current_ptr;
        path.push({current_ptr, 0});
    }
    return bstree_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::end()
{
    return bstree_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::begin() const
{
    return cbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::end() const
{
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::cbegin() const
{
    if (_root == nullptr || _size == 0) {
        return end();
    }
    std::stack<std::pair<bstree_node* const*, size_t>> path;
    bstree_node* const* current_ptr = &_root;
    bstree_node* current_node = _root;
    path.push({current_ptr, 0});
    while (current_node->is_internal()) {
        current_ptr = &(current_node->_pointers[0]);
        current_node = *current_ptr;
        path.push({current_ptr, 0});
    }
    return bstree_const_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::cend() const
{
    return bstree_const_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rbegin()
{
    if (_root == nullptr || _size == 0) {
        return rend();
    }
    std::stack<std::pair<bstree_node**, size_t>> path;
    bstree_node** current_ptr = &_root;
    path.push({current_ptr, 0});
    bstree_node* current_node = _root;
    while (current_node->is_internal()) {
        current_ptr = &(current_node->_pointers[current_node->_pointers.size() - 1]);
        path.push({current_ptr, current_node->_pointers.size() - 1});
        current_node = *current_ptr;
    }
    return bstree_reverse_iterator(path, current_node->_keys.size() - 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rend()
{
    return bstree_reverse_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rbegin() const
{
    return crbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::rend() const
{
    return crend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::crbegin() const
{
    if (_root == nullptr || _size == 0) {
        return rend();
    }
    std::stack<std::pair<bstree_node**, size_t>> path;
    bstree_node* const* current_ptr = &_root;
    path.push({current_ptr, 0});
    bstree_node* current_node = _root;
    while (current_node->is_internal()) {
        current_ptr = &(current_node->_pointers[current_node->_pointers.size() - 1]);
        path.push({current_ptr, current_node->_pointers.size() - 1});
        current_node = *current_ptr;
    }
    return bstree_reverse_iterator(path, current_node->_keys.size() - 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_reverse_iterator BS_tree<tkey, tvalue, compare, t>::crend() const
{
    return bstree_const_reverse_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BS_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return size() == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    auto it = lower_bound(key);
    if (it != end() && !compare_keys(key, it->first)) {
        return it;
    }
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    bstree_iterator it = find(key);
    return bstree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr) {
        return end();
    }
    std::stack<std::pair<bstree_node**, size_t>> path;
    std::stack<std::pair<bstree_node**, size_t>> last_valid_path;
    size_t last_valid_index = 0;
    bool found_larger = false;
    bstree_node** current_ptr = &_root;
    size_t idx_in_parent = 0;
    while (true) {
        bstree_node* node = *current_ptr;
        size_t i = std::distance(node->_keys.begin(), 
            std::lower_bound(node->_keys.begin(), node->_keys.end(), key, 
            [this](const auto& data, const tkey& k) {
                return compare_keys(data.first, k);
            }));
        if (i < node->_keys.size() && !compare_keys(key, node->_keys[i].first)) {
            path.push(std::make_pair(current_ptr, idx_in_parent));
            return bstree_iterator(path, i);
        }
        if (node->is_leaf()) {
            if (i < node->_keys.size()) {
                path.push(std::make_pair(current_ptr, idx_in_parent));
                return bstree_iterator(path, i);
            }
            if (found_larger) {
                return bstree_iterator(last_valid_path, last_valid_index);
            }
            return end();
        } else {
            if (i < node->_keys.size()) {
                last_valid_path = path;
                last_valid_path.push(std::make_pair(current_ptr, idx_in_parent));
                last_valid_index = i;
                found_larger = true;
            }
            path.push(std::make_pair(current_ptr, idx_in_parent));
            current_ptr = &(node->_pointers[i]);
            idx_in_parent = i;
        }
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    bstree_iterator it = lower_bound(key);
    return bstree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    bstree_iterator it = lower_bound(key);
    if (!compare_keys(key, it->first)) {
        ++it;
    }
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_const_iterator BS_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    bstree_iterator it = upper_bound(key);
    return bstree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BS_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (_root == nullptr) {
        return;
    }
    std::stack<bstree_node*> nodes;
    nodes.push(_root);
    while (!nodes.empty()) {
        bstree_node* current = nodes.top();
        nodes.pop();
        if (!current->is_leaf()) {
            for (bstree_node* child : current->_pointers) {
                if (child != nullptr) {
                    nodes.push(child);
                }
            }
        }
        _allocator.delete_object(current);
    }
    _root = nullptr;
    _size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool> BS_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool> BS_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
std::pair<typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator, bool> BS_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    std::stack<std::pair<bstree_node**, size_t>> path;
    if (_root == nullptr) {
        _root = _allocator.template new_object<bstree_node>();
        _root->_keys.push_back(std::move(data));
        _size = 1;
        path.push(std::make_pair(&_root, 0));
        return std::make_pair(bstree_iterator(path, 0), true);
    }
    
    bstree_node** current_ptr;
    size_t i;

    bstree_iterator it = lower_bound(data.first);
    if (it == end()) {
        auto r_it = rbegin();
        current_ptr = r_it._path.top().first;
        i = r_it._index + 1;
        path = r_it._path;
    } else if (it->first == data.first) {
        // (*it._path.top().first)->_keys[it._index].second = data.second;
        return std::make_pair(it, false);
    } else {
        current_ptr = it._path.top().first;
        if ((*current_ptr)->is_internal()) {
            --it;
            i = it._index + 1;
            current_ptr = it._path.top().first;
        } else {
            i = it._index;
        }
        path = it._path;
    }

    (*current_ptr)->_keys.insert((*current_ptr)->_keys.begin() + i, std::move(data));
    ++_size;

    bstree_node* cur_node = *current_ptr;
    while (cur_node->_keys.size() > maximum_keys_in_node) {
        if (path.size() == 1) {
            bstree_node* new_root = _allocator.template new_object<bstree_node>();
            new_root->_pointers.push_back(cur_node);
            split_1to2(new_root, cur_node, 0);
            _root = new_root;
            break;
        }
        auto idx_in_parent = path.top().second;
        path.pop();
        auto* parent = *path.top().first;
        if (idx_in_parent > 0) {
            auto* left = parent->_pointers[idx_in_parent - 1];
            if (left->_keys.size() < maximum_keys_in_node) {
                // redistribute_2to2(left, cur_node, parent, idx_in_parent - 1);
                rotate_left(left, cur_node, parent, idx_in_parent - 1);
                continue;
            }
        }
        if (idx_in_parent + 1 < parent->_pointers.size()) {
            auto* right = parent->_pointers[idx_in_parent + 1];
            if (right->_keys.size() < maximum_keys_in_node) {
                // redistribute_2to2(cur_node, right, parent, idx_in_parent);
                rotate_right(cur_node, right, parent, idx_in_parent);
                continue;
            }
        }
        if (idx_in_parent > 0) {
            auto* left = parent->_pointers[idx_in_parent - 1];
            split_2to3(parent, left, cur_node, idx_in_parent - 1);
            continue;
        }
        if (idx_in_parent + 1 < parent->_pointers.size()) {
            auto* right = parent->_pointers[idx_in_parent + 1];
            split_2to3(parent, cur_node, right, idx_in_parent);
            continue;
        }
        cur_node = parent;
    }
    return std::make_pair(find(data.first), true);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::split_1to2(bstree_node* parent, bstree_node* full_node,
    size_t idx)
{
    bstree_node* new_node = _allocator.template new_object<bstree_node>();
    auto mid_idx = full_node->_keys.size() / 2;
    parent->_keys.insert(parent->_keys.begin() + idx, std::move(full_node->_keys[mid_idx]));
    new_node->_keys.assign(
        std::make_move_iterator(full_node->_keys.begin() + mid_idx + 1),
        std::make_move_iterator(full_node->_keys.end()));
    full_node->_keys.erase(full_node->_keys.begin() + mid_idx, full_node->_keys.end());
    if (full_node->is_internal()) {
        new_node->_pointers.assign(full_node->_pointers.begin() + mid_idx + 1, full_node->_pointers.end());
        full_node->_pointers.erase(full_node->_pointers.begin() + mid_idx + 1, full_node->_pointers.end());
    }
    parent->_pointers.insert(parent->_pointers.begin() + idx + 1, new_node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::split_2to3(bstree_node* parent, bstree_node* left,
    bstree_node* right, size_t left_idx)
{
    bstree_node* new_node = _allocator.template new_object<bstree_node>();
    auto total = left->_keys.size() + 1 + right->_keys.size();
    auto s1 = (total - 2) / 3;
    auto s2 = (total - 2 - s1) / 2;
    auto s3 = total - 2 - s1 - s2;
    new_node->_keys.assign(
        std::make_move_iterator(right->_keys.end() - s3),
        std::make_move_iterator(right->_keys.end()));
    parent->_keys.insert(parent->_keys.begin() + left_idx + 1,
                         std::move(right->_keys[right->_keys.size() - s3 - 1]));
    right->_keys.erase(right->_keys.end() - s3 - 1, right->_keys.end());
    right->_keys.insert(right->_keys.begin(), std::move(parent->_keys[left_idx]));
    auto to_s2 = s2 - right->_keys.size();
    right->_keys.insert(right->_keys.begin(),
        std::make_move_iterator(left->_keys.end() - to_s2),
        std::make_move_iterator(left->_keys.end()));
    parent->_keys[left_idx] = std::move(left->_keys[left->_keys.size() - to_s2 - 1]);
    left->_keys.erase(left->_keys.end() - to_s2 - 1, left->_keys.end());
    if (left->is_internal()) {
        auto s1_p = s1 + 1;
        auto s2_p = s2 + 1;
        auto s3_p = s3 + 1;
        new_node->_pointers.assign(right->_pointers.end() - s3_p, right->_pointers.end());
        right->_pointers.erase(right->_pointers.end() - s3_p, right->_pointers.end());
        auto to_s2_p = s2_p - right->_pointers.size();
        right->_pointers.insert(right->_pointers.begin(), left->_pointers.end() - to_s2_p, left->_pointers.end());
        left->_pointers.erase(left->_pointers.end() - to_s2_p, left->_pointers.end());
    }
    parent->_pointers.insert(parent->_pointers.begin() + left_idx + 2, new_node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::rotate_left(bstree_node* left, bstree_node* right,
    bstree_node* parent, int left_idx)
{
    left->_keys.push_back(std::move(parent->_keys[left_idx]));
    parent->_keys[left_idx] = std::move(right->_keys.front());
    right->_keys.erase(right->_keys.begin());
    if (right->is_internal()) {
        left->_pointers.push_back(right->_pointers.front());
        right->_pointers.erase(right->_pointers.begin());
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::rotate_right(bstree_node* left, bstree_node* right,
    bstree_node* parent, int left_idx)
{
    right->_keys.insert(right->_keys.begin(), std::move(parent->_keys[left_idx]));
    parent->_keys[left_idx] = std::move(left->_keys.back());
    left->_keys.pop_back();
    if (left->is_internal()) {
        right->_pointers.insert(right->_pointers.begin(), left->_pointers.back());
        left->_pointers.pop_back();
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::redistribute_2to2(bstree_node* left, bstree_node* right,
    bstree_node* parent, int left_idx)
{
    auto total = left->_keys.size() + right->_keys.size();
    auto s1 = total / 2;
    auto s2 = total - s1;
    while (left->_keys.size() > s1) {
        rotate_right(left, right, parent, left_idx);
    }
    while (left->_keys.size() < s1) {
        rotate_left(left, right, parent, left_idx);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    return emplace_or_assign(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    return emplace_or_assign(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    auto it = lower_bound(data.first);
    if (it != end() && !compare_keys(data.first, it->first)) {
        auto& target_pair = (*it._path.top().first)->_keys[it._index];
        target_pair.second = std::move(data.second);
        return it;
    }
    return insert(std::move(data)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_iterator pos)
{
    return erase(bstree_const_iterator(pos));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_const_iterator pos)
{
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_iterator beg, bstree_iterator en)
{
    return erase(bstree_const_iterator(beg), bstree_const_iterator(en));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(bstree_const_iterator beg, bstree_const_iterator en)
{
    std::vector<tkey> keys;
    for (auto it = beg; it != en; ++it) {
        keys.emplace_back(it->first);
    }
    bstree_iterator it = end();
    for (const auto& key : keys) {
        it = erase(key);
    }
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BS_tree<tkey, tvalue, compare, t>::bstree_iterator BS_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    auto it = find(key);
    if (it == end()) {
        return end();
    }
    auto next_it = it;
    ++next_it;
    tkey next_key;
    bool has_next = (next_it != end());
    if (has_next) {
        next_key = next_it->first;
    }

    if (!it.is_terminate_node()) {
        bstree_node* internal_node = *it._path.top().first;
        internal_node->_keys[it._index] = *next_it;
        ++it;
    }
    assert(it.is_terminate_node());
    bstree_node* leaf_node = *it._path.top().first;
    leaf_node->_keys.erase(leaf_node->_keys.begin() + it._index);
    while (it._path.size() > 1) {
        bstree_node* cur_node = *it._path.top().first;
        auto cur_node_idx = it._path.top().second;
        it._path.pop();
        bstree_node* parent = *it._path.top().first;
        if (cur_node->_keys.size() >= minimum_keys_in_node) {
            break;
        }
        if (parent == _root && parent->_pointers.size() == 2) {
            if (cur_node->_keys.size() < maximum_keys_in_node / 2) {
                if (cur_node_idx == 0) {
                    auto* right = parent->_pointers[1];
                    if (right->_keys.size() > maximum_keys_in_node / 2) {
                        rotate_left(cur_node, right, parent, 0);
                    } else {
                        merge_2to1(cur_node, right, parent, 0);
                    }
                } else {
                    auto* left = parent->_pointers[0];
                    if (left->_keys.size() > maximum_keys_in_node / 2) {
                        rotate_right(left, cur_node, parent, 0);
                    } else {
                        merge_2to1(left, cur_node, parent, 0);
                    }
                }
            }
            break;
        }
        if (cur_node_idx == 0) {
            auto* right = parent->_pointers[cur_node_idx + 1];
            auto* right_right = parent->_pointers[cur_node_idx + 2];
            if (right->_keys.size() > minimum_keys_in_node) {
                rotate_left(cur_node, right, parent, cur_node_idx);
                continue;
            }
            if (right_right->_keys.size() > minimum_keys_in_node) {
                rotate_left(right, right_right, parent, cur_node_idx + 1);
                rotate_left(cur_node, right, parent, cur_node_idx);
                continue;
            }
            merge_3to2(cur_node, right, right_right, parent, cur_node_idx);
            continue;
        }
        if (cur_node_idx == parent->_pointers.size() - 1) {
            auto* left = parent->_pointers[cur_node_idx - 1];
            auto* left_left = parent->_pointers[cur_node_idx - 2];
            if (left->_keys.size() > minimum_keys_in_node) {
                rotate_right(left, cur_node, parent, cur_node_idx - 1);
                continue;
            }
            if (left_left->_keys.size() > minimum_keys_in_node) {
                rotate_left(left_left, left, parent, cur_node_idx - 2);
                rotate_left(left, cur_node, parent, cur_node_idx - 1);
                continue;
            }
            merge_3to2(left_left, left, cur_node, parent, cur_node_idx - 2);
            continue;
        }
        bstree_node* right = parent->_pointers[cur_node_idx + 1];
        if (right->_keys.size() > minimum_keys_in_node) {
            rotate_left(cur_node, right, parent, cur_node_idx);
            continue;
        }
        bstree_node* left = parent->_pointers[cur_node_idx - 1];
        if (left->_keys.size() > minimum_keys_in_node) {
            rotate_right(left, cur_node, parent, cur_node_idx - 1);
            continue;
        }
        merge_3to2(left, cur_node, right, parent, cur_node_idx - 1);
    }
    if (_root && _root->_keys.empty()) {
        assert(_root->_pointers.size() <= 1);
        if (_root->_pointers.size() == 1) {
            bstree_node* old_root = _root;
            _root = _root->_pointers[0];
            _allocator.delete_object(old_root);
        } else {
            _root = nullptr;
        }
    }
    --_size;
    return has_next ? find(next_key) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::merge_2to1(bstree_node* left, bstree_node* right,
    bstree_node* parent, int left_idx)
{
    left->_keys.push_back(std::move(parent->_keys[left_idx]));
    left->_keys.insert(left->_keys.end(),
        std::make_move_iterator(right->_keys.begin()),
        std::make_move_iterator(right->_keys.end()));
    left->_pointers.insert(left->_pointers.end(), right->_pointers.begin(), right->_pointers.end());
    parent->_keys.erase(parent->_keys.begin() + left_idx);
    parent->_pointers.erase(parent->_pointers.begin() + left_idx + 1);
    _allocator.delete_object(right);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::merge_3to2(bstree_node* left, bstree_node* middle,
    bstree_node* right, bstree_node* parent, int left_idx)
{
    auto total = left->_keys.size() + middle->_keys.size() + right->_keys.size() + 2;
    auto s1 = (total - 1) / 2;
    auto s2 = total - 1 - s1;
    left->_keys.push_back(std::move(parent->_keys[left_idx]));
    auto to_s1 = s1 - left->_keys.size();
    left->_keys.insert(left->_keys.end(),
        std::make_move_iterator(middle->_keys.begin()),
        std::make_move_iterator(middle->_keys.begin() + to_s1));
    parent->_keys[left_idx] = std::move(middle->_keys[to_s1]);
    middle->_keys.erase(middle->_keys.begin(), middle->_keys.begin() + to_s1 + 1);
    middle->_keys.insert(middle->_keys.end(),
        std::make_move_iterator(right->_keys.begin()),
        std::make_move_iterator(right->_keys.end()));
    if (left->is_internal()) {
        auto s1_p = s1 + 1;
        auto s2_p = s2 + 1;
        auto to_s1 = s1 - left->_pointers.size();
        left->_pointers.insert(left->_pointers.end(), middle->_pointers.begin(), middle->_pointers.begin() + to_s1);
        middle->_pointers.erase(middle->_pointers.begin(), middle->_pointers.begin() + to_s1);
        middle->_pointers.insert(middle->_pointers.end(), right->_pointers.begin(), right->_pointers.end());
    }
    parent->_keys.erase(parent->_keys.begin() + left_idx + 1);
    parent->_pointers.erase(parent->_pointers.begin() + left_idx + 2);
    _allocator.delete_object(right);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::print_recursive(BS_tree<tkey, tvalue, compare, t>::bstree_node* node, int depth) {
    if (!node) return;
    std::string indent(depth * 4, ' ');
    if (node->is_leaf()) {
        std::cout << indent << "[Leaf]: ";
        for (const auto& key : node->_keys) {
            std::cout << key.first << ":" << key.second << " ";
        }
        std::cout << "\n";
    } else {
        size_t i = 0;
        for (; i < node->_keys.size(); ++i) {
            if (i < node->_pointers.size()) {
                print_recursive(node->_pointers[i], depth + 1);
            }
            std::cout << indent << "Key: " << node->_keys[i].first << " Val: " << node->_keys[i].second << "\n";
        }
        if (i < node->_pointers.size()) {
            print_recursive(node->_pointers[i], depth + 1);
        }
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BS_tree<tkey, tvalue, compare, t>::print_bstree() {
    if (!_root) {
        std::cout << "Tree is empty.\n";
        return;
    }
    std::cout << "--- B-Tree Structure ---\n";
    print_recursive(_root, 0);
    std::cout << "------------------------\n";
}

#endif
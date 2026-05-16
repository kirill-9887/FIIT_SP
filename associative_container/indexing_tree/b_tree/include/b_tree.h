#ifndef SYS_PROG_B_TREE_H
#define SYS_PROG_B_TREE_H

#include <iterator>
#include <utility>
#include <boost/container/static_vector.hpp>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <not_implemented.h>
#include <initializer_list>

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class B_tree final : private compare // EBCO
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    static constexpr const size_t minimum_keys_in_node = t - 1;
    static constexpr const size_t maximum_keys_in_node = 2 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration


    struct btree_node
    {
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<btree_node*, maximum_keys_in_node + 2> _pointers;
        btree_node() noexcept;

        bool is_leaf() {
            return _pointers.empty();
        }

        bool is_internal() {
            return !is_leaf();
        }
    };

    pp_allocator<value_type> _allocator;
    btree_node* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit B_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit B_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit B_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    B_tree(const B_tree& other);

    B_tree(B_tree&& other) noexcept;

    B_tree& operator=(const B_tree& other);

    B_tree& operator=(B_tree&& other) noexcept;

    ~B_tree() noexcept;

    // endregion five declaration

    // region btree_balancing declaration

    void split(btree_node* parent, size_t index, btree_node* full_node);

    void left_rotation(btree_node* node, btree_node* right, btree_node* parent, int node_idx);

    void right_rotation(btree_node* left, btree_node* node, btree_node* parent, int node_idx);

    void merge_nodes(btree_node* left, btree_node* right, btree_node* parent, int left_idx);

    // endregion btree_balancing declaration

    // region iterators declaration

    class btree_iterator;
    class btree_reverse_iterator;
    class btree_const_iterator;
    class btree_const_reverse_iterator;

    class btree_iterator final
    {
        std::stack<std::pair<btree_node**, size_t>> _path;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_const_iterator;
        friend class btree_const_reverse_iterator;

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

        explicit btree_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);

    };

    class btree_const_iterator final
    {
        std::stack<std::pair<btree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_const_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_iterator;
        friend class btree_const_reverse_iterator;

        btree_const_iterator(const btree_iterator& it) noexcept;

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

        explicit btree_const_iterator(const std::stack<std::pair<btree_node* const*, size_t>>& path = std::stack<std::pair<btree_node* const*, size_t>>(), size_t index = 0);
    };

    class btree_reverse_iterator final
    {
        std::stack<std::pair<btree_node**, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_reverse_iterator;

        friend class B_tree;
        friend class btree_iterator;
        friend class btree_const_iterator;
        friend class btree_const_reverse_iterator;

        btree_reverse_iterator(const btree_iterator& it) noexcept;
        operator btree_iterator() const noexcept;

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

        explicit btree_reverse_iterator(const std::stack<std::pair<btree_node**, size_t>>& path = std::stack<std::pair<btree_node**, size_t>>(), size_t index = 0);
    };

    class btree_const_reverse_iterator final
    {
        std::stack<std::pair<btree_node* const*, size_t>> _path;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = btree_const_reverse_iterator;

        friend class B_tree;
        friend class btree_reverse_iterator;
        friend class btree_const_iterator;
        friend class btree_iterator;

        btree_const_reverse_iterator(const btree_reverse_iterator& it) noexcept;
        operator btree_const_iterator() const noexcept;

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

        explicit btree_const_reverse_iterator(const std::stack<std::pair<btree_node* const*, size_t>>& path = std::stack<std::pair<btree_node* const*, size_t>>(), size_t index = 0);
    };

    friend class btree_iterator;
    friend class btree_const_iterator;
    friend class btree_reverse_iterator;
    friend class btree_const_reverse_iterator;

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

    btree_iterator begin();
    btree_iterator end();

    btree_const_iterator begin() const;
    btree_const_iterator end() const;

    btree_const_iterator cbegin() const;
    btree_const_iterator cend() const;

    btree_reverse_iterator rbegin();
    btree_reverse_iterator rend();

    btree_const_reverse_iterator rbegin() const;
    btree_const_reverse_iterator rend() const;

    btree_const_reverse_iterator crbegin() const;
    btree_const_reverse_iterator crend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    btree_iterator find(const tkey& key);
    btree_const_iterator find(const tkey& key) const;

    btree_iterator lower_bound(const tkey& key);
    btree_const_iterator lower_bound(const tkey& key) const;

    btree_iterator upper_bound(const tkey& key);
    btree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<btree_iterator, bool> insert(const tree_data_type& data);
    std::pair<btree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<btree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    btree_iterator insert_or_assign(const tree_data_type& data);
    btree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    btree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    btree_iterator erase(btree_iterator pos);
    btree_iterator erase(btree_const_iterator pos);

    btree_iterator erase(btree_iterator beg, btree_iterator en);
    btree_iterator erase(btree_const_iterator beg, btree_const_iterator en);


    btree_iterator erase(const tkey& key);

    // endregion modifiers declaration
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
B_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> B_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
B_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> B_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_pairs(const B_tree::tree_data_type &lhs,
                                                     const B_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_node::btree_node() noexcept
{
    _keys.reserve(maximum_keys_in_node + 1);
    _pointers.reserve(maximum_keys_in_node + 2);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename B_tree<tkey, tvalue, compare, t>::value_type> B_tree<tkey, tvalue, compare, t>::get_allocator() const noexcept
{
    return _allocator;
}

// region constructors implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        const compare& cmp,
        pp_allocator<value_type> alloc)
    : compare(cmp),
      _allocator(alloc),
      _root(nullptr),
      _size(0)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        pp_allocator<value_type> alloc,\
        const compare& comp)
    : B_tree(comp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
B_tree<tkey, tvalue, compare, t>::B_tree(
        iterator begin,
        iterator end,
        const compare& cmp,
        pp_allocator<value_type> alloc)
    : B_tree(cmp, alloc)
{
    for (auto it = begin; it != end; ++it) {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(
        std::initializer_list<std::pair<tkey, tvalue>> data,
        const compare& cmp,
        pp_allocator<value_type> alloc)
    : B_tree(data.begin(), data.end(), cmp, alloc)
{
}

// endregion constructors implementation

// region five implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::~B_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(const B_tree& other)
    : B_tree(static_cast<const compare&>(other), other._allocator)
{
    if (other._root == nullptr) {
        return;
    }
    _size = other._size;
    std::stack<btree_node**> copy_to;
    copy_to.push(&_root);
    std::stack<btree_node*> copy_from;
    copy_from.push(other._root);
    while (copy_from.size() > 0) {
        btree_node* copying_from = copy_from.top();
        copy_from.pop();
        for (size_t i = 0; i < copying_from->_pointers.size(); ++i) {
            copy_from.push(copying_from->_pointers[i]);
        }
        btree_node** copying_to = copy_to.top();
        copy_to.pop();
        *copying_to = _allocator.template new_object<btree_node>();
        *copying_to->_keys = copying_from->_keys;
        *copying_to->_pointers.resize(copying_from->_pointers.size());
        for (size_t i = 0; i < copying_from->_pointers.size(); ++i) {
            copy_to.push(&copying_to->_pointers[i]);
        }
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(const B_tree& other)
{
    if (this != &other) {
        B_tree tmp(other);
        std::swap(static_cast<compare&>(*this), static_cast<compare&>(tmp));
        std::swap(_allocator, tmp._allocator);
        std::swap(_root, tmp._root);
        std::swap(_size, tmp._size);
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::B_tree(B_tree&& other) noexcept
    : compare(std::move(static_cast<compare&>(other))),
      _allocator(std::move(other._allocator)),
      _root(other._root),
      _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>& B_tree<tkey, tvalue, compare, t>::operator=(B_tree&& other) noexcept
{
    if (this != &other) {
        B_tree tmp(std::move(other));
        std::swap(static_cast<compare&>(*this), static_cast<compare&>(tmp));
        std::swap(_allocator, tmp._allocator);
        std::swap(_root, tmp._root);
        std::swap(_size, tmp._size);
    }
    return *this;
}

// endregion five implementation

// region iterators implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_iterator::btree_iterator(
        const std::stack<std::pair<btree_node**, size_t>>& path, size_t index)
    : _path(path),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator*() const noexcept
{
    btree_node* current_node = *_path.top().first;
    return reinterpret_cast<reference>(current_node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++()
{
    if (_path.empty()) {
        return *this;
    }
    btree_node* cur_node = *_path.top().first;
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
        btree_node* parent;
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
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator&
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--()
{
    if (_path.empty()) {
        return *this; 
    }
    btree_node* cur_node = *_path.top().first;
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
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::btree_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator==(const self& other) const noexcept
{
    if (_index != other._index) {
        return false;
    }
    if (_path.empty() && other._path.empty()) {
        return true;
    }
    if (_path.empty() || other._path.empty()) {
        return false;
    }
    return *_path.top().first == *other._path.top().first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::depth() const noexcept
{
    return _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) {
        return 0;
    }
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) {
        return false;
    }
    return (*_path.top().first)->is_leaf();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
        const std::stack<std::pair<btree_node* const*, size_t>>& path, size_t index)
    : _path(path),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::btree_const_iterator(
        const btree_iterator& it) noexcept
    : _index(it._index)
{
    auto path_copy = it._path;
    std::vector<std::pair<btree_node**, size_t>> elements;
    while (!path_copy.empty()) {
        elements.emplace_back(path_copy.top());
        path_copy.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        _path.emplace(std::make_pair(const_cast<btree_node**>(it_el->first), it_el->second));
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator*() const noexcept
{
    btree_node* const current_node = *_path.top().first;
    return reinterpret_cast<reference>(current_node->_keys[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++()
{
    btree_iterator it;
    it._index = _index;
    std::vector<std::pair<btree_node* const*, size_t>> elements;
    while (!_path.empty()) {
        elements.emplace_back(_path.top());
        _path.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        it._path.push(std::make_pair(const_cast<btree_node**>(it_el->first), it_el->second));
    }
    ++it;
    *this = btree_const_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--()
{
    btree_iterator it;
    it._index = _index;
    std::vector<std::pair<btree_node* const*, size_t>> elements;
    while (!_path.empty()) {
        elements.emplace_back(_path.top());
        _path.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        it._path.emplace({it_el->first, it_el->second});
    }
    --it;
    *this = btree_const_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator==(const self& other) const noexcept
{
    if (_index != other._index) {
        return false;
    }
    if (_path.empty() && other._path.empty()) {
        return true;
    }
    if (_path.empty() || other._path.empty()) {
        return false;
    }
    return *_path.top().first == *other._path.top().first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::depth() const noexcept
{
    return _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) {
        return 0;
    }
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) {
        return false;
    }
    return (*_path.top().first)->is_leaf();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const std::stack<std::pair<btree_node**, size_t>>& path, size_t index)
    : _path(path),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::btree_reverse_iterator(
        const btree_iterator& it) noexcept
    : _index(it._index),
      _path(it._path)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_iterator() const noexcept
{
    return btree_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator*() const noexcept
{
    return *static_cast<btree_iterator>(*this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++()
{
    btree_iterator it = static_cast<btree_iterator>(*this);
    --it;
    *this = btree_reverse_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--()
{
    btree_iterator it = static_cast<btree_iterator>(*this);
    ++it;
    *this = btree_reverse_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator==(const self& other) const noexcept
{
    return static_cast<btree_iterator>(*this) == static_cast<btree_iterator>(other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::depth() const noexcept
{
    return _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) {
        return 0;
    }
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) {
        return false;
    }
    return (*_path.top().first)->is_leaf();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const std::stack<std::pair<btree_node* const*, size_t>>& path, size_t index)
    : _path(path),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::btree_const_reverse_iterator(
        const btree_reverse_iterator& it) noexcept
{
    auto path_copy = it._path;
    std::vector<std::pair<btree_node**, size_t>> elements;
    while (!path_copy.empty()) {
        elements.emplace_back(path_copy.top());
        path_copy.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        _path.emplace({it_el->first, it_el->second});
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator B_tree<tkey, tvalue, compare, t>::btree_const_iterator() const noexcept
{
    return btree_const_iterator(_path, _index);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::reference
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator*() const noexcept
{
    return *static_cast<btree_const_iterator>(*this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::pointer
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++()
{
    btree_reverse_iterator it;
    it._index = _index;
    std::vector<std::pair<btree_node* const*, size_t>> elements;
    while (!_path.empty()) {
        elements.emplace_back(_path.top());
        _path.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        it._path.emplace({it_el->first, it_el->second});
    }
    ++it;
    *this = btree_const_reverse_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator&
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--()
{
    btree_reverse_iterator it;
    it._index = _index;
    std::vector<std::pair<btree_node* const*, size_t>> elements;
    while (!_path.empty()) {
        elements.emplace_back(_path.top());
        _path.pop();
    }
    for (auto it_el = elements.rbegin(); it_el != elements.rend(); ++it_el) {
        it._path.emplace({it_el->first, it_el->second});
    }
    --it;
    *this = btree_const_reverse_iterator(it);
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator
B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator--(int)
{
    self temp = *this;
    --(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator==(const self& other) const noexcept
{
    return static_cast<btree_const_iterator>(*this) == static_cast<btree_const_iterator>(other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::depth() const noexcept
{
    return _path.size() - 1;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::current_node_keys_count() const noexcept
{
    if (_path.empty()) {
        return 0;
    }
    return (*_path.top().first)->_keys.size();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::is_terminate_node() const noexcept
{
    if (_path.empty()) {
        return false;
    }
    return (*_path.top().first)->is_leaf();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator::index() const noexcept
{
    return _index;
}

// endregion iterators implementation

// region element access implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end()) {
        throw std::out_of_range("Key not found.");
    }
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& B_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end()) {
        throw std::out_of_range("Key not found.");
    }
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    return emplace(std::make_pair(key, tvalue())).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& B_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    return emplace(std::make_pair(std::move(key), tvalue())).first->second;
}

// endregion element access implementation

// region iterator begins implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::begin()
{
    if (_root == nullptr || _size == 0) {
        return end();
    }
    std::stack<std::pair<btree_node**, size_t>> path;
    btree_node** current_ptr = &_root;
    btree_node* current_node = _root;
    path.push({current_ptr, 0});
    while (current_node->is_internal()) {
        current_ptr = &(current_node->_pointers[0]);
        current_node = *current_ptr;
        path.push({current_ptr, 0});
    }
    return btree_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::end()
{
    return btree_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::begin() const
{
    return cbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::end() const
{
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cbegin() const
{
    if (_root == nullptr || _size == 0) {
        return end();
    }
    std::stack<std::pair<btree_node* const*, size_t>> path;
    btree_node* const* current_ptr = &_root;
    btree_node* current_node = _root;
    path.push({current_ptr, 0});
    while (current_node->is_internal()) {
        current_ptr = &(current_node->_pointers[0]);
        current_node = *current_ptr;
        path.push({current_ptr, 0});
    }
    return btree_const_iterator(path, 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::cend() const
{
    return btree_const_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin()
{
    if (_root == nullptr || _size == 0) {
        return rend();
    }
    std::stack<std::pair<btree_node**, size_t>> path;
    btree_node** current_ptr = &_root;
    path.push({current_ptr, 0});
    btree_node* current_node = _root;
    while (current_node->is_internal()) {
        current_ptr = &(current_node->_pointers[current_node->_pointers.size() - 1]);
        path.push({current_ptr, current_node->_pointers.size() - 1});
        current_node = *current_ptr;
    }
    return btree_reverse_iterator(path, current_node->_keys.size() - 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend()
{
    return btree_reverse_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rbegin() const
{
    return crbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::rend() const
{
    return crend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crbegin() const
{
    if (_root == nullptr || _size == 0) {
        return rend();
    }
    std::stack<std::pair<btree_node**, size_t>> path;
    btree_node* const* current_ptr = &_root;
    path.push({current_ptr, 0});
    btree_node* current_node = _root;
    while (current_node->is_internal()) {
        current_ptr = &(current_node->_pointers[current_node->_pointers.size() - 1]);
        path.push({current_ptr, current_node->_pointers.size() - 1});
        current_node = *current_ptr;
    }
    return btree_reverse_iterator(path, current_node->_keys.size() - 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_reverse_iterator B_tree<tkey, tvalue, compare, t>::crend() const
{
    return btree_const_reverse_iterator();
}

// endregion iterator begins implementation

// region lookup implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t B_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return size() == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    auto it = lower_bound(key);
    if (it != end() && !compare_keys(key, it->first)) {
        return it;
    }
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    btree_iterator it = find(key);
    return btree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr) {
        return end();
    }
    std::stack<std::pair<btree_node**, size_t>> path;
    std::stack<std::pair<btree_node**, size_t>> last_valid_path;
    size_t last_valid_index = 0;
    bool found_larger = false;
    btree_node** current_ptr = &_root;
    size_t index_in_parent = 0;
    while (true) {
        btree_node* node = *current_ptr;
        size_t i = std::distance(node->_keys.begin(), 
            std::lower_bound(node->_keys.begin(), node->_keys.end(), key, 
            [this](const auto& data, const tkey& k) {
                return compare_keys(data.first, k);
            }));
        if (i < node->_keys.size() && !compare_keys(key, node->_keys[i].first)) {
            path.push(std::make_pair(current_ptr, index_in_parent));
            return btree_iterator(path, i);
        }
        if (node->is_leaf()) {
            if (i < node->_keys.size()) {
                path.push(std::make_pair(current_ptr, index_in_parent));
                return btree_iterator(path, i);
            }
            if (found_larger) {
                return btree_iterator(last_valid_path, last_valid_index);
            }
            return end();
        } else {
            if (i < node->_keys.size()) {
                last_valid_path = path;
                last_valid_path.push(std::make_pair(current_ptr, index_in_parent));
                last_valid_index = i;
                found_larger = true;
            }
            path.push(std::make_pair(current_ptr, index_in_parent));
            current_ptr = &(node->_pointers[i]);
            index_in_parent = i;
        }
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    btree_iterator it = lower_bound(key);
    return btree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    btree_iterator it = lower_bound(key);
    // if (!compare_keys(key, it->first)) {
    //     ++it;
    // }
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_const_iterator B_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    btree_iterator it = upper_bound(key);
    return btree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool B_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
   return find(key) != end();
}

// endregion lookup implementation

// region modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (_root == nullptr) {
        return;
    }
    std::stack<btree_node*> nodes;
    nodes.push(_root);
    while (!nodes.empty()) {
        btree_node* current = nodes.top();
        nodes.pop();
        if (!current->is_leaf()) {
            for (btree_node* child : current->_pointers) {
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
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
std::pair<typename B_tree<tkey, tvalue, compare, t>::btree_iterator, bool>
B_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    std::stack<std::pair<btree_node**, size_t>> path;
    if (_root == nullptr) {
        _root = _allocator.template new_object<btree_node>();
        _root->_keys.push_back(std::move(data));
        _size = 1;
        path.push(std::make_pair(&_root, 0));
        return std::make_pair(btree_iterator(path, 0), true);
    }
    
    btree_node** current_ptr;
    size_t i;

    btree_iterator it = lower_bound(data.first);
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

    btree_node* child_to_split = *current_ptr;
    while (child_to_split->_keys.size() > maximum_keys_in_node) {
        if (path.size() == 1) {
            btree_node* new_root = _allocator.template new_object<btree_node>();
            new_root->_pointers.push_back(child_to_split);
            split(new_root, 0, child_to_split);
            _root = new_root;
            break;
        }
        auto index_in_parent = path.top().second;
        path.pop();
        auto parent = *path.top().first;
        split(parent, index_in_parent, child_to_split);
        child_to_split = parent;
    }
    return std::make_pair(find(data.first), true);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::split(btree_node* parent, size_t index, btree_node* full_node)
{
    btree_node* new_node = _allocator.template new_object<btree_node>();
    parent->_keys.insert(parent->_keys.begin() + index, std::move(full_node->_keys[t]));
    for (size_t j = t + 1; j < full_node->_keys.size(); ++j) {
        new_node->_keys.push_back(std::move(full_node->_keys[j]));
    }
    full_node->_keys.erase(full_node->_keys.begin() + t, full_node->_keys.end());
    if (full_node->is_internal()) {
        for (size_t j = t + 1; j < full_node->_pointers.size(); ++j) {
            new_node->_pointers.push_back(full_node->_pointers[j]);
        }
        full_node->_pointers.erase(full_node->_pointers.begin() + t + 1, full_node->_pointers.end());
    }
    parent->_pointers.insert(parent->_pointers.begin() + index + 1, new_node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    return emplace_or_assign(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    return emplace_or_assign(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename... Args>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
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
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator pos)
{
    return erase(btree_const_iterator(pos));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator pos)
{
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_iterator beg, btree_iterator en)
{
    return erase(btree_const_iterator(beg), btree_const_iterator(en));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(btree_const_iterator beg, btree_const_iterator en)
{
    std::vector<tkey> keys;
    for (auto it = beg; it != en; ++it) {
        keys.emplace_back(it->first);
    }
    btree_iterator it = end();
    for (const auto& key : keys) {
        it = erase(key);
    }
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename B_tree<tkey, tvalue, compare, t>::btree_iterator
B_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
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
        btree_node* internal_node = *it._path.top().first;
        internal_node->_keys[it._index] = *next_it;
        ++it;
    }
    assert(it.is_terminate_node());
    btree_node* leaf_node = *it._path.top().first;
    leaf_node->_keys.erase(leaf_node->_keys.begin() + it._index);
    while (it._path.size() > 1) {
        btree_node* cur_node = *it._path.top().first;
        auto cur_node_idx = it._path.top().second;
        it._path.pop();
        btree_node* parent = *it._path.top().first;
        if (cur_node->_keys.size() < minimum_keys_in_node) {
            if (cur_node_idx + 1 < parent->_pointers.size()) {
                btree_node* right = parent->_pointers[cur_node_idx + 1];
                if (right->_keys.size() > minimum_keys_in_node) {
                    left_rotation(cur_node, right, parent, cur_node_idx);
                    continue;
                }
            }
            if (cur_node_idx > 0) {
                btree_node* left = parent->_pointers[cur_node_idx - 1];
                if (left->_keys.size() > minimum_keys_in_node) {
                    right_rotation(left, cur_node, parent, cur_node_idx);
                    continue;
                }
            }
            if (cur_node_idx + 1 < parent->_pointers.size()) {
                btree_node* right = parent->_pointers[cur_node_idx + 1];
                merge_nodes(cur_node, right, parent, cur_node_idx);
                continue;
            }
            if (cur_node_idx > 0) {
                btree_node* left = parent->_pointers[cur_node_idx - 1];
                merge_nodes(left, cur_node, parent, cur_node_idx - 1);
                continue;
            }
        }
    }
    if (_root && _root->_keys.empty()) {
        assert(_root->_pointers.size() <= 1);
        if (_root->_pointers.size() == 1) {
            btree_node* old_root = _root;
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
void B_tree<tkey, tvalue, compare, t>::left_rotation(B_tree<tkey, tvalue, compare, t>::btree_node* node, 
        B_tree<tkey, tvalue, compare, t>::btree_node* right, 
        B_tree<tkey, tvalue, compare, t>::btree_node* parent, int node_idx) {
    assert(node_idx + 1 < parent->_pointers.size());
    auto right_idx = node_idx + 1;
    assert(right->_keys.size() > minimum_keys_in_node);
    node->_keys.push_back(parent->_keys[node_idx]);
    auto right_data = right->_keys.front();
    right->_keys.erase(right->_keys.begin());
    parent->_keys[node_idx] = right_data;
    if (right->_pointers.size() > 0) {
        auto* pointer = right->_pointers.front();
        right->_pointers.erase(right->_pointers.begin());
        node->_pointers.push_back(pointer);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::right_rotation(B_tree<tkey, tvalue, compare, t>::btree_node* left,
        B_tree<tkey, tvalue, compare, t>::btree_node* node,
        B_tree<tkey, tvalue, compare, t>::btree_node* parent, int node_idx) {
    assert(node_idx > 0);
    auto left_idx = node_idx - 1;
    assert(left->_keys.size() > minimum_keys_in_node);
    node->_keys.insert(node->_keys.begin(), parent->_keys[left_idx]);
    auto left_data = left->_keys.back();
    left->_keys.pop_back();
    parent->_keys[left_idx] = left_data;
    if (left->_pointers.size() > 0) {
        auto* pointer = left->_pointers.back();
        left->_pointers.pop_back();
        node->_pointers.insert(node->_pointers.begin(), pointer);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void B_tree<tkey, tvalue, compare, t>::merge_nodes(B_tree<tkey, tvalue, compare, t>::btree_node* left,
        B_tree<tkey, tvalue, compare, t>::btree_node* right,
        B_tree<tkey, tvalue, compare, t>::btree_node* parent, int left_idx) {
    assert(left->_keys.size() + 1 + right->_keys.size() <= maximum_keys_in_node);
    left->_keys.push_back(parent->_keys[left_idx]);
    left->_keys.insert(left->_keys.end(), right->_keys.begin(), right->_keys.end());
    left->_pointers.insert(left->_pointers.end(), right->_pointers.begin(), right->_pointers.end());
    _allocator.delete_object(right);
    parent->_keys.erase(parent->_keys.begin() + left_idx);
    parent->_pointers.erase(parent->_pointers.begin() + left_idx + 1);
}

// endregion modifiers implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_pairs(const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &lhs,
                   const typename B_tree<tkey, tvalue, compare, t>::tree_data_type &rhs)
{
    return compare_pairs(lhs.first, lhs.second);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool compare_keys(const tkey &lhs, const tkey &rhs)
{
    compare comp;
    return comp(lhs, rhs);
}


#endif
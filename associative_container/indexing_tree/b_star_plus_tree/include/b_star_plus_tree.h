#include <iterator>
#include <utility>
#include <vector>
#include <boost/container/static_vector.hpp>
#include <concepts>
#include <stack>
#include <pp_allocator.h>
#include <associative_container.h>
#include <initializer_list>
#include <not_implemented.h>

#ifndef SYS_PROG_BS_PLUS_TREE_H
#define SYS_PROG_BS_PLUS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BSP_tree final : private compare
{
public:

    using tree_data_type = std::pair<tkey, tvalue>;
    using tree_data_type_const = std::pair<const tkey, tvalue>;
    using value_type = tree_data_type_const;

private:

    // TODO: Another restrictions
    static constexpr const size_t minimum_keys_in_node = 2 * t - 1;
    static constexpr const size_t maximum_keys_in_node = 3 * t - 1;

    // region comparators declaration

    inline bool compare_keys(const tkey& lhs, const tkey& rhs) const;
    inline bool compare_pairs(const tree_data_type& lhs, const tree_data_type& rhs) const;

    // endregion comparators declaration

    struct bsptree_node_base
    {
        bool _is_terminated;
        virtual size_t size() = 0;

        bsptree_node_base() noexcept;
        virtual ~bsptree_node_base() =default;
    };

    struct bsptree_node_term : public bsptree_node_base
    {
        bsptree_node_term* _next;
        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _data;
        bsptree_node_term() noexcept;

        size_t size() override {
            return _data.size();
        }
    };

    struct bsptree_node_middle : public bsptree_node_base
    {
        boost::container::static_vector<tkey, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bsptree_node_base*, maximum_keys_in_node + 2> _pointers;
        bsptree_node_middle() noexcept;

        size_t size() override {
            return _keys.size();
        }
    };

    pp_allocator<value_type> _allocator;
    bsptree_node_base* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit BSP_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BSP_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BSP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BSP_tree(const BSP_tree& other);

    BSP_tree(BSP_tree&& other) noexcept;

    BSP_tree& operator=(const BSP_tree& other);

    BSP_tree& operator=(BSP_tree&& other) noexcept;

    ~BSP_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bsptree_iterator;
    class bsptree_const_iterator;

    class bsptree_iterator final
    {
        bsptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bsptree_iterator;

        friend class BSP_tree;
        friend class bsptree_const_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bsptree_iterator(bsptree_node_term* node = nullptr, size_t index = 0);

    };

    class bsptree_const_iterator final
    {
        const bsptree_node_term* _node;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bsptree_const_iterator;

        friend class BSP_tree;
        friend class bsptree_iterator;

        bsptree_const_iterator(const bsptree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bsptree_const_iterator(const bsptree_node_term* node = nullptr, size_t index = 0);
    };

    friend class btree_iterator;
    friend class btree_const_iterator;

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

    bsptree_iterator begin();
    bsptree_iterator end();

    bsptree_const_iterator begin() const;
    bsptree_const_iterator end() const;

    bsptree_const_iterator cbegin() const;
    bsptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bsptree_iterator find(const tkey& key);
    bsptree_const_iterator find(const tkey& key) const;

    bsptree_iterator lower_bound(const tkey& key);
    bsptree_const_iterator lower_bound(const tkey& key) const;

    bsptree_iterator upper_bound(const tkey& key);
    bsptree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bsptree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bsptree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bsptree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bsptree_iterator insert_or_assign(const tree_data_type& data);
    bsptree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bsptree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bsptree_iterator erase(bsptree_iterator pos);
    bsptree_iterator erase(bsptree_const_iterator pos);

    bsptree_iterator erase(bsptree_iterator beg, bsptree_iterator en);
    bsptree_iterator erase(bsptree_const_iterator beg, bsptree_const_iterator en);


    bsptree_iterator erase(const tkey& key);

    // endregion modifiers declaration

    void split_1to2_term(bsptree_node_middle* parent, size_t index, bsptree_node_term* full_node);
    
    void split_1to2_middle(bsptree_node_middle* parent, size_t index, bsptree_node_middle* full_node);
    
    void split_2to3_middle(bsptree_node_middle* parent, bsptree_node_middle* left, bsptree_node_middle* right, size_t left_idx);
    
    void split_2to3_term(bsptree_node_middle* parent, bsptree_node_term* left, bsptree_node_term* right, size_t left_idx);
    
    void rotate_left_middle(bsptree_node_middle* left, bsptree_node_middle* right, bsptree_node_middle* parent, int left_idx);
    
    void rotate_right_middle(bsptree_node_middle* left, bsptree_node_middle* right, bsptree_node_middle* parent, int left_idx);
    
    void rotate_left_term(bsptree_node_term* left, bsptree_node_term* right, bsptree_node_middle* parent, int left_idx);
    
    void rotate_right_term(bsptree_node_term* left, bsptree_node_term* right, bsptree_node_middle* parent, int left_idx);
    
    void merge_2to1_term(bsptree_node_term* left, bsptree_node_term* right, bsptree_node_middle* parent, int left_idx);
    
    void merge_2to1_middle(bsptree_node_middle* left, bsptree_node_middle* right, bsptree_node_middle* parent, int left_idx);
    
    void merge_3to2_middle(bsptree_node_middle* left, bsptree_node_middle* middle, bsptree_node_middle* right, bsptree_node_middle* parent, int left_idx);
    
    void merge_3to2_term(bsptree_node_term* left, bsptree_node_term* middle, bsptree_node_term* right, bsptree_node_middle* parent, int left_idx);
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BSP_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BSP_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BSP_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::compare_pairs(const BSP_tree::tree_data_type &lhs,
                                                      const BSP_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}

// region bsptree_node_base implementation

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_base::bsptree_node_base() noexcept
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_term::bsptree_node_term() noexcept
    : bsptree_node_base()
{
    this->_is_terminated = true;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_node_middle::bsptree_node_middle() noexcept
    : bsptree_node_base()
{
    this->_is_terminated = false;
}

// region BSP_tree constructor implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BSP_tree<tkey, tvalue, compare, t>::value_type> BSP_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return _allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::bsptree_const_iterator(const bsptree_node_term *node,
    size_t index)
    : _node(node),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(const compare& cmp, pp_allocator<value_type> alloc)
    : compare(cmp),
    _allocator(alloc),
    _root(nullptr),
    _size(0)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(pp_allocator<value_type> alloc, const compare& cmp)
    : BSP_tree<tkey, tvalue, compare, t>::BSP_tree(cmp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc)
    : BSP_tree<tkey, tvalue, compare, t>::BSP_tree(cmp, alloc)
{
    for (auto it = begin; it != end; ++it) {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc)
    : BSP_tree<tkey, tvalue, compare, t>::BSP_tree(data.begin(), data.end(), cmp, alloc)
{
}

// endregion BSP_tree constructor implementations

// region BSP_tree copy and move constructors

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(const BSP_tree& other)
    : BSP_tree<tkey, tvalue, compare, t>::BSP_tree(static_cast<const compare&>(other), other._allocator)
{
    if (other._root == nullptr) {
        return;
    }
    _size = other._size;
    std::stack<bsptree_node_base**> copy_to;
    copy_to.push(&_root);
    std::stack<bsptree_node_base*> copy_from;
    copy_from.push(other._root);
    std::vector<bsptree_node_term*> term_nodes;
    while (copy_from.size() > 0) {
        bsptree_node_base* copying_from = copy_from.top();
        copy_from.pop();
        if (!copying_from->_is_terminated) {
            for (size_t i = 0; i < copying_from->_pointers.size(); ++i) {
                copy_from.push(copying_from->_pointers[i]);
            }
        }
        bsptree_node_base** copying_to = copy_to.top();
        copy_to.pop();
        if (!copying_from->_is_terminated) {
            *copying_to = _allocator.template new_object<bsptree_node_middle>();
            *copying_to->_keys = copying_from->_keys;
            *copying_to->_pointers.resize(copying_from->_pointers.size());
            for (size_t i = 0; i < copying_from->_pointers.size(); ++i) {
                copy_to.push(&copying_to->_pointers[i]);
            }
        } else {
            *copying_to = _allocator.template new_object<bsptree_node_term>();
            *copying_to->_data = copying_from->_data;
            term_nodes.push_back(copying_to);
        }
    }
    for (size_t i = 0; i + 1 < term_nodes.size(); ++i) {
        term_nodes[i]->_next = term_nodes[i + 1];
    }
    if (!term_nodes.empty()) {
        term_nodes.back()->_next = nullptr;
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::BSP_tree(BSP_tree&& other) noexcept
    : compare(std::move(static_cast<compare&>(other))),
      _allocator(std::move(other._allocator)),
      _root(other._root),
      _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

// endregion BSP_tree copy and move constructors

// region BSP_tree copy and move assignment operators

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>& BSP_tree<tkey, tvalue, compare, t>::operator=(const BSP_tree& other)
{
    if (this != &other) {
        BSP_tree<tkey, tvalue, compare, t> tmp(other);
        std::swap(static_cast<compare&>(*this), static_cast<compare&>(tmp));
        std::swap(_allocator, tmp._allocator);
        std::swap(_root, tmp._root);
        std::swap(_size, tmp._size);
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>& BSP_tree<tkey, tvalue, compare, t>::operator=(BSP_tree&& other) noexcept
{
    if (this != &other) {
        BSP_tree<tkey, tvalue, compare, t> tmp(std::move(other));
        std::swap(static_cast<compare&>(*this), static_cast<compare&>(tmp));
        std::swap(_allocator, tmp._allocator);
        std::swap(_root, tmp._root);
        std::swap(_size, tmp._size);
    }
    return *this;
}

// endregion BSP_tree copy and move assignment operators

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::~BSP_tree() noexcept
{
    clear();
}

// region BSP_tree iterators implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::bsptree_iterator(bsptree_node_term* node, size_t index)
    : _node(node),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::reference BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference&>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::pointer BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator& BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator++()
{
    ++_index;
    if (_index == _node->_data.size()) {
        _index = 0;
        _node = _node->_next;
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator==(const self& other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::current_node_keys_count() const noexcept
{
    return _node ? _node->size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::bsptree_const_iterator(const bsptree_iterator& it) noexcept
    : _node(it._node),
      _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::reference BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator*() const noexcept
{
    return reinterpret_cast<const reference&>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::pointer BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator& BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator++()
{
    ++_index;
    if (_index == _node->_data.size()) {
        _index = 0;
        _node = _node->_next;
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator==(const self& other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::operator!=(const self& other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::current_node_keys_count() const noexcept
{
    return _node ? _node->size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator::index() const noexcept
{
    return _index;
}

// endregion BSP_tree iterators implementations

// region BSP_tree element access implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end()) {
        throw std::out_of_range("Key not found.");
    }
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue& BSP_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end()) {
        throw std::out_of_range("Key not found.");
    }
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::operator[](const tkey& key)
{
    return emplace(std::make_pair(key, tvalue())).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue& BSP_tree<tkey, tvalue, compare, t>::operator[](tkey&& key)
{
    return emplace(std::make_pair(std::move(key), tvalue())).first->second;
}

// endregion BSP_tree element access implementations

// region BSP_tree iterator begins implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::begin()
{
    if (_root == nullptr || _size == 0) {
        return end();
    }
    bsptree_node_base* current_node = _root;
    while (!current_node->_is_terminated) {
        current_node = static_cast<bsptree_node_middle*>(current_node)->_pointers[0];
    }
    return bsptree_iterator(static_cast<bsptree_node_term*>(current_node), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::end()
{
    return bsptree_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::begin() const
{
    return cbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::end() const
{
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::cbegin() const
{
    if (_root == nullptr || _size == 0) {
        return end();
    }
    bsptree_node_base* current_node = _root;
    while (!current_node->_is_terminated) {
        current_node = static_cast<bsptree_node_middle*>(current_node)->_pointers[0];
    }
    return bsptree_const_iterator(static_cast<bsptree_node_term*>(current_node), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::cend() const
{
    return bsptree_const_iterator();
}

// endregion BSP_tree iterator begins implementations

// region BSP_tree lookup implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BSP_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return size() == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    auto it = lower_bound(key);
    if (it != end() && !compare_keys(key, it->first)) {
        return it;
    }
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    bsptree_iterator it = find(key);
    return bsptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr) {
        return end();
    }
    bsptree_node_base* node = _root;
    while (!node->_is_terminated) {
        auto& keys = static_cast<bsptree_node_middle*>(node)->_keys;
        size_t i = std::distance(keys.begin(), 
            std::upper_bound(keys.begin(), keys.end(), key,
                [this](const tkey& a, const tkey& b) {
                    return compare_keys(a, b);
                }));
        node = static_cast<bsptree_node_middle*>(node)->_pointers[i];
    }
    bsptree_node_term* node_as_term = static_cast<bsptree_node_term*>(node);
    size_t i = std::distance(node_as_term->_data.begin(), 
        std::lower_bound(node_as_term->_data.begin(), node_as_term->_data.end(), key, 
            [this](const auto& data, const tkey& key) {
                return compare_keys(data.first, key);
            }));
    if (i == node_as_term->_data.size()) {
        return end();
    }
    return bsptree_iterator(node_as_term, i);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    bsptree_iterator it = lower_bound(key);
    return bsptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    bsptree_iterator it = lower_bound(key);
    if (!compare_keys(key, it->first)) {
        ++it;
    }
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_const_iterator BSP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    bsptree_iterator it = upper_bound(key);
    return bsptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BSP_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

// endregion BSP_tree lookup implementations

// region BSP_tree modifiers implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (_root == nullptr) {
        return;
    }
    std::stack<bsptree_node_base*> nodes;
    nodes.push(_root);
    while (!nodes.empty()) {
        bsptree_node_base* current = nodes.top();
        nodes.pop();
        if (!current->_is_terminated) {
            for (bsptree_node_base* child : static_cast<bsptree_node_middle*>(current)->_pointers) {
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
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::insert(const tree_data_type& data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename ...Args>
std::pair<typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator, bool> BSP_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    auto [key, value] = data;
    if (_root == nullptr) {
        _root = _allocator.template new_object<bsptree_node_term>();
        auto* root_as_term = static_cast<bsptree_node_term*>(_root);
        root_as_term->_data.push_back(std::move(data));
        _size = 1;
        return std::make_pair(bsptree_iterator(root_as_term, 0), true);
    }
    
    std::stack<std::pair<bsptree_node_base**, size_t>> path;
    bsptree_node_base** node_ptr = &_root;
    bsptree_node_base* node = _root;
    path.push(std::make_pair(node_ptr, 0));
    while (!node->_is_terminated) {
        auto& keys = static_cast<bsptree_node_middle*>(node)->_keys;
        size_t i = std::distance(keys.begin(), 
            std::upper_bound(keys.begin(), keys.end(), key,
                [this](const tkey& a, const tkey& b) {
                    return compare_keys(a, b);
                }));
        auto& pointers = static_cast<bsptree_node_middle*>(node)->_pointers;
        node_ptr = &(pointers[i]);
        node = pointers[i];
        path.push(std::make_pair(node_ptr, i));
    }
    bsptree_node_term* node_as_term = static_cast<bsptree_node_term*>(node);
    size_t i = std::distance(node_as_term->_data.begin(), 
        std::lower_bound(node_as_term->_data.begin(), node_as_term->_data.end(), key, 
        [this](const auto& data, const tkey& key) {
            return compare_keys(data.first, key);
        }));
    
    if (i < node_as_term->_data.size() && node_as_term->_data[i].first == key) {
        node_as_term->_data[i].second = value;
        return std::make_pair(bsptree_iterator(static_cast<bsptree_node_term*>(node), i), false);
    }
    node_as_term->_data.insert(node_as_term->_data.begin() + i, std::move(data));
    ++_size;

    while (node->size() > maximum_keys_in_node) {
        if (path.size() == 1) {
            bsptree_node_middle* new_root = _allocator.template new_object<bsptree_node_middle>();
            new_root->_pointers.push_back(node);
            if (node->_is_terminated) {
                split_1to2_term(new_root, 0, static_cast<bsptree_node_term*>(node));
            } else {
                split_1to2_middle(new_root, 0, static_cast<bsptree_node_middle*>(node));
            }
            _root = new_root;
            break;
        }
        auto idx_in_parent = path.top().second;
        path.pop();
        auto* parent = static_cast<bsptree_node_middle*>(*path.top().first);
        if (idx_in_parent > 0) {
            auto* left = parent->_pointers[idx_in_parent - 1];
            if (left->size() < maximum_keys_in_node) {
                if (node->_is_terminated) {
                    rotate_left_term(
                        static_cast<bsptree_node_term*>(left),
                        static_cast<bsptree_node_term*>(node),
                        static_cast<bsptree_node_middle*>(parent),
                        idx_in_parent - 1);
                } else {
                    rotate_left_middle(
                        static_cast<bsptree_node_middle*>(left),
                        static_cast<bsptree_node_middle*>(node),
                        static_cast<bsptree_node_middle*>(parent),
                        idx_in_parent - 1);
                }
                continue;
            }
        }
        if (idx_in_parent + 1 < parent->_pointers.size()) {
            auto* right = parent->_pointers[idx_in_parent + 1];
            if (right->size() < maximum_keys_in_node) {
                if (node->_is_terminated) {
                    rotate_right_term(
                        static_cast<bsptree_node_term*>(node),
                        static_cast<bsptree_node_term*>(right),
                        static_cast<bsptree_node_middle*>(parent),
                        idx_in_parent);
                } else {
                    rotate_right_middle(
                        static_cast<bsptree_node_middle*>(node),
                        static_cast<bsptree_node_middle*>(right),
                        static_cast<bsptree_node_middle*>(parent),
                        idx_in_parent);
                }
                continue;
            }
        }
        if (idx_in_parent > 0) {
            auto* left = parent->_pointers[idx_in_parent - 1];
            if (node->_is_terminated) {
                split_2to3_term(
                    static_cast<bsptree_node_middle*>(parent),
                    static_cast<bsptree_node_term*>(left),
                    static_cast<bsptree_node_term*>(node),
                    idx_in_parent - 1);
            } else {
                split_2to3_middle(
                    static_cast<bsptree_node_middle*>(parent),
                    static_cast<bsptree_node_middle*>(left),
                    static_cast<bsptree_node_middle*>(node),
                    idx_in_parent - 1);
            }
            continue;
        }
        if (idx_in_parent + 1 < parent->_pointers.size()) {
            auto* right = parent->_pointers[idx_in_parent + 1];
            if (node->_is_terminated) {
                split_2to3_term(
                    static_cast<bsptree_node_middle*>(parent),
                    static_cast<bsptree_node_term*>(node),
                    static_cast<bsptree_node_term*>(right),
                    idx_in_parent);
            } else {
                split_2to3_middle(
                    static_cast<bsptree_node_middle*>(parent),
                    static_cast<bsptree_node_middle*>(node),
                    static_cast<bsptree_node_middle*>(right),
                    idx_in_parent);
            }
            continue;
        }
        node = parent;
    }
    return std::make_pair(find(key), true);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::split_1to2_term(bsptree_node_middle* parent,
    size_t index, bsptree_node_term* full_node)
{
    bsptree_node_term* new_node = _allocator.template new_object<bsptree_node_term>();
    auto mid_idx = t;
    new_node->_data.assign(
        std::make_move_iterator(full_node->_data.begin() + mid_idx),
        std::make_move_iterator(full_node->_data.end()));
    parent->_keys.insert(parent->_keys.begin() + index, new_node->_data[0].first);
    parent->_pointers.insert(parent->_pointers.begin() + index + 1, new_node);
    full_node->_data.erase(full_node->_data.begin() + mid_idx, full_node->_data.end());
    new_node->_next = full_node->_next;
    full_node->_next = new_node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::split_1to2_middle(bsptree_node_middle* parent,
    size_t index, bsptree_node_middle* full_node)
{
    auto mid_idx = t;
    bsptree_node_middle* new_node = _allocator.template new_object<bsptree_node_middle>();
    parent->_keys.insert(parent->_keys.begin() + index, std::move(full_node->_keys[mid_idx]));
    new_node->_keys.assign(
        std::make_move_iterator(full_node->_keys.begin() + mid_idx + 1),
        std::make_move_iterator(full_node->_keys.end()));
    full_node->_keys.erase(full_node->_keys.begin() + mid_idx, full_node->_keys.end());
    new_node->_pointers.assign(full_node->_pointers.begin() + mid_idx + 1, full_node->_pointers.end());
    full_node->_pointers.erase(full_node->_pointers.begin() + mid_idx + 1, full_node->_pointers.end());
    parent->_pointers.insert(parent->_pointers.begin() + index + 1, new_node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::split_2to3_middle(bsptree_node_middle* parent, bsptree_node_middle* left,
    bsptree_node_middle* right, size_t left_idx)
{
    bsptree_node_middle* new_node = _allocator.template new_object<bsptree_node_middle>();
    auto total = left->size() + 1 + right->size();
    auto s1 = (total - 2) / 3;
    auto s2 = (total - 2 - s1) / 2;
    auto s3 = total - 2 - s1 - s2;
    new_node->_keys.assign(
        std::make_move_iterator(right->_keys.end() - s3),
        std::make_move_iterator(right->_keys.end()));
    parent->_keys.insert(parent->_keys.begin() + left_idx + 1,
                         std::move(right->_keys[right->size() - s3 - 1]));
    right->_keys.erase(right->_keys.end() - s3 - 1, right->_keys.end());
    right->_keys.insert(right->_keys.begin(), std::move(parent->_keys[left_idx]));
    auto to_s2 = s2 - right->size();
    right->_keys.insert(right->_keys.begin(),
        std::make_move_iterator(left->_keys.end() - to_s2),
        std::make_move_iterator(left->_keys.end()));
    parent->_keys[left_idx] = std::move(left->_keys[left->size() - to_s2 - 1]);
    left->_keys.erase(left->_keys.end() - to_s2 - 1, left->_keys.end());
    auto s1_p = s1 + 1;
    auto s2_p = s2 + 1;
    auto s3_p = s3 + 1;
    new_node->_pointers.assign(right->_pointers.end() - s3_p, right->_pointers.end());
    right->_pointers.erase(right->_pointers.end() - s3_p, right->_pointers.end());
    auto to_s2_p = s2_p - right->_pointers.size();
    right->_pointers.insert(right->_pointers.begin(), left->_pointers.end() - to_s2_p, left->_pointers.end());
    left->_pointers.erase(left->_pointers.end() - to_s2_p, left->_pointers.end());
    parent->_pointers.insert(parent->_pointers.begin() + left_idx + 2, new_node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::split_2to3_term(bsptree_node_middle* parent, bsptree_node_term* left,
    bsptree_node_term* right, size_t left_idx)
{
    bsptree_node_term* new_node = _allocator.template new_object<bsptree_node_term>();
    auto total = left->_data.size() + right->_data.size();
    auto s1 = total / 3;
    auto s2 = (total - s1) / 2;
    auto s3 = total - s1 - s2;
    new_node->_data.assign(
        std::make_move_iterator(right->_data.end() - s3),
        std::make_move_iterator(right->_data.end()));
    right->_data.erase(right->_data.end() - s3, right->_data.end());
    auto to_s2 = s2 - right->_data.size();
    right->_data.insert(right->_data.begin(),
        std::make_move_iterator(left->_data.end() - to_s2),
        std::make_move_iterator(left->_data.end()));
    left->_data.erase(left->_data.end() - to_s2, left->_data.end());
    parent->_keys[left_idx] = right->_data[0].first;
    parent->_keys.insert(parent->_keys.begin() + left_idx + 1, new_node->_data[0].first);
    parent->_pointers.insert(parent->_pointers.begin() + left_idx + 2, new_node);
    new_node->_next = right->_next;
    right->_next = new_node;
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::rotate_left_middle(bsptree_node_middle* left, bsptree_node_middle* right,
    bsptree_node_middle* parent, int left_idx)
{
    left->_keys.push_back(std::move(parent->_keys[left_idx]));
    parent->_keys[left_idx] = std::move(right->_keys.front());
    right->_keys.erase(right->_keys.begin());
    left->_pointers.push_back(right->_pointers.front());
    right->_pointers.erase(right->_pointers.begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::rotate_right_middle(bsptree_node_middle* left, bsptree_node_middle* right,
    bsptree_node_middle* parent, int left_idx)
{
    right->_keys.insert(right->_keys.begin(), std::move(parent->_keys[left_idx]));
    parent->_keys[left_idx] = std::move(left->_keys.back());
    left->_keys.pop_back();
    right->_pointers.insert(right->_pointers.begin(), left->_pointers.back());
    left->_pointers.pop_back();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::rotate_left_term(bsptree_node_term* left, bsptree_node_term* right,
    bsptree_node_middle* parent, int left_idx)
{
    left->_data.push_back(std::move(right->_data.front()));
    right->_data.erase(right->_data.begin());
    parent->_keys[left_idx] = right->_data.front().first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::rotate_right_term(bsptree_node_term* left, bsptree_node_term* right,
    bsptree_node_middle* parent, int left_idx)
{
    right->_data.insert(right->_data.begin(), std::move(left->_data.back()));
    left->_data.pop_back();
    parent->_keys[left_idx] = right->_data.front().first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    return emplace_or_assign(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    return emplace_or_assign(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<typename ...Args>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    auto it = lower_bound(data.first);
    if (it != end() && !compare_keys(data.first, it->first)) {
        it._node->_data[it._index].second = std::move(data.second);
        return it;
    }
    return insert(std::move(data)).first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_iterator pos)
{
    return erase(bsptree_const_iterator(pos));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_const_iterator pos)
{
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_iterator beg, bsptree_iterator en)
{
    return erase(bsptree_const_iterator(beg), bsptree_const_iterator(en));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(bsptree_const_iterator beg, bsptree_const_iterator en)
{
    std::vector<tkey> keys;
    for (auto it = beg; it != en; ++it) {
        keys.emplace_back(it->first);
    }
    bsptree_iterator it = end();
    for (const auto& key : keys) {
        it = erase(key);
    }
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BSP_tree<tkey, tvalue, compare, t>::bsptree_iterator BSP_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    std::stack<std::pair<bsptree_node_base**, size_t>> path;
    bsptree_node_base** node_ptr = &_root;
    bsptree_node_base* node = _root;
    path.push(std::make_pair(node_ptr, 0));
    while (!node->_is_terminated) {
        auto& keys = static_cast<bsptree_node_middle*>(node)->_keys;
        size_t i = std::distance(keys.begin(), 
            std::upper_bound(keys.begin(), keys.end(), key,
                [this](const tkey& a, const tkey& b) {
                    return compare_keys(a, b);
                }));
        auto& pointers = static_cast<bsptree_node_middle*>(node)->_pointers;
        node_ptr = &(pointers[i]);
        node = pointers[i];
        path.push(std::make_pair(node_ptr, i));
    }
    bsptree_node_term* node_as_term = static_cast<bsptree_node_term*>(node);
    size_t i = std::distance(node_as_term->_data.begin(), 
        std::lower_bound(node_as_term->_data.begin(), node_as_term->_data.end(), key, 
        [this](const auto& data, const tkey& key) {
            return compare_keys(data.first, key);
        }));
    
    if (i == node_as_term->_data.size()) {
        return end();
    }

    bool has_next = false;
    tkey next_key;
    if (i + 1 < node_as_term->_data.size()) {
        has_next = true;
        next_key = node_as_term->_data[i + 1].first;
    }

    node_as_term->_data.erase(node_as_term->_data.begin() + i);
    while (path.size() > 1) {
        bsptree_node_base* cur_node = *path.top().first;
        auto cur_node_idx = path.top().second;
        path.pop();
        auto* parent = static_cast<bsptree_node_middle*>(*path.top().first);
        if (cur_node->_is_terminated) {
            auto* cur_node_as_term = static_cast<bsptree_node_term*>(cur_node);
            if (cur_node_as_term->_data.size() >= minimum_keys_in_node) {
                break;
            }
            if (parent == _root && parent->_pointers.size() == 2) {
                if (cur_node_idx == 0) {
                    auto* right = static_cast<bsptree_node_term*>(parent->_pointers[1]);
                    if (right->size() > minimum_keys_in_node) {
                        rotate_left_term(cur_node_as_term, right, parent, 0);
                    } else {
                        merge_2to1_term(cur_node_as_term, right, parent, 0);
                    }
                } else {
                    auto* left = static_cast<bsptree_node_term*>(parent->_pointers[0]);
                    if (left->size() > minimum_keys_in_node) {
                        rotate_right_term(left, cur_node_as_term, parent, 0);
                    } else {
                        merge_2to1_term(left, cur_node_as_term, parent, 0);
                    }
                }
                break;
            }
            if (cur_node_idx == 0) {
                auto* right = static_cast<bsptree_node_term*>(parent->_pointers[cur_node_idx + 1]);
                auto* right_right = static_cast<bsptree_node_term*>(parent->_pointers[cur_node_idx + 2]);
                if (right->size() > minimum_keys_in_node) {
                    rotate_left_term(cur_node_as_term, right, parent, cur_node_idx);
                    continue;
                }
                if (right_right->size() > minimum_keys_in_node) {
                    rotate_left_term(right, right_right, parent, cur_node_idx + 1);
                    rotate_left_term(cur_node_as_term, right, parent, cur_node_idx);
                    continue;
                }
                merge_3to2_term(cur_node_as_term, right, right_right, parent, cur_node_idx);
                continue;
            }
            if (cur_node_idx == parent->_pointers.size() - 1) {
                auto* left = static_cast<bsptree_node_term*>(parent->_pointers[cur_node_idx - 1]);
                auto* left_left = static_cast<bsptree_node_term*>(parent->_pointers[cur_node_idx - 2]);
                if (left->size() > minimum_keys_in_node) {
                    rotate_right_term(left, cur_node_as_term, parent, cur_node_idx - 1);
                    continue;
                }
                if (left_left->size() > minimum_keys_in_node) {
                    rotate_left_term(left_left, left, parent, cur_node_idx - 2);
                    rotate_left_term(left, cur_node_as_term, parent, cur_node_idx - 1);
                    continue;
                }
                merge_3to2_term(left_left, left, cur_node_as_term, parent, cur_node_idx - 2);
                continue;
            }
            auto* right = static_cast<bsptree_node_term*>(parent->_pointers[cur_node_idx + 1]);
            if (right->size() > minimum_keys_in_node) {
                rotate_left_term(cur_node_as_term, right, parent, cur_node_idx);
                continue;
            }
            auto* left = static_cast<bsptree_node_term*>(parent->_pointers[cur_node_idx - 1]);
            if (left->size() > minimum_keys_in_node) {
                rotate_right_term(left, cur_node_as_term, parent, cur_node_idx - 1);
                continue;
            }
            merge_3to2_term(left, cur_node_as_term, right, parent, cur_node_idx - 1);
        } else {
            auto* cur_node_as_middle = static_cast<bsptree_node_middle*>(cur_node);
            if (cur_node_as_middle->size() >= minimum_keys_in_node) {
                break;
            }
            if (parent == _root && parent->_pointers.size() == 2) {
                if (cur_node_idx == 0) {
                    auto* right = static_cast<bsptree_node_middle*>(parent->_pointers[1]);
                    if (right->size() > minimum_keys_in_node) {
                        rotate_left_middle(cur_node_as_middle, right, parent, 0);
                    } else {
                        merge_2to1_middle(cur_node_as_middle, right, parent, 0);
                    }
                } else {
                    auto* left = static_cast<bsptree_node_middle*>(parent->_pointers[0]);
                    if (left->size() > minimum_keys_in_node) {
                        rotate_right_middle(left, cur_node_as_middle, parent, 0);
                    } else {
                        merge_2to1_middle(left, cur_node_as_middle, parent, 0);
                    }
                }
                break;
            }
            if (cur_node_idx == 0) {
                auto* right = static_cast<bsptree_node_middle*>(parent->_pointers[cur_node_idx + 1]);
                auto* right_right = static_cast<bsptree_node_middle*>(parent->_pointers[cur_node_idx + 2]);
                if (right->size() > minimum_keys_in_node) {
                    rotate_left_middle(cur_node_as_middle, right, parent, cur_node_idx);
                    continue;
                }
                if (right_right->size() > minimum_keys_in_node) {
                    rotate_left_middle(right, right_right, parent, cur_node_idx + 1);
                    rotate_left_middle(cur_node_as_middle, right, parent, cur_node_idx);
                    continue;
                }
                merge_3to2_middle(cur_node_as_middle, right, right_right, parent, cur_node_idx);
                continue;
            }
            if (cur_node_idx == parent->_pointers.size() - 1) {
                auto* left = static_cast<bsptree_node_middle*>(parent->_pointers[cur_node_idx - 1]);
                auto* left_left = static_cast<bsptree_node_middle*>(parent->_pointers[cur_node_idx - 2]);
                if (left->size() > minimum_keys_in_node) {
                    rotate_right_middle(left, cur_node_as_middle, parent, cur_node_idx - 1);
                    continue;
                }
                if (left_left->size() > minimum_keys_in_node) {
                    rotate_left_middle(left_left, left, parent, cur_node_idx - 2);
                    rotate_left_middle(left, cur_node_as_middle, parent, cur_node_idx - 1);
                    continue;
                }
                merge_3to2_middle(left_left, left, cur_node_as_middle, parent, cur_node_idx - 2);
                continue;
            }
            auto* right = static_cast<bsptree_node_middle*>(parent->_pointers[cur_node_idx + 1]);
            if (right->size() > minimum_keys_in_node) {
                rotate_left_middle(cur_node_as_middle, right, parent, cur_node_idx);
                continue;
            }
            auto* left = static_cast<bsptree_node_middle*>(parent->_pointers[cur_node_idx - 1]);
            if (left->size() > minimum_keys_in_node) {
                rotate_right_middle(left, cur_node_as_middle, parent, cur_node_idx - 1);
                continue;
            }
            merge_3to2_middle(left, cur_node_as_middle, right, parent, cur_node_idx - 1);
        }
    }
    if (_root && _root->size() == 0) {
        if (!_root->_is_terminated) {
            bsptree_node_base* old_root = _root;
            _root = static_cast<bsptree_node_middle*>(_root)->_pointers[0];
            _allocator.delete_object(old_root);
        } else {
            _root = nullptr;
        }
    }
    --_size;
    return has_next ? find(next_key) : end();
}

// endregion BSP_tree modifiers implementations

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::merge_2to1_term(bsptree_node_term* left,
        bsptree_node_term* right, bsptree_node_middle* parent, int left_idx) {
    left->_data.insert(left->_data.end(),
        std::make_move_iterator(right->_data.begin()),
        std::make_move_iterator(right->_data.end()));
    left->_next = right->_next;
    parent->_keys.erase(parent->_keys.begin() + left_idx);
    parent->_pointers.erase(parent->_pointers.begin() + left_idx + 1);
    left->_next = right->_next;
    _allocator.delete_object(right);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::merge_2to1_middle(bsptree_node_middle* left,
        bsptree_node_middle* right, bsptree_node_middle* parent, int left_idx) {
    left->_keys.push_back(std::move(parent->_keys[left_idx]));
    left->_keys.insert(left->_keys.end(),
        std::make_move_iterator(right->_keys.begin()),
        std::make_move_iterator(right->_keys.end()));
    left->_pointers.insert(left->_pointers.end(), right->_pointers.begin(), right->_pointers.end());
    _allocator.delete_object(right);
    parent->_keys.erase(parent->_keys.begin() + left_idx);
    parent->_pointers.erase(parent->_pointers.begin() + left_idx + 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::merge_3to2_middle(bsptree_node_middle* left, bsptree_node_middle* middle,
    bsptree_node_middle* right, bsptree_node_middle* parent, int left_idx)
{
    auto total = left->size() + middle->size() + right->size() + 2;
    auto s1 = (total - 1) / 2;
    auto s2 = total - 1 - s1;
    left->_keys.push_back(std::move(parent->_keys[left_idx]));
    auto to_s1 = s1 - left->size();
    left->_keys.insert(left->_keys.end(),
        std::make_move_iterator(middle->_keys.begin()),
        std::make_move_iterator(middle->_keys.begin() + to_s1));
    parent->_keys[left_idx] = std::move(middle->_keys[to_s1]);
    middle->_keys.erase(middle->_keys.begin(), middle->_keys.begin() + to_s1 + 1);
    middle->_keys.insert(middle->_keys.end(),
        std::make_move_iterator(right->_keys.begin()),
        std::make_move_iterator(right->_keys.end()));
    auto s1_p = s1 + 1;
    auto s2_p = s2 + 1;
    auto to_s1_p = s1_p - left->_pointers.size();
    left->_pointers.insert(left->_pointers.end(), middle->_pointers.begin(), middle->_pointers.begin() + to_s1_p);
    middle->_pointers.erase(middle->_pointers.begin(), middle->_pointers.begin() + to_s1_p);
    middle->_pointers.insert(middle->_pointers.end(), right->_pointers.begin(), right->_pointers.end());
    parent->_keys.erase(parent->_keys.begin() + left_idx + 1);
    parent->_pointers.erase(parent->_pointers.begin() + left_idx + 2);
    _allocator.delete_object(right);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BSP_tree<tkey, tvalue, compare, t>::merge_3to2_term(bsptree_node_term* left, bsptree_node_term* middle,
    bsptree_node_term* right, bsptree_node_middle* parent, int left_idx)
{
    auto total = left->_data.size() + middle->_data.size() + right->_data.size();
    auto s1 = total / 2;
    auto s2 = total - s1;
    auto to_s1 = s1 - left->_data.size();
    left->_data.insert(left->_data.end(),
        std::make_move_iterator(middle->_data.begin()),
        std::make_move_iterator(middle->_data.begin() + to_s1));
    middle->_data.erase(middle->_data.begin(), middle->_data.begin() + to_s1);
    middle->_data.insert(middle->_data.end(),
        std::make_move_iterator(right->_data.begin()),
        std::make_move_iterator(right->_data.end()));
    parent->_keys.erase(parent->_keys.begin() + left_idx + 1);
    parent->_pointers.erase(parent->_pointers.begin() + left_idx + 2);
    parent->_keys[left_idx] = middle->_data[0].first;
    middle->_next = right->_next;
    _allocator.delete_object(right);
}

#endif
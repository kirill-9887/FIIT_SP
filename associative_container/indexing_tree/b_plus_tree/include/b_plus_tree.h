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

#ifndef SYS_PROG_B_PLUS_TREE_H
#define SYS_PROG_B_PLUS_TREE_H

template <typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5>
class BP_tree final : private compare //EBCO
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

    struct bptree_node_base
    {
        bool _is_terminate;
        virtual size_t size() = 0;

        bptree_node_base() noexcept;
        virtual ~bptree_node_base() =default;
    };

    struct bptree_node_term : public bptree_node_base
    {
        bptree_node_term* _next;

        boost::container::static_vector<tree_data_type, maximum_keys_in_node + 1> _data;
        bptree_node_term() noexcept;

        size_t size() override {
            return _data.size();
        }
    };

    struct bptree_node_middle : public bptree_node_base
    {
        boost::container::static_vector<tkey, maximum_keys_in_node + 1> _keys;
        boost::container::static_vector<bptree_node_base*, maximum_keys_in_node + 2> _pointers;
        bptree_node_middle() noexcept;

        size_t size() override {
            return _keys.size();
        }
    };

    pp_allocator<value_type> _allocator;
    bptree_node_base* _root;
    size_t _size;

    pp_allocator<value_type> get_allocator() const noexcept;

public:

    // region constructors declaration

    explicit BP_tree(const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    explicit BP_tree(pp_allocator<value_type> alloc, const compare& comp = compare());

    template<input_iterator_for_pair<tkey, tvalue> iterator>
    explicit BP_tree(iterator begin, iterator end, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp = compare(), pp_allocator<value_type> = pp_allocator<value_type>());

    // endregion constructors declaration

    // region five declaration

    BP_tree(const BP_tree& other);

    BP_tree(BP_tree&& other) noexcept;

    BP_tree& operator=(const BP_tree& other);

    BP_tree& operator=(BP_tree&& other) noexcept;

    ~BP_tree() noexcept;

    // endregion five declaration

    // region iterators declaration

    class bptree_iterator;
    class bptree_const_iterator;

    class bptree_iterator final
    {
        bptree_node_term* _node;
        size_t _index;

    public:
        using value_type = tree_data_type_const;
        using reference = value_type&;
        using pointer = value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_iterator;

        friend class BP_tree;
        friend class bptree_const_iterator;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bptree_iterator(bptree_node_term* node = nullptr, size_t index = 0);

    };

    class bptree_const_iterator final
    {
        const bptree_node_term* _node;
        size_t _index;

    public:

        using value_type = tree_data_type_const;
        using reference = const value_type&;
        using pointer = const value_type*;
        using iterator_category = std::forward_iterator_tag;
        using difference_type = ptrdiff_t;
        using self = bptree_const_iterator;

        friend class BP_tree;
        friend class bptree_iterator;

        bptree_const_iterator(const bptree_iterator& it) noexcept;

        reference operator*() const noexcept;
        pointer operator->() const noexcept;

        self& operator++();
        self operator++(int);

        bool operator==(const self& other) const noexcept;
        bool operator!=(const self& other) const noexcept;

        size_t current_node_keys_count() const noexcept;
        size_t index() const noexcept;

        explicit bptree_const_iterator(const bptree_node_term* node = nullptr, size_t index = 0);
    };

    friend class bptree_iterator;
    friend class bptree_const_iterator;

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

    bptree_iterator begin();
    bptree_iterator end();

    bptree_const_iterator begin() const;
    bptree_const_iterator end() const;

    bptree_const_iterator cbegin() const;
    bptree_const_iterator cend() const;

    // endregion iterator begins declaration

    // region lookup declaration

    size_t size() const noexcept;
    bool empty() const noexcept;

    /*
     * Returns end() if not exist
     */

    bptree_iterator find(const tkey& key);
    bptree_const_iterator find(const tkey& key) const;

    bptree_iterator lower_bound(const tkey& key);
    bptree_const_iterator lower_bound(const tkey& key) const;

    bptree_iterator upper_bound(const tkey& key);
    bptree_const_iterator upper_bound(const tkey& key) const;

    bool contains(const tkey& key) const;

    // endregion lookup declaration

    // region modifiers declaration

    void clear() noexcept;

    /*
     * Does nothing if key exists, delegates to emplace.
     * Second return value is true, when inserted
     */
    std::pair<bptree_iterator, bool> insert(const tree_data_type& data);
    std::pair<bptree_iterator, bool> insert(tree_data_type&& data);

    template <typename ...Args>
    std::pair<bptree_iterator, bool> emplace(Args&&... args);

    /*
     * Updates value if key exists, delegates to emplace.
     */
    bptree_iterator insert_or_assign(const tree_data_type& data);
    bptree_iterator insert_or_assign(tree_data_type&& data);

    template <typename ...Args>
    bptree_iterator emplace_or_assign(Args&&... args);

    /*
     * Return iterator to node next ro removed or end() if key not exists
     */
    bptree_iterator erase(bptree_iterator pos);
    bptree_iterator erase(bptree_const_iterator pos);

    bptree_iterator erase(bptree_iterator beg, bptree_iterator en);
    bptree_iterator erase(bptree_const_iterator beg, bptree_const_iterator en);


    bptree_iterator erase(const tkey& key);

    // endregion modifiers declaration

    // region bptree_balancing declaration

    void split_term(bptree_node_middle* parent, size_t index, bptree_node_term* full_node);

    void split_middle(bptree_node_middle* parent, size_t index, bptree_node_middle* full_node);
    
    void left_rotation_term(bptree_node_term* node, bptree_node_term* right, bptree_node_middle* parent, int node_idx);
    
    void right_rotation_term(bptree_node_term* left, bptree_node_term* node, bptree_node_middle* parent, int node_idx);
    
    void left_rotation_middle(bptree_node_middle* node, bptree_node_middle* right, bptree_node_middle* parent, int node_idx);
    
    void right_rotation_middle(bptree_node_middle* left, bptree_node_middle* node, bptree_node_middle* parent, int node_idx);
    
    void merge_nodes_term(bptree_node_term* left, bptree_node_term* right, bptree_node_middle* parent, int left_idx);

    void merge_nodes_middle(bptree_node_middle* left, bptree_node_middle* right, bptree_node_middle* parent, int left_idx);
    
    // endregion bptree_balancing declaration
};

template<std::input_iterator iterator, comparator<typename std::iterator_traits<iterator>::value_type::first_type> compare = std::less<typename std::iterator_traits<iterator>::value_type::first_type>,
        std::size_t t = 5, typename U>
BP_tree(iterator begin, iterator end, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BP_tree<typename std::iterator_traits<iterator>::value_type::first_type, typename std::iterator_traits<iterator>::value_type::second_type, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare = std::less<tkey>, std::size_t t = 5, typename U>
BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare &cmp = compare(), pp_allocator<U> = pp_allocator<U>()) -> BP_tree<tkey, tvalue, compare, t>;

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_pairs(const BP_tree::tree_data_type &lhs,
                                                     const BP_tree::tree_data_type &rhs) const
{
    return compare_keys(lhs.first, rhs.first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_base::bptree_node_base() noexcept
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_term::bptree_node_term() noexcept
    : bptree_node_base()
{
    this->_is_terminate = true;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_node_middle::bptree_node_middle() noexcept
    : bptree_node_base()
{
    this->_is_terminate = false;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
pp_allocator<typename BP_tree<tkey, tvalue, compare, t>::value_type> BP_tree<tkey, tvalue, compare, t>::
get_allocator() const noexcept
{
    return _allocator;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_iterator::operator*() const noexcept
{
    return reinterpret_cast<reference&>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::pointer BP_tree<tkey, tvalue, compare, t>::bptree_iterator
::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self & BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++()
{
    ++_index;
    if (_index == _node->_data.size()) {
        _index = 0;
        _node = _node->_next;
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator::self BP_tree<tkey, tvalue, compare, t>::bptree_iterator::
operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator==(const self &other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::current_node_keys_count() const noexcept
{
    return _node ? _node->_keys.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_iterator::bptree_iterator(bptree_node_term *node, size_t index)
    : _node(node),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_iterator &it) noexcept
    : _node(it._node),
      _index(it._index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::reference BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator*() const noexcept
{
    return reinterpret_cast<const reference&>(_node->_data[_index]);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::pointer BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator->() const noexcept
{
    return &(**this);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self & BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++()
{
    ++_index;
    if (_index == _node->_data.size()) {
        _index = 0;
        _node = _node->_next;
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::self BP_tree<tkey, tvalue, compare, t>::
bptree_const_iterator::operator++(int)
{
    self temp = *this;
    ++(*this);
    return temp;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator==(const self &other) const noexcept
{
    return _node == other._node && _index == other._index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::operator!=(const self &other) const noexcept
{
    return !(*this == other);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::current_node_keys_count() const noexcept
{
    return _node ? _node->_keys.size() : 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::index() const noexcept
{
    return _index;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator::bptree_const_iterator(const bptree_node_term *node, size_t index)
    : _node(node),
      _index(index)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::at(const tkey& key)
{
    auto it = find(key);
    if (it == end()) {
        throw std::out_of_range("Key not found.");
    }
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
const tvalue & BP_tree<tkey, tvalue, compare, t>::at(const tkey& key) const
{
    auto it = find(key);
    if (it == end()) {
        throw std::out_of_range("Key not found.");
    }
    return it->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::operator[](const tkey &key)
{
    return emplace(std::make_pair(key, tvalue())).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
tvalue & BP_tree<tkey, tvalue, compare, t>::operator[](tkey &&key)
{
    return emplace(std::make_pair(std::move(key), tvalue())).first->second;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::insert(
    const tree_data_type &data)
{
    return emplace(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::compare_keys(const tkey &lhs, const tkey &rhs) const
{
    return compare::operator()(lhs, rhs);
}


template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const compare& cmp, pp_allocator<value_type> alloc)
    : compare(cmp),
    _allocator(alloc),
    _root(nullptr),
    _size(0)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(pp_allocator<value_type> alloc, const compare& cmp)
    : BP_tree<tkey, tvalue, compare, t>::BP_tree(cmp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template<input_iterator_for_pair<tkey, tvalue> iterator>
BP_tree<tkey, tvalue, compare, t>::BP_tree(iterator begin, iterator end, const compare& cmp, pp_allocator<value_type> alloc)
    : BP_tree<tkey, tvalue, compare, t>::BP_tree(cmp, alloc)
{
    for (auto it = begin; it != end; ++it) {
        insert(*it);
    }
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(std::initializer_list<std::pair<tkey, tvalue>> data, const compare& cmp, pp_allocator<value_type> alloc)
    : BP_tree<tkey, tvalue, compare, t>::BP_tree(data.begin(), data.end(), cmp, alloc)
{
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::BP_tree(const BP_tree& other)
    : BP_tree<tkey, tvalue, compare, t>::BP_tree(static_cast<const compare&>(other), other._allocator)
{
    if (other._root == nullptr) {
        return;
    }
    _size = other._size;
    std::stack<bptree_node_base**> copy_to;
    copy_to.push(&_root);
    std::stack<bptree_node_base*> copy_from;
    copy_from.push(other._root);
    std::vector<bptree_node_term*> term_nodes;
    while (copy_from.size() > 0) {
        bptree_node_base* copying_from = copy_from.top();
        copy_from.pop();
        if (!copying_from->_is_terminate) {
            for (size_t i = 0; i < copying_from->_pointers.size(); ++i) {
                copy_from.push(copying_from->_pointers[i]);
            }
        }
        bptree_node_base** copying_to = copy_to.top();
        copy_to.pop();
        if (!copying_from->_is_terminate) {
            *copying_to = _allocator.template new_object<bptree_node_middle>();
            *copying_to->_keys = copying_from->_keys;
            *copying_to->_pointers.resize(copying_from->_pointers.size());
            for (size_t i = 0; i < copying_from->_pointers.size(); ++i) {
                copy_to.push(&copying_to->_pointers[i]);
            }
        } else {
            *copying_to = _allocator.template new_object<bptree_node_term>();
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
BP_tree<tkey, tvalue, compare, t>::BP_tree(BP_tree&& other) noexcept
    : compare(std::move(static_cast<compare&>(other))),
      _allocator(std::move(other._allocator)),
      _root(other._root),
      _size(other._size)
{
    other._root = nullptr;
    other._size = 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>& BP_tree<tkey, tvalue, compare, t>::operator=(const BP_tree& other)
{
    if (this != &other) {
        BP_tree<tkey, tvalue, compare, t> tmp(other);
        std::swap(static_cast<compare&>(*this), static_cast<compare&>(tmp));
        std::swap(_allocator, tmp._allocator);
        std::swap(_root, tmp._root);
        std::swap(_size, tmp._size);
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>& BP_tree<tkey, tvalue, compare, t>::operator=(BP_tree&& other) noexcept
{
    if (this != &other) {
        BP_tree<tkey, tvalue, compare, t> tmp(std::move(other));
        std::swap(static_cast<compare&>(*this), static_cast<compare&>(tmp));
        std::swap(_allocator, tmp._allocator);
        std::swap(_root, tmp._root);
        std::swap(_size, tmp._size);
    }
    return *this;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
BP_tree<tkey, tvalue, compare, t>::~BP_tree() noexcept
{
    clear();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::begin()
{
    if (_root == nullptr || _size == 0) {
        return end();
    }
    bptree_node_base* current_node = _root;
    while (!current_node->_is_terminate) {
        current_node = static_cast<bptree_node_middle*>(current_node)->_pointers[0];
    }
    return bptree_iterator(static_cast<bptree_node_term*>(current_node), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::end()
{
    return bptree_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::begin() const
{
    return cbegin();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::end() const
{
    return cend();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cbegin() const
{
    if (_root == nullptr || _size == 0) {
        return end();
    }
    bptree_node_base* current_node = _root;
    while (!current_node->_is_terminate) {
        current_node = static_cast<bptree_node_middle*>(current_node)->_pointers[0];
    }
    return bptree_const_iterator(static_cast<bptree_node_term*>(current_node), 0);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::cend() const
{
    return bptree_const_iterator();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
size_t BP_tree<tkey, tvalue, compare, t>::size() const noexcept
{
    return _size;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::empty() const noexcept
{
    return size() == 0;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey& key)
{
    auto it = lower_bound(key);
    if (it != end() && !compare_keys(key, it->first)) {
        return it;
    }
    return end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::find(const tkey& key) const
{
    bptree_iterator it = find(key);
    return bptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key)
{
    if (_root == nullptr) {
        return end();
    }
    bptree_node_base* node = _root;
    while (!node->_is_terminate) {
        auto& keys = static_cast<bptree_node_middle*>(node)->_keys;
        size_t i = std::distance(keys.begin(), 
            std::upper_bound(keys.begin(), keys.end(), key,
                [this](const tkey& a, const tkey& b) {
                    return compare_keys(a, b);
                }));
        node = static_cast<bptree_node_middle*>(node)->_pointers[i];
    }
    bptree_node_term* node_as_term = static_cast<bptree_node_term*>(node);
    size_t i = std::distance(node_as_term->_data.begin(), 
        std::lower_bound(node_as_term->_data.begin(), node_as_term->_data.end(), key, 
            [this](const auto& data, const tkey& key) {
                return compare_keys(data.first, key);
            }));
    if (i == node_as_term->_data.size()) {
        return end();
    }
    return bptree_iterator(node_as_term, i);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::lower_bound(const tkey& key) const
{
    bptree_iterator it = lower_bound(key);
    return bptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key)
{
    bptree_iterator it = lower_bound(key);
    if (!compare_keys(key, it->first)) {
        ++it;
    }
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_const_iterator BP_tree<tkey, tvalue, compare, t>::upper_bound(const tkey& key) const
{
    bptree_iterator it = upper_bound(key);
    return bptree_const_iterator(it);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
bool BP_tree<tkey, tvalue, compare, t>::contains(const tkey& key) const
{
    return find(key) != end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::clear() noexcept
{
    if (_root == nullptr) {
        return;
    }
    std::stack<bptree_node_base*> nodes;
    nodes.push(_root);
    while (!nodes.empty()) {
        bptree_node_base* current = nodes.top();
        nodes.pop();
        if (!current->_is_terminate) {
            for (bptree_node_base* child : static_cast<bptree_node_middle*>(current)->_pointers) {
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
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::insert(tree_data_type&& data)
{
    return emplace(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
std::pair<typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator, bool> BP_tree<tkey, tvalue, compare, t>::emplace(Args&&... args)
{
    tree_data_type data(std::forward<Args>(args)...);
    auto [key, value] = data;
    if (_root == nullptr) {
        _root = _allocator.template new_object<bptree_node_term>();
        auto* root_as_term = static_cast<bptree_node_term*>(_root);
        root_as_term->_data.push_back(std::move(data));
        _size = 1;
        return std::make_pair(bptree_iterator(root_as_term, 0), true);
    }
    
    std::stack<std::pair<bptree_node_base**, size_t>> path;
    bptree_node_base** node_ptr = &_root;
    bptree_node_base* node = _root;
    path.push(std::make_pair(node_ptr, 0));
    while (!node->_is_terminate) {
        auto& keys = static_cast<bptree_node_middle*>(node)->_keys;
        size_t i = std::distance(keys.begin(), 
            std::upper_bound(keys.begin(), keys.end(), key,
                [this](const tkey& a, const tkey& b) {
                    return compare_keys(a, b);
                }));
        auto& pointers = static_cast<bptree_node_middle*>(node)->_pointers;
        node_ptr = &(pointers[i]);
        node = pointers[i];
        path.push(std::make_pair(node_ptr, i));
    }
    bptree_node_term* node_as_term = static_cast<bptree_node_term*>(node);
    size_t i = std::distance(node_as_term->_data.begin(), 
        std::lower_bound(node_as_term->_data.begin(), node_as_term->_data.end(), key, 
        [this](const auto& data, const tkey& key) {
            return compare_keys(data.first, key);
        }));
    
    if (i < node_as_term->_data.size() && node_as_term->_data[i].first == key) {
        node_as_term->_data[i].second = value;
        return std::make_pair(bptree_iterator(static_cast<bptree_node_term*>(node), i), false);
    }
    node_as_term->_data.insert(node_as_term->_data.begin() + i, std::move(data));
    ++_size;

    while (node->size() > maximum_keys_in_node) {
        if (path.size() == 1) {
            bptree_node_middle* new_root = _allocator.template new_object<bptree_node_middle>();
            new_root->_pointers.push_back(node);
            if (node->_is_terminate) {
                split_term(new_root, 0, static_cast<bptree_node_term*>(node));
            } else {
                split_middle(new_root, 0, static_cast<bptree_node_middle*>(node));
            }
            _root = new_root;
            break;
        }
        auto index_in_parent = path.top().second;
        path.pop();
        auto parent = *path.top().first;
        if (node->_is_terminate) {
            split_term(static_cast<bptree_node_middle*>(parent), index_in_parent, static_cast<bptree_node_term*>(node));
        } else {
            split_middle(static_cast<bptree_node_middle*>(parent), index_in_parent, static_cast<bptree_node_middle*>(node));
        }
        node = parent;
    }
    return std::make_pair(find(key), true);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::split_term(bptree_node_middle* parent,
    size_t index, bptree_node_term* full_node)
{
    bptree_node_term* new_node = _allocator.template new_object<bptree_node_term>();
    new_node->_data.assign(
        std::make_move_iterator(full_node->_data.begin() + t),
        std::make_move_iterator(full_node->_data.end()));
    parent->_keys.insert(parent->_keys.begin() + index, new_node->_data[0].first);
    parent->_pointers.insert(parent->_pointers.begin() + index + 1, new_node);
    full_node->_data.erase(full_node->_data.begin() + t, full_node->_data.end());
    new_node->_next = full_node->_next;
    full_node->_next = new_node;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::split_middle(bptree_node_middle* parent,
    size_t index, bptree_node_middle* full_node)
{
    bptree_node_middle* new_node = _allocator.template new_object<bptree_node_middle>();
    parent->_keys.insert(parent->_keys.begin() + index, std::move(full_node->_keys[t]));
    new_node->_keys.assign(
        std::make_move_iterator(full_node->_keys.begin() + t + 1),
        std::make_move_iterator(full_node->_keys.end()));
    full_node->_keys.erase(full_node->_keys.begin() + t, full_node->_keys.end());
    new_node->_pointers.assign(full_node->_pointers.begin() + t + 1, full_node->_pointers.end());
    full_node->_pointers.erase(full_node->_pointers.begin() + t + 1, full_node->_pointers.end());
    parent->_pointers.insert(parent->_pointers.begin() + index + 1, new_node);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::insert_or_assign(const tree_data_type& data)
{
    return emplace_or_assign(data);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::insert_or_assign(tree_data_type&& data)
{
    return emplace_or_assign(std::move(data));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
template <typename ...Args>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::emplace_or_assign(Args&&... args)
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
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator pos)
{
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator pos)
{
    return erase(pos->first);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_iterator beg, bptree_iterator en)
{
    return erase(bptree_const_iterator(beg), bptree_const_iterator(en));
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(bptree_const_iterator beg, bptree_const_iterator en)
{
    std::vector<tkey> keys;
    for (auto it = beg; it != en; ++it) {
        keys.emplace_back(it->first);
    }
    bptree_iterator it = end();
    for (const auto& key : keys) {
        it = erase(key);
    }
    return it;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
typename BP_tree<tkey, tvalue, compare, t>::bptree_iterator BP_tree<tkey, tvalue, compare, t>::erase(const tkey& key)
{
    std::stack<std::pair<bptree_node_base**, size_t>> path;
    bptree_node_base** node_ptr = &_root;
    bptree_node_base* node = _root;
    path.push(std::make_pair(node_ptr, 0));
    while (!node->_is_terminate) {
        auto& keys = static_cast<bptree_node_middle*>(node)->_keys;
        size_t i = std::distance(keys.begin(), 
            std::upper_bound(keys.begin(), keys.end(), key,
                [this](const tkey& a, const tkey& b) {
                    return compare_keys(a, b);
                }));
        auto& pointers = static_cast<bptree_node_middle*>(node)->_pointers;
        node_ptr = &(pointers[i]);
        node = pointers[i];
        path.push(std::make_pair(node_ptr, i));
    }
    bptree_node_term* node_as_term = static_cast<bptree_node_term*>(node);
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
        bptree_node_base* cur_node = *path.top().first;
        auto cur_node_idx = path.top().second;
        path.pop();
        auto* parent = static_cast<bptree_node_middle*>(*path.top().first);
        if (cur_node->_is_terminate) {
            auto* cur_node_as_term = static_cast<bptree_node_term*>(cur_node);
            if (cur_node_as_term->_data.size() < minimum_keys_in_node) {
                if (cur_node_idx + 1 < parent->_pointers.size()) {
                    auto* right = static_cast<bptree_node_term*>(parent->_pointers[cur_node_idx + 1]);
                    if (right->_data.size() > minimum_keys_in_node) {
                        left_rotation_term(cur_node_as_term, right, parent, cur_node_idx);
                        continue;
                    }
                }
                if (cur_node_idx > 0) {
                    auto* left = static_cast<bptree_node_term*>(parent->_pointers[cur_node_idx - 1]);
                    if (left->_data.size() > minimum_keys_in_node) {
                        right_rotation_term(left, cur_node_as_term, parent, cur_node_idx);
                        continue;
                    }
                }
                if (cur_node_idx + 1 < parent->_pointers.size()) {
                    auto* right = static_cast<bptree_node_term*>(parent->_pointers[cur_node_idx + 1]);
                    merge_nodes_term(cur_node_as_term, right, parent, cur_node_idx);
                    continue;
                }
                if (cur_node_idx > 0) {
                    auto* left = static_cast<bptree_node_term*>(parent->_pointers[cur_node_idx - 1]);
                    merge_nodes_term(left, cur_node_as_term, parent, cur_node_idx - 1);
                    continue;
                }
            }
        } else {
            auto* cur_node_as_mid = static_cast<bptree_node_middle*>(cur_node);
            if (cur_node_as_mid->_keys.size() < minimum_keys_in_node) {
                if (cur_node_idx + 1 < parent->_pointers.size()) {
                    auto* right = static_cast<bptree_node_middle*>(parent->_pointers[cur_node_idx + 1]);
                    if (right->_keys.size() > minimum_keys_in_node) {
                        left_rotation_middle(cur_node_as_mid, right, parent, cur_node_idx);
                        continue;
                    }
                }
                if (cur_node_idx > 0) {
                    auto* left = static_cast<bptree_node_middle*>(parent->_pointers[cur_node_idx - 1]);
                    if (left->_keys.size() > minimum_keys_in_node) {
                        right_rotation_middle(left, cur_node_as_mid, parent, cur_node_idx);
                        continue;
                    }
                }
                if (cur_node_idx + 1 < parent->_pointers.size()) {
                    auto* right = static_cast<bptree_node_middle*>(parent->_pointers[cur_node_idx + 1]);
                    merge_nodes_middle(cur_node_as_mid, right, parent, cur_node_idx);
                    continue;
                }
                if (cur_node_idx > 0) {
                    auto* left = static_cast<bptree_node_middle*>(parent->_pointers[cur_node_idx - 1]);
                    merge_nodes_middle(left, cur_node_as_mid, parent, cur_node_idx - 1);
                    continue;
                }
            }
        }
    }
    if (_root && _root->size() == 0) {
        if (!_root->_is_terminate) {
            bptree_node_base* old_root = _root;
            _root = static_cast<bptree_node_middle*>(_root)->_pointers[0];
            _allocator.delete_object(old_root);
        } else {
            _root = nullptr;
        }
    }
    --_size;
    return has_next ? find(next_key) : end();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::left_rotation_term(bptree_node_term* node, 
        bptree_node_term* right, bptree_node_middle* parent, int node_idx) {
    assert(right->_data.size() > minimum_keys_in_node);
    node->_data.push_back(std::move(right->_data[0]));
    right->_data.erase(right->_data.begin());
    parent->_keys[node_idx] = right->_data[0].first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::right_rotation_term(bptree_node_term* left,
        bptree_node_term* node, bptree_node_middle* parent, int node_idx) {
    assert(left->_data.size() > minimum_keys_in_node);
    node->_data.insert(node->_data.begin(), std::move(left->_data.back()));
    left->_data.pop_back();
    assert(node_idx > 0);
    auto left_idx = node_idx - 1;
    parent->_keys[left_idx] = node->_data[0].first;
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::left_rotation_middle(bptree_node_middle* left,
        bptree_node_middle* right, bptree_node_middle* parent, int left_idx) {
    left->_keys.push_back(std::move(parent->_keys[left_idx]));
    parent->_keys[left_idx] = std::move(right->_keys[0]);
    right->_keys.erase(right->_keys.begin());
    left->_pointers.push_back(right->_pointers[0]);
    right->_pointers.erase(right->_pointers.begin());
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::right_rotation_middle(bptree_node_middle* left,
        bptree_node_middle* right,bptree_node_middle* parent, int right_idx) {
    right->_keys.insert(right->_keys.begin(), std::move(parent->_keys[right_idx - 1]));
    parent->_keys[right_idx - 1] = std::move(left->_keys.back());
    left->_keys.pop_back();
    right->_pointers.insert(right->_pointers.begin(), left->_pointers.back());
    left->_pointers.pop_back();
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::merge_nodes_term(bptree_node_term* left,
        bptree_node_term* right, bptree_node_middle* parent, int left_idx) {
    assert(left->_data.size() + right->_data.size() <= maximum_keys_in_node);
    left->_data.insert(left->_data.end(),
        std::make_move_iterator(right->_data.begin()),
        std::make_move_iterator(right->_data.end()));
    left->_next = right->_next;
    _allocator.delete_object(right);
    parent->_keys.erase(parent->_keys.begin() + left_idx);
    parent->_pointers.erase(parent->_pointers.begin() + left_idx + 1);
}

template<typename tkey, typename tvalue, comparator<tkey> compare, std::size_t t>
void BP_tree<tkey, tvalue, compare, t>::merge_nodes_middle(bptree_node_middle* left,
        bptree_node_middle* right, bptree_node_middle* parent, int left_idx) {
    assert(left->_keys.size() + 1 + right->_keys.size() <= maximum_keys_in_node);
    left->_keys.push_back(std::move(parent->_keys[left_idx]));
    left->_keys.insert(left->_keys.end(),
        std::make_move_iterator(right->_keys.begin()),
        std::make_move_iterator(right->_keys.end()));
    left->_pointers.insert(left->_pointers.end(), right->_pointers.begin(), right->_pointers.end());
    _allocator.delete_object(right);
    parent->_keys.erase(parent->_keys.begin() + left_idx);
    parent->_pointers.erase(parent->_pointers.begin() + left_idx + 1);
}

#endif
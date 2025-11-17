#pragma once

#include "radix_tree_it.hpp"
#include "radix_tree_node.hpp"

#include <cassert>
#include <string>
#include <utility>
#include <vector>

template<typename K, typename R = K>
R radix_substr(const K &key, size_t begin, size_t num);

//template<>
//inline std::string radix_substr<std::string>(const std::string &key, size_t begin, size_t num)
//{
//    return key.substr(begin, num);
//}

inline std::string_view radix_substr(const std::string& key, size_t begin, size_t num)
{
    std::string_view sv(key);
    return sv.substr(begin, num);
}

inline std::string_view radix_substr(const std::string_view& key, size_t begin, size_t num)
{
    return key.substr(begin, num);
}

template<typename K>
K radix_join(const K &key1, const K &key2);

template<>
inline std::string radix_join<std::string>(const std::string &key1, const std::string &key2)
{
    return key1 + key2;
}

template <typename K, typename T, typename Compare>
class radix_tree {
public:
    using key_type = K;
    using view_type = typename as_view<K>::type;
    using mapped_type = T;
    using value_type = std::pair<const K, T>;
    using iterator   = radix_tree_it<K, T, Compare>;
    using size_type  = std::size_t;

    radix_tree() noexcept = default;
    explicit radix_tree(Compare&& pred) noexcept
        : m_predicate(std::forward<Compare>(pred)), m_root(m_predicate)
    { }
    radix_tree(const radix_tree& other) = delete;
    radix_tree& operator=(const radix_tree& other) = delete;

    size_type size() const noexcept {
        return m_size;
    }
    bool empty() const noexcept {
        return m_size == 0;
    }
    void clear() noexcept {
        m_root = {};
        m_size = 0;
    }

    iterator find(const view_type &key) noexcept;
    iterator begin() noexcept;
    iterator end() noexcept;

    std::pair<iterator, bool> insert(const value_type &val);
    bool erase(const view_type &key);
    iterator erase(iterator it);
    void prefix_match(const view_type &key, std::vector<iterator> &vec);
    std::pair<iterator, iterator> prefix_range(const view_type& key) noexcept;
    void greedy_match(const view_type &key,  std::vector<iterator> &vec);
    value_type longest_match(const view_type &key);

    T& operator[] (const view_type &lhs);

private:
    using node_type = radix_tree_node<K, T, Compare>;
    using node_ptr = node_type*;

    size_type m_size = 0;
    Compare m_predicate {};
    node_type m_root {m_predicate};

    node_ptr begin(node_type &node) noexcept;
    node_ptr find_node(const view_type &key, node_type &node, uint32_t depth) noexcept;
    node_ptr append(node_ptr parent, const value_type &val);
    node_ptr prepend(node_ptr node, const value_type &val);
    void greedy_match(node_type &node, std::vector<iterator> &vec);
};

template <typename K, typename T, typename Compare>
void radix_tree<K, T, Compare>::prefix_match(const view_type &key, std::vector<iterator> &vec)
{
    vec.clear();

    auto node = find_node(key, m_root, 0);
    if (node->m_is_leaf)
        node = node->m_parent;

    auto len = radix_length(key) - node->m_depth;
    if (radix_substr(key, node->m_depth, len) != radix_substr(node->first, 0, len))
        return;

    greedy_match(*node, vec);
}

template <typename K, typename T, typename Compare>
auto
radix_tree<K, T, Compare>::prefix_range(const view_type& key) noexcept -> std::pair<iterator, iterator>
{
    // Find the node that corresponds to the search path for `key`
    auto* node = find_node(key, m_root, 0);

    // If find_node returned a leaf, the subtree of matches is the leaf's parent
    if (node->m_is_leaf)
        node = node->m_parent;

    // Verify the path actually matches the given prefix
    auto len = radix_length(key) - node->m_depth;
    if (radix_substr(key, node->m_depth, len) != radix_substr(node->first, 0, len))
        return { end(), end() }; // no matches

    // last: the rightmost leaf under `node` -> one-past-last is ++last_it
    auto* last_node = (not node->m_is_leaf && not node->m_children.empty())
                      ? node->m_children.rbegin()->second : node;

    return { iterator(begin(*node)), ++iterator(last_node) };
}

template <typename K, typename T, typename Compare>
auto radix_tree<K, T, Compare>::longest_match(const view_type& key) -> value_type
{
    auto node = find_node(key, m_root, 0);
    if (node->m_is_leaf)
        return *node;

    if (radix_substr(key, node->m_depth, radix_length(node->first)) != node->first)
        node = node->m_parent;

    for (; node != &m_root; node = node->m_parent) {
        auto it = node->m_children.find(K{});
        if ((it != node->m_children.end()) && it->second->m_is_leaf) {
            return iterator(it->second).GetValue();
        }
    }

    return { K{}, T{} };
}

template <typename K, typename T, typename Compare>
auto radix_tree<K, T, Compare>::end() noexcept -> iterator
{
    return {};
}

template <typename K, typename T, typename Compare>
auto radix_tree<K, T, Compare>::begin() noexcept -> iterator
{
    return iterator(m_size ? begin(m_root) : nullptr);
}

template <typename K, typename T, typename Compare>
auto
radix_tree<K, T, Compare>::begin(node_type &node) noexcept -> node_ptr
{
    if (node.m_is_leaf)
        return &node;
    assert(!node.m_children.empty());
    return begin(*node.m_children.begin()->second);
}

template <typename K, typename T, typename Compare>
T& radix_tree<K, T, Compare>::operator[] (const view_type &lhs)
{
    iterator it = find(lhs);

    if (it == end()) {
        auto [iti, isInserted] = insert({ K(lhs), {} });
        assert(isInserted);
        it = iti;
    }

    return it->second;
}

template <typename K, typename T, typename Compare>
void radix_tree<K, T, Compare>::greedy_match(const view_type &key, std::vector<iterator> &vec)
{
    vec.clear();

    auto *node = find_node(key, m_root, 0);
    if (node->m_is_leaf)
        node = node->m_parent;

    greedy_match(*node, vec);
}

template <typename K, typename T, typename Compare>
void radix_tree<K, T, Compare>::greedy_match(node_type &node, std::vector<iterator> &vec)
{
    if (node.m_is_leaf) {
        vec.push_back(iterator(&node));
        return;
    }

    std::ranges::for_each(node.m_children, [&](auto const& vt) {
            greedy_match(*vt.second, vec);
        });
}

template <typename K, typename T, typename Compare>
auto radix_tree<K, T, Compare>::erase(iterator it) -> iterator
{
    auto itR = it;
    ++itR;
    erase(it->first);
    return itR;
}

template <typename K, typename T, typename Compare>
bool radix_tree<K, T, Compare>::erase(const view_type &key)
{
    auto child = find_node(key, m_root, 0);
    if (not child->m_is_leaf)
        return false;

    auto parent = child->m_parent;
    parent->erase_child(K{});

    delete child;

    --m_size;

    if (parent == &m_root)
        return true;

    if (parent->m_children.size() > 1)
        return true;

    node_ptr grandparent;
    if (parent->m_children.empty()) {
        grandparent = parent->m_parent;
        grandparent->erase_child(parent->first);
        delete parent;
    } else {
        grandparent = parent;
    }

    if (grandparent == &m_root) {
        return true;
    }

    if (grandparent->m_children.size() == 1) {
        // merge grandparent with the uncle
        auto it = grandparent->m_children.begin();
        auto *uncle = it->second;
        if (uncle->m_is_leaf)
            return true;

        uncle->first = radix_join(grandparent->first, uncle->first);
        uncle->m_depth = grandparent->m_depth;
        uncle->m_parent = grandparent->m_parent;

        grandparent->erase_child(it);

        grandparent->m_parent->erase_child(grandparent->first);
        grandparent->m_parent->add_child(uncle);

        delete grandparent;
    }

    return true;
}


template <typename K, typename T, typename Compare>
auto
radix_tree<K, T, Compare>::append(node_ptr parent, const value_type &val) -> node_ptr
{
    auto depth = parent->m_depth + radix_length(parent->first);
    auto len   = radix_length(val.first) - depth;

    if (len == 0) {
        auto node_c = new node_type(parent, K{}, val.second, m_predicate);
        node_c->m_depth   = static_cast<uint32_t>(depth);
        node_c->m_is_leaf = true;

        parent->add_child(node_c);
        return node_c;
    }

    auto node_c = new node_type(parent, K(radix_substr(val.first, depth, len)), val.second, m_predicate);
    node_c->m_depth  = static_cast<uint32_t>(depth);
    parent->add_child(node_c);

    auto node_cc = new node_type(node_c, K{}, m_predicate);
    node_cc->m_depth   = static_cast<uint32_t>(depth + len);
    node_cc->m_is_leaf = true;
    node_c->add_child(node_cc);

    return node_cc;
}

template <typename K, typename T, typename Compare>
auto
radix_tree<K, T, Compare>::prepend(node_ptr node, const value_type &val) -> node_ptr
{
    size_t const len1 = radix_length(node->first);
    size_t const len2 = radix_length(val.first) - node->m_depth;
    size_t const l = std::min(len1, len2);
    size_t count;
    for (count = 0; count < l; count++) {
        if (node->first[count] != val.first[count + node->m_depth])
            break;
    }

    assert(count != 0);

    node->m_parent->erase_child(node->first);

    auto* node_a = new node_type(node->m_parent, K(radix_substr(node->first, 0, count)), m_predicate);
    node_a->m_depth  = node->m_depth;
    node_a->m_parent->add_child(node_a);

    node->m_depth  += static_cast<uint32_t>(count);
    node->m_parent  = node_a;
    node->first     = radix_substr(node->first, count, len1 - count);
    node->m_parent->add_child(node);

    if (count == len2) {
        auto node_b = new node_type(node_a, K{}, val.second, m_predicate);
        node_b->m_depth   = static_cast<uint32_t>(node_a->m_depth + count);
        node_b->m_is_leaf = true;
        node_b->m_parent->add_child(node_b);
        return node_b;
    }

    auto node_b = new node_type(node_a, K(radix_substr(val.first, node->m_depth, len2 - count)), m_predicate);
    node_b->m_depth  = node->m_depth;
    node_b->m_parent->add_child(node_b);

    auto node_c = new node_type(node_b, K{}, val.second, m_predicate);
    node_c->m_depth   = static_cast<uint32_t>(radix_length(val.first));
    node_c->m_is_leaf = true;
    node_c->m_parent->add_child(node_c);
    return node_c;
}

template <typename K, typename T, typename Compare>
std::pair<typename radix_tree<K, T, Compare>::iterator, bool>
radix_tree<K, T, Compare>::insert(const value_type& val)
{
    auto* node = find_node(val.first, m_root, 0);

    if (node->m_is_leaf) {
        return { iterator(node), false };
    }

    ++m_size;
    if (node == &m_root) {
        return { iterator(append(node, val)), true };
    }

    auto xpend = (radix_substr(val.first, node->m_depth, radix_length(node->first)) == node->first)
                    ? &radix_tree<K, T, Compare>::append : &radix_tree<K, T, Compare>::prepend;
    return { iterator((this->*xpend)(node, val)), true };
}

template <typename K, typename T, typename Compare>
auto
radix_tree<K, T, Compare>::find(const view_type& key) noexcept -> iterator
{
    radix_tree_node<K, T, Compare> *node = find_node(key, m_root, 0);
    // if the node is a internal node, return end()
    return node->m_is_leaf ? iterator(node) : end();
}

template <typename K, typename T, typename Compare>
auto
radix_tree<K, T, Compare>::find_node(const view_type& key, node_type& node, uint32_t depth) noexcept -> node_ptr
{
    if (node.m_children.empty())
        return &node;

    auto len_key = radix_length(key) - depth;
    for (auto const& child : node.m_children)
    {
        if (len_key == 0) {
            if (child.second->m_is_leaf)
                return child.second;
        }
        else if (not child.second->m_is_leaf && (key[depth] == child.first[0])) {
            auto len_node = radix_length(child.first);
            return (radix_substr(key, depth, len_node) == child.first)
                ? find_node(key, *child.second, static_cast<uint32_t>(depth + len_node))
                : child.second;
        }
    }

    return &node;
}


template<typename K, typename T, typename Compare, typename _UnaryPred>
void erase_if(radix_tree<K, T, Compare>& trie, _UnaryPred pred)
{
  for (auto it = trie.begin(); it != trie.end(); )
  {
    if (pred(it->first))
      it = trie.erase(it);
    else
      ++it;
  }
}


/*

(root)
|
|---------------
|       |      |
abcde   bcdef  c
|   |   |      |------
|   |   $3     |  |  |
f   ge         d  e  $6
|   |          |  |
$1  $2         $4 $5

find_node():
  bcdef  -> $3
  bcdefa -> bcdef
  c      -> $6
  cf     -> c
  abch   -> abcde
  abc    -> abcde
  abcde  -> abcde
  abcdef -> $1
  abcdeh -> abcde
  de     -> (root)


(root)
|
abcd
|
$

(root)
|
$

*/

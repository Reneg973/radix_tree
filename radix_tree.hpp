#ifndef RADIX_TREE_HPP
#define RADIX_TREE_HPP

#include "radix_tree_it.hpp"
#include "radix_tree_node.hpp"

#include <cassert>
#include <string>
#include <utility>
#include <vector>

template<typename K>
K radix_substr(const K &key, int begin, int num);

template<>
inline std::string radix_substr<std::string>(const std::string &key, int begin, int num)
{
    return key.substr(begin, num);
}

inline std::string_view radix_substr_v(const std::string_view& key, int begin, int num)
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

template<typename K>
int radix_length(const K &key);

template<>
inline int radix_length<std::string>(const std::string &key)
{
    return static_cast<int>(key.size());
}

template<>
inline int radix_length<std::string_view>(const std::string_view& key)
{
    return static_cast<int>(key.size());
}

template <typename K, typename T, typename Compare>
class radix_tree {
public:
    typedef K key_type;
    typedef T mapped_type;
    typedef std::pair<const K, T> value_type;
    typedef radix_tree_it<K, T, Compare>   iterator;
    typedef std::size_t           size_type;

    radix_tree()
        : m_predicate(Compare()), m_root(new radix_tree_node<K, T, Compare>(m_predicate))
    { }
    explicit radix_tree(Compare&& pred)
        : m_predicate(std::forward<Compare>(pred)), m_root(new radix_tree_node<K, T, Compare>(m_predicate))
    { }

    size_type size() const {
        return m_size;
    }
    bool empty() const {
        return m_size == 0;
    }
    void clear() {
        delete m_root;
        m_root = nullptr;
        m_size = 0;
    }

    iterator find(const K &key);
    iterator begin();
    iterator end();

    std::pair<iterator, bool> insert(const value_type &val);
    bool erase(const K &key);
    radix_tree_it<K, T, Compare> erase(iterator it);
    void prefix_match(const K &key, std::vector<iterator> &vec);
    void greedy_match(const K &key,  std::vector<iterator> &vec);
    iterator longest_match(const K &key);

    T& operator[] (const K &lhs);

    template<class _UnaryPred>
    void remove_if(_UnaryPred pred)
    {
        for (auto it = begin(); it != end(); )
        {
            if (pred(it->first))
                it = erase(it);
            else
                ++it;
        }
    }

private:
    size_type m_size = 0;
    Compare m_predicate;
    radix_tree_node<K, T, Compare> *m_root;

    radix_tree_node<K, T, Compare>* begin(radix_tree_node<K, T, Compare> &node);
    radix_tree_node<K, T, Compare>* find_node(const K &key, radix_tree_node<K, T, Compare> &node, int depth);
    radix_tree_node<K, T, Compare>* append(radix_tree_node<K, T, Compare> *parent, const value_type &val);
    radix_tree_node<K, T, Compare>* prepend(radix_tree_node<K, T, Compare> *node, const value_type &val);
    void greedy_match(radix_tree_node<K, T, Compare> *node, std::vector<iterator> &vec);

    radix_tree(const radix_tree& other) = delete;
    radix_tree& operator =(const radix_tree other) = delete;
};

template <typename K, typename T, typename Compare>
void radix_tree<K, T, Compare>::prefix_match(const K &key, std::vector<iterator> &vec)
{
    vec.clear();

    auto node = find_node(key, *m_root, 0);
    if (node->m_is_leaf)
        node = node->m_parent;

    int len = radix_length(key) - node->m_depth;
    if (radix_substr_v(key, node->m_depth, len) != radix_substr_v(node->m_key, 0, len))
        return;

    greedy_match(node, vec);
}

template <typename K, typename T, typename Compare>
typename radix_tree<K, T, Compare>::iterator radix_tree<K, T, Compare>::longest_match(const K &key)
{
    auto node = find_node(key, *m_root, 0);
    if (node->m_is_leaf)
        return iterator(node);

    if (radix_substr_v(key, node->m_depth, radix_length(node->m_key)) != node->m_key)
        node = node->m_parent;

    for (; node; node = node->m_parent) {
        auto it = node->m_children.find(K{});
        if (it != node->m_children.end() && it->second->m_is_leaf)
            return iterator(it->second);
    }

    return end();
}

template <typename K, typename T, typename Compare>
radix_tree<K, T, Compare>::iterator radix_tree<K, T, Compare>::end()
{
    return {};
}

template <typename K, typename T, typename Compare>
radix_tree<K, T, Compare>::iterator radix_tree<K, T, Compare>::begin()
{
    return iterator(m_size ? begin(*m_root) : nullptr);
}

template <typename K, typename T, typename Compare>
radix_tree_node<K, T, Compare>*
radix_tree<K, T, Compare>::begin(radix_tree_node<K, T, Compare> &node)
{
    if (node.m_is_leaf)
        return &node;
    assert(!node.m_children.empty());
    return begin(*node.m_children.begin()->second);
}

template <typename K, typename T, typename Compare>
T& radix_tree<K, T, Compare>::operator[] (const K &lhs)
{
    iterator it = find(lhs);

    if (it == end()) {
        auto [iti, isInserted] = insert({ lhs, {} });
        assert(isInserted);
        it = iti;
    }

    return it->second;
}

template <typename K, typename T, typename Compare>
void radix_tree<K, T, Compare>::greedy_match(const K &key, std::vector<iterator> &vec)
{
    vec.clear();

    auto node = find_node(key, *m_root, 0);

    if (node->m_is_leaf)
        node = node->m_parent;

    greedy_match(node, vec);
}

template <typename K, typename T, typename Compare>
void radix_tree<K, T, Compare>::greedy_match(radix_tree_node<K, T, Compare> *node, std::vector<iterator> &vec)
{
    if (node->m_is_leaf) {
        vec.push_back(iterator(node));
        return;
    }

    for (auto &child :node->m_children) {
        greedy_match(child.second, vec);
    }
}

template <typename K, typename T, typename Compare>
radix_tree_it<K, T, Compare> radix_tree<K, T, Compare>::erase(iterator it)
{
    auto itR = it;
    ++itR;
    erase(it->first);
    return itR;
}

template <typename K, typename T, typename Compare>
bool radix_tree<K, T, Compare>::erase(const K &key)
{
    auto child = find_node(key, *m_root, 0);
    if (not child->m_is_leaf)
        return false;

    auto parent = child->m_parent;
    parent->m_children.erase(K{});

    delete child;

    --m_size;

    if (parent == m_root)
        return true;

    if (parent->m_children.size() > 1)
        return true;

    radix_tree_node<K, T, Compare> *grandparent;
    if (parent->m_children.empty()) {
        grandparent = parent->m_parent;
        grandparent->m_children.erase(parent->m_key);
        delete parent;
    } else {
        grandparent = parent;
    }

    if (grandparent == m_root) {
        return true;
    }

    if (grandparent->m_children.size() == 1) {
        // merge grandparent with the uncle
        auto it = grandparent->m_children.begin();
        auto *uncle = it->second;
        if (uncle->m_is_leaf)
            return true;

        uncle->m_depth = grandparent->m_depth;
        uncle->m_key   = radix_join(grandparent->m_key, uncle->m_key);
        uncle->m_parent = grandparent->m_parent;

        grandparent->m_children.erase(it);

        grandparent->m_parent->m_children.erase(grandparent->m_key);
        grandparent->m_parent->m_children[uncle->m_key] = uncle;

        delete grandparent;
    }

    return true;
}


template <typename K, typename T, typename Compare>
radix_tree_node<K, T, Compare>*
radix_tree<K, T, Compare>::append(radix_tree_node<K, T, Compare> *parent, const value_type &val)
{
    auto depth = parent->m_depth + radix_length(parent->m_key);
    auto len   = radix_length(val.first) - depth;

    if (len == 0) {
        auto node_c = new radix_tree_node<K, T, Compare>(parent, K{}, val, m_predicate);
        node_c->m_depth   = depth;
        node_c->m_is_leaf = true;

        parent->m_children[node_c->m_key] = node_c;
        return node_c;
    }

    auto node_c = new radix_tree_node<K, T, Compare>(parent, radix_substr(val.first, depth, len), val, m_predicate);
    node_c->m_depth  = depth;
    parent->m_children[node_c->m_key] = node_c;

    auto node_cc = new radix_tree_node<K, T, Compare>(val, m_predicate);
    node_cc->m_depth   = depth + len;
    node_cc->m_parent  = node_c;
    node_cc->m_is_leaf = true;
    node_c->m_children[node_cc->m_key] = node_cc;

    return node_cc;
}

template <typename K, typename T, typename Compare>
radix_tree_node<K, T, Compare>* radix_tree<K, T, Compare>::prepend(radix_tree_node<K, T, Compare> *node, const value_type &val)
{
    int len1 = radix_length(node->m_key);
    int len2 = radix_length(val.first) - node->m_depth;

    int count;
    for (count = 0; count < len1 && count < len2; count++) {
        if (not (node->m_key[count] == val.first[count + node->m_depth]) )
            break;
    }

    assert(count != 0);

    node->m_parent->m_children.erase(node->m_key);

    radix_tree_node<K, T, Compare> *node_a = new radix_tree_node<K, T, Compare>(m_predicate);

    node_a->m_parent = node->m_parent;
    node_a->m_key    = radix_substr(node->m_key, 0, count);
    node_a->m_depth  = node->m_depth;
    node_a->m_parent->m_children[node_a->m_key] = node_a;


    node->m_depth  += count;
    node->m_parent  = node_a;
    node->m_key     = radix_substr(node->m_key, count, len1 - count);
    node->m_parent->m_children[node->m_key] = node;

    if (count == len2) {
        auto node_b = new radix_tree_node<K, T, Compare>(val, m_predicate);

        node_b->m_parent  = node_a;
        node_b->m_depth   = node_a->m_depth + count;
        node_b->m_is_leaf = true;
        node_b->m_parent->m_children[node_b->m_key] = node_b;
        return node_b;
    } else {
        auto node_b = new radix_tree_node<K, T, Compare>(node_a, radix_substr(val.first, node->m_depth, len2 - count), m_predicate);
        node_b->m_depth  = node->m_depth;
        node_b->m_parent->m_children[node_b->m_key] = node_b;

        auto node_c = new radix_tree_node<K, T, Compare>(node_b, K{}, val, m_predicate);
        node_c->m_depth   = radix_length(val.first);
        node_c->m_is_leaf = true;
        node_c->m_parent->m_children[node_c->m_key] = node_c;

        return node_c;
    }
}

template <typename K, typename T, typename Compare>
std::pair<typename radix_tree<K, T, Compare>::iterator, bool>
radix_tree<K, T, Compare>::insert(const value_type &val)
{
    auto *node = find_node(val.first, *m_root, 0);

    if (node->m_is_leaf) {
        return { node, false };
    }

    ++m_size;
    if (node == m_root) {
        return { append(m_root, val), true };
    }

    if (radix_substr_v(val.first, node->m_depth, radix_length(node->m_key)) == node->m_key)
        return { append(node, val), true };
    return { prepend(node, val), true };
}

template <typename K, typename T, typename Compare>
typename radix_tree<K, T, Compare>::iterator
radix_tree<K, T, Compare>::find(const K &key)
{
    radix_tree_node<K, T, Compare> *node = find_node(key, *m_root, 0);

    // if the node is a internal node, return NULL
    if (not node->m_is_leaf)
        return end();

    return iterator(node);
}

template <typename K, typename T, typename Compare>
radix_tree_node<K, T, Compare>* radix_tree<K, T, Compare>::find_node(const K &key, radix_tree_node<K, T, Compare> &node, int depth)
{
    if (node.m_children.empty())
        return &node;

    typename radix_tree_node<K, T, Compare>::it_child it;
    int len_key = radix_length(key) - depth;

    for (auto const& child : node.m_children) {
        if (len_key == 0) {
            if (child.second->m_is_leaf)
                return child.second;
            continue;
        }

        if (!child.second->m_is_leaf && key[depth] == child.first[0] ) {
            int len_node = radix_length(child.first);
            return (radix_substr_v(key, depth, len_node) == child.first)
                ? find_node(key, *child.second, depth+len_node)
                : child.second;
        }
    }

    return &node;
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

#endif // RADIX_TREE_HPP

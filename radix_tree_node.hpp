#ifndef RADIX_TREE_NODE_HPP
#define RADIX_TREE_NODE_HPP

#include "radix_tree_it.hpp"

#include <map>
#include <optional>
#include <string_view>

template <typename T>
struct as_view { using type = T; };
template <>
struct as_view<std::string> { using type = std::string_view; };

template <typename K, typename T, typename Compare>
class radix_tree_node {
    friend class radix_tree<K, T, Compare>;
    friend class radix_tree_it<K, T, Compare>;

    using ThisType = radix_tree_node<K, T, Compare>;
    using value_type = std::pair<const K, T>;
    // the KV view type always has its corresponding lifetime managed by the original K stored in the node
    using KV = typename as_view<K>::type;
    using MapType = std::map<KV, ThisType*, Compare>;
    using it_child = typename MapType::iterator;

private:
    radix_tree_node(Compare& pred)
        : radix_tree_node(nullptr, K(), pred)
    { }
    radix_tree_node(const value_type& val, Compare& pred)
        : radix_tree_node(nullptr, K(), val, pred)
    { }
    radix_tree_node(radix_tree_node* parent, const K& key, Compare& pred)
        : m_children(pred)
        , m_parent(parent)
        , m_key(key)
        , m_pred(pred)
    { }
    radix_tree_node(radix_tree_node* parent, const K& key, const value_type& val, Compare& pred)
        : m_children(pred)
        , m_parent(parent)
        , m_value(val)
        , m_key(key)
        , m_pred(pred)
    { }

    radix_tree_node(const radix_tree_node&) = delete;
    radix_tree_node& operator=(const radix_tree_node&) = delete;

    ~radix_tree_node()
    {
        for (auto& child : m_children) {
            delete child.second;
        }
    }

    MapType m_children;
    ThisType *m_parent;
    std::optional<value_type> m_value; // not every node has a value
    int m_depth = 0;
    K m_key;
    Compare& m_pred;
    bool m_is_leaf = false;
};

#endif // RADIX_TREE_NODE_HPP

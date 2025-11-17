#pragma once

#include "radix_tree_it.hpp"

#include <algorithm>
#include <map>

template <typename K, typename T, typename Compare>
class radix_tree_node : public std::pair<K, T> {
    friend class radix_tree<K, T, Compare>;
    friend class radix_tree_it<K, T, Compare>;

    using value_type = T;
    using BaseType = std::pair<K, T>;
    using ThisType = radix_tree_node<K, T, Compare>;
    // the map uses a view type aas its corresponding lifetime is managed by the K stored in the node
    using MapType = std::map<typename as_view<K>::type, ThisType*, Compare>;

private:
    radix_tree_node(Compare& pred)
        : radix_tree_node(nullptr, {}, pred)
    { }
    radix_tree_node(const T& val, Compare& pred)
        : radix_tree_node(nullptr, {}, val, pred)
    { }
    radix_tree_node(radix_tree_node* parent, const K& key, Compare& pred)
        : BaseType(key, {})
        , m_children(pred)
        , m_parent(parent)
    { }
    radix_tree_node(radix_tree_node* parent, const K& key, const T& val, Compare& pred)
        : BaseType(key, val)
        , m_children(pred)
        , m_parent(parent)
    { }

    radix_tree_node(const radix_tree_node&) = delete;
    radix_tree_node& operator=(const radix_tree_node&) = delete;

    ~radix_tree_node()
    {
        std::ranges::for_each(m_children, [](auto& vt)
            {
                delete vt.second;
            });
    }

    void add_child(radix_tree_node* node)
    {
        m_children[node->first] = node;
    }

    template<typename U>
    void erase_child(U child)
    {
        m_children.erase(child);
    }

    MapType m_children;
    ThisType *m_parent;
    uint32_t m_depth = 0;
    bool m_is_leaf = false;
};

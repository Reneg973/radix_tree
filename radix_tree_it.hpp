#pragma once

#include <cassert>
#include <string>
#include <string_view>

template <typename T>
struct as_view { using type = T; };
template <>
struct as_view<std::string> { using type = std::string_view; };

template<typename K>
void radix_copyto(const typename as_view<K>::type& from, K& to, size_t& off);

template<>
inline void radix_copyto<std::string>(const std::string_view& from, std::string& to, size_t& off)
{
    off -= from.size();
    from.copy(to.data() + off, from.size());
}

template<typename K>
size_t radix_length(const K& key);

template<>
inline size_t radix_length<std::string>(const std::string& key)
{
    return key.size();
}

template<>
inline size_t radix_length<std::string_view>(const std::string_view& key)
{
    return key.size();
}


struct StringCompare
{
    using is_transparent = void;
    template <typename T, typename U>
    bool operator()(const T& lhs, const U& rhs) const noexcept
    {
        return lhs < rhs;
    }
};

template <typename T>
struct Comparer { using type = std::less<T>; };
template <>
struct Comparer<std::string> { using type = StringCompare; };

// forward declaration
template <typename K, typename T, class Compare = Comparer<K>::type> class radix_tree;
template <typename K, typename T, class Compare = Comparer<K>::type> class radix_tree_node;

template <typename K, typename T, class Compare = Comparer<K>>
class radix_tree_it {
    friend class radix_tree<K, T, Compare>;

public:
    // iterator aliases required by std::iterator_traits
    using iterator_category = std::forward_iterator_tag;
    using value_type        = std::pair<K, T>;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;
    using node_ptr          = radix_tree_node<K, T, Compare>*;

    radix_tree_it() noexcept = default;

    value_type& operator*  () const;
    value_type* operator-> () const;
    const radix_tree_it& operator++ ();
    radix_tree_it operator++ (int);
    bool operator== (const radix_tree_it &lhs) const noexcept = default;

    std::pair<K, const T&> GetValue() const;

private:
    node_ptr m_pointee = nullptr;
    explicit radix_tree_it(node_ptr p) noexcept : m_pointee(p) { }

    node_ptr increment(node_ptr node) const;
    node_ptr descend(node_ptr node) const;
};

template <typename K, typename T, typename Compare>
auto radix_tree_it<K, T, Compare>::increment(node_ptr node) const -> node_ptr
{
    auto* parent = node->m_parent;
    if (parent == nullptr)
        return nullptr;

    auto it = parent->m_children.find(node->first);
    assert(it != parent->m_children.end());
    return (++it == parent->m_children.end()) ? increment(parent) : descend(it->second);
}

template <typename K, typename T, typename Compare>
auto radix_tree_it<K, T, Compare>::descend(node_ptr node) const -> node_ptr
{
    while (not node->m_is_leaf)
    {
        auto it = node->m_children.begin();
        assert(it != node->m_children.end());
        node = it->second;
    }
    return node;
}

template <typename K, typename T, typename Compare>
auto radix_tree_it<K, T, Compare>::GetValue() const -> std::pair<K, const T&>
{
    size_t s = 0;
    for (auto node = m_pointee->m_parent; node && node->m_parent; node = node->m_parent) {
        s += radix_length(node->first);
    }

    K k(s, 0);
    for (auto node = m_pointee->m_parent; node && node->m_parent; node = node->m_parent) {
        radix_copyto(node->first, k, s);
    }

    return { std::move(k), m_pointee->second };
}

template <typename K, typename T, typename Compare>
auto radix_tree_it<K, T, Compare>::operator* () const -> reference
{
    return *m_pointee;
}

template <typename K, typename T, typename Compare>
auto radix_tree_it<K, T, Compare>::operator-> () const -> pointer
{
    return m_pointee;
}

template <typename K, typename T, typename Compare>
auto radix_tree_it<K, T, Compare>::operator++ () -> const radix_tree_it&
{
    if (m_pointee)
        m_pointee = increment(m_pointee);
    return *this;
}

template <typename K, typename T, typename Compare>
auto radix_tree_it<K, T, Compare>::operator++ (int) -> radix_tree_it
{
    radix_tree_it copy(*this);
    ++(*this);
    return copy;
}

#ifndef RADIX_TREE_IT
#define RADIX_TREE_IT

#include <cassert>
#include <iterator>

struct StringComparer
{
    using is_transparent = void;
    bool operator()(const std::string_view& lhs, const std::string_view& rhs) const noexcept
    {
        return lhs < rhs;
    }
    bool operator()(const std::string& lhs, const std::string_view& rhs) const noexcept
    {
        return lhs < rhs;
    }
    bool operator()(const std::string_view& lhs, const std::string& rhs) const noexcept
    {
        return lhs < rhs;
    }
    bool operator()(const std::string& lhs, const std::string& rhs) const noexcept
    {
        return lhs < rhs;
    }
};

template <typename T>
struct Comparer { using type = std::less<T>; };
template <>
struct Comparer<std::string> { using type = StringComparer; };

// forward declaration
template <typename K, typename T, class Compare = typename Comparer<K>::type > class radix_tree;
template <typename K, typename T, class Compare = typename Comparer<K>::type > class radix_tree_node;

template <typename K, typename T, class Compare = Comparer<K> >
class radix_tree_it {
    friend class radix_tree<K, T, Compare>;

public:
    // iterator aliases required by std::iterator_traits
    using iterator_category = std::forward_iterator_tag;
    using value_type        = std::pair<const K, T>;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;

    radix_tree_it() : m_pointee(nullptr) { }
    radix_tree_it(const radix_tree_it& r) : m_pointee(r.m_pointee) { }
    radix_tree_it& operator=(const radix_tree_it& r) { m_pointee = r.m_pointee; return *this; }
    ~radix_tree_it() { }

    std::pair<const K, T>& operator*  () const;
    std::pair<const K, T>* operator-> () const;
    const radix_tree_it<K, T, Compare>& operator++ ();
    radix_tree_it<K, T, Compare> operator++ (int);
    bool operator== (const radix_tree_it<K, T, Compare> &lhs) const;

private:
    radix_tree_node<K, T, Compare> *m_pointee;
    radix_tree_it(radix_tree_node<K, T, Compare> *p) : m_pointee(p) { }

    radix_tree_node<K, T, Compare>* increment(radix_tree_node<K, T, Compare>* node) const;
    radix_tree_node<K, T, Compare>* descend(radix_tree_node<K, T, Compare>* node) const;
};

template <typename K, typename T, typename Compare>
radix_tree_node<K, T, Compare>* radix_tree_it<K, T, Compare>::increment(radix_tree_node<K, T, Compare>* node) const
{
    auto* parent = node->m_parent;
    if (parent == nullptr)
        return nullptr;

    auto it = parent->m_children.find(node->m_key);
    assert(it != parent->m_children.end());
    return (++it == parent->m_children.end()) ? increment(parent) : descend(it->second);
}

template <typename K, typename T, typename Compare>
radix_tree_node<K, T, Compare>* radix_tree_it<K, T, Compare>::descend(radix_tree_node<K, T, Compare>* node) const
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
std::pair<const K, T>& radix_tree_it<K, T, Compare>::operator* () const
{
    return m_pointee->m_value.get();
}

template <typename K, typename T, typename Compare>
std::pair<const K, T>* radix_tree_it<K, T, Compare>::operator-> () const
{
    return m_pointee->m_value.has_value() ? &m_pointee->m_value.value() : nullptr;
}

template <typename K, typename T, typename Compare>
bool radix_tree_it<K, T, Compare>::operator== (const radix_tree_it<K, T, Compare> &lhs) const
{
    return m_pointee == lhs.m_pointee;
}

template <typename K, typename T, typename Compare>
const radix_tree_it<K, T, Compare>& radix_tree_it<K, T, Compare>::operator++ ()
{
    if (m_pointee) // it is undefined behaviour to dereference iterator that is out of bounds...
        m_pointee = increment(m_pointee);
    return *this;
}

template <typename K, typename T, typename Compare>
radix_tree_it<K, T, Compare> radix_tree_it<K, T, Compare>::operator++ (int)
{
    radix_tree_it<K, T, Compare> copy(*this);
    ++(*this);
    return copy;
}

#endif // RADIX_TREE_IT

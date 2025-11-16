#ifndef RADIX_TREE_IT
#define RADIX_TREE_IT

#include <cassert>
#include <iterator>

// forward declaration
template <typename K, typename T, class Compare = std::less<K> > class radix_tree;
template <typename K, typename T, class Compare = std::less<K> > class radix_tree_node;

template <typename K, typename T, class Compare = std::less<K> >
class radix_tree_it {
    friend class radix_tree<K, T, Compare>;

public:
    // iterator aliases required by std::iterator_traits
    using iterator_category = std::forward_iterator_tag;
    using value_type        = std::pair<const K, T>;
    using difference_type   = std::ptrdiff_t;
    using pointer           = value_type*;
    using reference         = value_type&;

    radix_tree_it() : m_pointee(0) { }
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
    radix_tree_node<K, T, Compare>* parent = node->m_parent;

    if (parent == NULL)
        return NULL;

    typename radix_tree_node<K, T, Compare>::it_child it = parent->m_children.find(node->m_key);
    assert(it != parent->m_children.end());
    ++it;

    return (it == parent->m_children.end()) ? increment(parent) : descend(it->second);
}

template <typename K, typename T, typename Compare>
radix_tree_node<K, T, Compare>* radix_tree_it<K, T, Compare>::descend(radix_tree_node<K, T, Compare>* node) const
{
    if (node->m_is_leaf)
        return node;

    auto it = node->m_children.begin();
    assert(it != node->m_children.end());
    return descend(it->second);
}

template <typename K, typename T, typename Compare>
std::pair<const K, T>& radix_tree_it<K, T, Compare>::operator* () const
{
    return *m_pointee->m_value;
}

template <typename K, typename T, typename Compare>
std::pair<const K, T>* radix_tree_it<K, T, Compare>::operator-> () const
{
    return m_pointee->m_value;
}

template <typename K, typename T, typename Compare>
bool radix_tree_it<K, T, Compare>::operator== (const radix_tree_it<K, T, Compare> &lhs) const
{
    return m_pointee == lhs.m_pointee;
}

template <typename K, typename T, typename Compare>
const radix_tree_it<K, T, Compare>& radix_tree_it<K, T, Compare>::operator++ ()
{
    if (m_pointee != NULL) // it is undefined behaviour to dereference iterator that is out of bounds...
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

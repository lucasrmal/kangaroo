#ifndef TREAPMAP_H
#define TREAPMAP_H

#include "Treap.h"
#include <stdexcept>

/*
 * Code from TreeMapIterator::operator++ taken (and modified) from
 * https://secweb.cs.odu.edu/~zeil/cs361/web/website/Lectures/treetraversal/page/treetraversal.html
 *
 */

namespace KLib
{

// Declaration of types
template<class K, class V> class TreapMap;
template<class K, class V> class TreapMapPair;
template<typename T, typename keytype, typename valuetype> class TreeMapIterator;


template<class K, class V>
class TreapMap
{
public:

    typedef V*        pointer;
    typedef const V*  const_pointer;
    typedef V&        reference;
    typedef const V&  const_reference;
    typedef K&        key;
    typedef const K&  const_key;
    typedef ods::TreapNode1<TreapMapPair<K,V>> node;

    typedef TreeMapIterator<TreapMap<K,V>, const_key, reference> iterator;
    typedef TreeMapIterator<const TreapMap<K,V>, const_key, const_reference> const_iterator;

    TreapMap() {}

    virtual bool add(const K& _key, const_reference _value);
    virtual bool remove(const K& _key);
    virtual void set(const K& _key, const_reference _value);

    //virtual const_reference operator[](const K& _key) const { return m_treap.find(TreapMapPair<K,V>(_key)).value; }

    virtual void clear() { m_treap.clear(); }
    virtual int size() const { return m_treap.size(); }
    virtual int count() const { return m_treap.size(); }
    virtual int empty() const { return m_treap.size() == 0; }

    virtual const_reference findLT(const K& _key) const;
    virtual reference findLT(const K& _key);

    virtual const_reference first() const;
    virtual const_reference last() const;
    virtual const_key lastKey() const;
    virtual reference first();
    virtual reference last();

    iterator end() { return iterator(nullptr, this); }
    iterator begin() { return iterator(this->findFirstNode(), this); }
    const_iterator end() const { return const_iterator(nullptr,  m_treap.root()); }
    const_iterator begin() const { return const_iterator(m_treap.findFirstNode(), m_treap.root()); }

private:

    ods::Treap1<TreapMapPair<K,V> > m_treap;

    node* m_last;
};

/* ####################################################################### */
/* Iterator */

template<typename T, typename keytype, typename valuetype>
class TreeMapIterator
{

private:
    typename T::node* m_node;
    typename T::node* m_root;

public:
    TreeMapIterator(typename T::node * _node, typename T::node* _root)
    {
        m_node = _node;
        m_root = _root;
    }

    bool operator==(const TreeMapIterator<T, keytype, valuetype>& _other) const { return m_node == _other.m_node; }
    bool operator!=(const TreeMapIterator<T, keytype, valuetype>& _other) const { return m_node != _other.m_node; }

    keytype key() const { return m_node->x.key; }
    valuetype value() const { return m_node->x.value; }

    valuetype& operator*()  { return (m_node->x.value); }

    TreeMapIterator& operator++()
    {
        typename T::node* p;

        if (m_node == nullptr)
        {
            // ++ from end(). get the root of the tree
            m_node = m_root;

            // error! ++ requested for an empty tree
            if (m_node == nullptr)
                throw std::underflow_error("TreeMapIterator operator++ (): tree empty");

            // move to the smallest value in the tree,
            // which is the first node inorder
            while (m_node->left != nullptr)
            {
                m_node = m_node->left;
            }
        }
        else if (m_node->right != nullptr)
        {
            // successor is the furthest left node of
            // right subtree
            m_node = m_node->right;

            while (m_node->left != nullptr)
            {
                m_node = m_node->left;
            }
        }
        else
        {
            // have already processed the left subtree, and
            // there is no right subtree. move up the tree,
            // looking for a parent for which m_node is a left child,
            // stopping if the parent becomes nullptr. a non-nullptr parent
            // is the successor. if parent is nullptr, the original node
            // was the last node inorder, and its successor
            // is the end of the list
            p = m_node->parent;

            while (p != nullptr && m_node == p->right)
            {
                m_node = p;
                p = p->parent;
            }

            // if we were previously at the right-most node in
            // the tree, m_node = nullptr, and the iterator specifies
            // the end of the list
            m_node = p;
        }

        return *this;
    }
};

/* ####################################################################### */
/* TreapMapPair */

template<class K, class V>
class TreapMapPair
{
public:
    TreapMapPair() {}
    TreapMapPair(const K& _k) : key(_k) {}
    TreapMapPair(const K& _k, const V& _v) : key(_k), value(_v) {}

    TreapMapPair(const TreapMapPair& _other)
    {
        key = _other.key;
        value = _other.value;
    }

    TreapMapPair& operator=(const TreapMapPair& _other)
    {
        key = _other.key;
        value = _other.value;
        return *this;
    }

    bool operator<(const TreapMapPair<K,V>& _other) const { return key < _other.key; }
    bool operator==(const TreapMapPair<K,V>& _other) const { return key == _other.key; }

    K key;
    V value;
};

/* ####################################################################### */
/* Function Definitions */

template<class K, class V>
bool TreapMap<K,V>::add(const K& _key, const_reference _value)
{
    if (m_treap.add(TreapMapPair<K,V>(_key, _value)))
    {
        m_last = m_treap.findLastNode();
        return true;
    }

    return false;
}

template<class K, class V>
void TreapMap<K,V>::set(const K& _key, const_reference _value)
{
    auto f = m_treap.findEQNode(TreapMapPair<K,V>(_key, _value));
    if (f)
    {
        f->x = TreapMapPair<K,V>(_key, _value);
    }
    else
    {
        add(_key, _value);
    }
}

template<class K, class V>
bool TreapMap<K,V>::remove(const K& _key)
{
    if (m_treap.remove(TreapMapPair<K,V>(_key)))
    {
        m_last = m_treap.findLastNode();
        return true;
    }

    return false;
}

template<class K, class V>
typename TreapMap<K,V>::const_reference TreapMap<K,V>::findLT(const K& _key) const
{
    auto p = m_treap.findLTNode(TreapMapPair<K,V>(_key));

    if (!p)
    {
        throw std::underflow_error("No key before this key");
    }
    else
    {
        return p->x.value;
    }
}

template<class K, class V>
typename TreapMap<K,V>::reference TreapMap<K,V>::findLT(const K& _key)
{
    auto p = m_treap.findLTNode(TreapMapPair<K,V>(_key));

    if (!p)
    {
        throw std::underflow_error("No key before this key");
    }
    else
    {
        return p->x.value;
    }
}

template<class K, class V>
typename TreapMap<K,V>::reference TreapMap<K,V>::last()
{
    if (!empty())
    {
        return m_last->x.value;
    }
    else
    {
        throw std::underflow_error("TreapMap is empty!");
    }
}

template<class K, class V>
typename TreapMap<K,V>::const_reference TreapMap<K,V>::last() const
{
    if (!empty())
    {
        return m_last->x.value;
    }
    else
    {
        throw std::underflow_error("TreapMap is empty!");
    }
}

template<class K, class V>
typename TreapMap<K,V>::const_key TreapMap<K,V>::lastKey() const
{
    if (!empty())
    {
        return m_last->x.key;
    }
    else
    {
        throw std::underflow_error("TreapMap is empty!");
    }
}

template<class K, class V>
typename TreapMap<K,V>::reference TreapMap<K,V>::first()
{
    auto p = m_treap.findFirstNode();

    if (!p)
    {
        throw std::underflow_error("TreapMap is empty!");
    }
    else
    {
        return p->x.value;
    }
}

template<class K, class V>
typename TreapMap<K,V>::const_reference TreapMap<K,V>::first() const
{
    auto p = m_treap.findFirstNode();

    if (!p)
    {
        throw std::underflow_error("TreapMap is empty!");
    }
    else
    {
        return p->x.value;
    }
}






}

#endif // TREAPMAP_H

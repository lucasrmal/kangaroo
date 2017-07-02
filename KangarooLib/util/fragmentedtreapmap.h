/*
 * Fragmented Treap Map
 * Copyright (C) 2015 Lucas Rioux-Maldague
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the copyright holder nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef FRAGMENTEDTREAPMAP_H
#define FRAGMENTEDTREAPMAP_H

#include "augmentedtreapmap.h"
#include <list>

namespace KLib
{
    namespace FragmentedTreap
    {
        template<class R, class S>
        S transform(const R& _ratio, const S& _previous) {
            return _ratio * _previous;
        }

        template<class R>
        R makeEmpty()
        {
            return 1;
        }

    }

    template<class K, class R, class Node>
    class FragmentedTreapMapNode
    {
        public:
            FragmentedTreapMapNode() :
                ratio(FragmentedTreap::makeEmpty<R>()),
                root(nullptr) {}

            FragmentedTreapMapNode(const K& _start, const R& _ratio, Node* _root) :
                start(_start),
                ratio(_ratio),
                root(_root) {}

            K start;
            R ratio;        ///< The sum BEFORE this node is multiplied by this ratio.
            Node* root;
    };

    //Forward-declaration of iterator
    template<typename T, typename valuetype>
    class FragmentedTreapMapIterator;

    template<class K, class V, class R, class Node>
    class FragmentedTreapMap
    {
            const FragmentedTreapMap<K,V,R,Node>& constThis()
            {
                return static_cast<const FragmentedTreapMap<K,V,R,Node>&>(*this);
            }

        public:
            typedef FragmentedTreapMapNode<K, R, Node> fnode;
            typedef std::list<fnode*> fnode_list;
            typedef typename std::list<fnode*>::iterator fnode_iterator;
            typedef typename std::list<fnode*>::const_iterator const_fnode_iterator;

            typedef V&        reference;
            typedef const V&  const_reference;
            typedef const K&  const_key;
            typedef typename  Node::weight_type weight;
            typedef const weight& const_weight;
            typedef const R& const_ratio;
            typedef Node      node_type;
            typedef V         value_type;
            typedef K         key_type;
            typedef R         ratio_type;

            typedef FragmentedTreapMapIterator<FragmentedTreapMap<K,V, R, Node>, reference> iterator;
            typedef FragmentedTreapMapIterator<const FragmentedTreapMap<K,V, R, Node>, const_reference> const_iterator;


            FragmentedTreapMap()
            {
                m_nodes.push_front(new fnode());
            }

            void insert(const_key _key, const_reference _value, const_weight _weight);
            bool remove(const_key _key, const_reference _value);

            bool remove(iterator i);
            bool removeAll(const_key _key);
            bool erase(iterator i)  { return remove(i); }

            bool move(const_key _currentKey, const_reference _value, const_key _newKey);
            bool setWeight(const_key _key, const_reference _value, const_weight _weight);


            bool splitFragmentAt(const_key _key, const_ratio _val);
            bool setFragmentRatio(const_key _key, const_ratio _val);
            bool joinFragmentsAt(const_key _key);
            int fragmentCount() const { return m_nodes.size(); }

            void    clear();
            int     isEmpty() const;
            int     count() const;
            int     size() const    { return count(); }

            //FragmentedTreapMap<K,V, r, Node>* split(const_key _key);
            //bool merge(FragmentedTreapMap<K,V, r, Node>& _other);

            //throw some exceptions??

            const_key firstKey() const
            {
                if (isEmpty())
                {
                    throw std::underflow_error("FragmentedTreapMap firstKey (): tree empty");
                }

                return TreapUtil<K,V,Node>::findFirstNode(m_nodes.front()->root)->key;
            }

            const_key lastKey() const
            {
                if (isEmpty())
                {
                    throw std::underflow_error("FragmentedTreapMap lastKey (): tree empty");
                }

                return TreapUtil<K,V,Node>::findLastNode(m_nodes.back()->root)->key;
            }

            weight sumBefore(const_iterator i) const;

            weight sumBefore(const_key _key) const;
            weight sumTo(const_key _key) const;
            weight sumFrom(const_key _key) const;
            weight sumAfter(const_key _key) const;
            weight sum() const;

            virtual weight sumBetween(const_key _from, const_key _to) const { return sumTo(_to) - sumBefore(_from); }

            const_iterator  lowerBound(const_key _key) const;
            const_iterator  upperBound(const_key _key) const;

            const_iterator  find(const_key _key) const;
            const_iterator  find(const_key _key, const_reference _val) const;

            const_iterator  end() const     { return const_iterator(nullptr,
                                                                    m_nodes.end(),
                                                                     m_nodes); }
            const_iterator  begin() const;

            iterator        upperBound(const_key _key)     { return unconstIter(constThis().upperBound(_key)); }
            iterator        lowerBound(const_key _key)     { return unconstIter(constThis().lowerBound(_key)); }

            iterator        find(const_key _key,
                                 const_reference _val)     { return unconstIter(constThis().find(_key, _val)); }
            iterator        find(const_key _key)           { return unconstIter(constThis().find(_key)); }

            iterator        end()                          { return unconstIter(constThis().end()); }
            iterator        begin()                        { return unconstIter(constThis().begin()); }

            bool contains(const_key _key, const_reference _val) const { return find(_key, _val) != end(); }

        protected:
            /**
             * @brief Returns an iterator to the fraction that could contain _key
             * @param _key
             * @return Iterator, GUARANTEED to be valid (not at end()).
             */
            fnode_iterator iterForKey(const_key _key);
            const_fnode_iterator iterForKey(const_key _key) const;
            Node** rootForKey(const_key _key) const { return &((*iterForKey(_key))->root); }

            void checkAndClean(Node* root);

            std::list<fnode*> m_nodes;

            iterator unconstIter(const_iterator ci) const;


    };

    template<class K, class V, class R, class S>
    class FragmentedTreapMap1 : public FragmentedTreapMap<K, V, R, AugmentedTreapNode<K,V,S> >
    {};

    /* ####################################################################### */
    /* Iterator */

    template<typename T, typename value_ref>
    class FragmentedTreapMapIterator
    {
        friend class T::const_iterator;

            typedef const typename T::fnode_list&       node_list;
            typedef typename T::const_fnode_iterator    node_iterator;
            typedef typename T::node_type               node_type;
            typedef typename T::node_type::Node         subnode_type;
            typedef typename T::weight                  weight_type;
            typedef typename T::key_type                key_type;
            typedef typename T::value_type              value_type;

            node_type*      m_node;
            node_iterator   m_iterator;
            node_list       m_list;
            subnode_type*   m_cur;

        public:
            FragmentedTreapMapIterator(const typename T::iterator& i) :
                m_node(i.m_node),
                m_iterator(i.m_iterator),
                m_list(i.m_list),
                m_cur(i.m_cur)
            {
            }

            FragmentedTreapMapIterator(node_type* _node, node_iterator _it, node_list _list) :
                m_node(_node),
                m_iterator(_it),
                m_list(_list),
                m_cur(_node ? _node->first : nullptr)
            {
            }

            FragmentedTreapMapIterator(node_type* _node, subnode_type* _cur, node_iterator _it, node_list _list) :
                m_node(_node),
                m_iterator(_it),
                m_list(_list),
                m_cur(_cur)
            {
            }

            node_type* root() const
            {
                return m_iterator == m_list.end() ? nullptr
                                                   : (*m_iterator)->root;
            }

            bool operator==(const FragmentedTreapMapIterator<T, value_ref>& _other) const
            {
                return m_node == _other.m_node
                       && m_cur == _other.m_cur;
            }

            bool operator!=(const FragmentedTreapMapIterator<T, value_ref>& _other) const
            {
                return m_node != _other.m_node
                       || m_cur != _other.m_cur;
                      // || m_iterator != _other.m_iterator;
            }

            typename T::const_key key() const { return m_node->key; }
            value_ref value() const { return m_cur->value; }

            /**
             * @brief Weight of current element
             */
            weight_type weight() const { return m_cur ? m_cur->weight
                                                             : AugmentedTreapSum::makeEmpty<weight_type>(); }

            /**
             * @brief Sum of all elements with current key
             */
            weight_type sumCurrent() const { return m_node ? m_node->sum - m_node->sumLeft() - m_node->sumRight()
                                                                  : AugmentedTreapSum::makeEmpty<weight_type>(); }

            /**
             * @brief Sum of all elements with current key
             */
            weight_type sum() const { return m_node ? m_node->sum : AugmentedTreapSum::makeEmpty<weight_type>(); }

            value_ref& operator*()  { return m_cur->value; }

        FragmentedTreapMapIterator& operator++() // prefix ++
        {
            //Check first if we can go forward in the same node
            if (m_cur && m_cur->next)
            {
                m_cur = m_cur->next;
            }
            else
            {
                if (!m_node) //We're at the end
                {
                    // ++ from end(). get the root of the tree
                    m_iterator = m_list.begin();
                    m_node = TreapUtil<key_type, value_type, node_type>::findFirstNode(root());

                    // error! ++ requested for an empty tree
                    if (!m_node)
                        throw std::underflow_error("FragmentedTreapMapIteratorIterator operator++ (): tree empty");
                }
                else if (m_node->right)
                {
                    // successor is the furthest left node of
                    // right subtree
                    m_node = TreapUtil<key_type, value_type, node_type>::findFirstNode(m_node->right);
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
                    typename T::node_type* p = m_node->parent;

                    while (p && m_node == p->right)
                    {
                        m_node = p;
                        p = p->parent;
                    }

                    m_node = p;

                    //Check again if node is null, in which case we should try and go to the next fraction
                    if (!m_node)
                    {
                        ++m_iterator;
                        m_node = TreapUtil<key_type, value_type, node_type>::findFirstNode(root());
                    }
                }

                m_cur = m_node ? m_node->first : nullptr;
            }

            return *this;
        }

        FragmentedTreapMapIterator& operator--() // prefix --
        {
            //Check first if we can go backwards in the same node
            if (m_cur && m_cur->previous)
            {
                m_cur = m_cur->previous;
            }
            else
            {
                if (!m_node) // -- from end()
                {
                    --m_iterator;
                    m_node = TreapUtil<key_type, value_type, node_type>::findLastNode(root());

                    // error! -- requested for an empty tree
                    if (!m_node)
                        throw std::underflow_error("FragmentedTreapMapIteratorIterator operator-- (): tree empty");
                }
                else if (m_node->left)
                {
                    // successor is the furthest right node of
                    // right subtree
                    m_node = TreapUtil<key_type, value_type, node_type>::findLastNode(m_node->left);
                }
                else
                {
                    // have already processed the right subtree, and
                    // there is no left subtree. move up the tree,
                    // looking for a parent for which m_node is a right child,
                    // stopping if the parent becomes nullptr. a non-nullptr parent
                    // is the successor. if parent is nullptr, the original node
                    // was the last node inorder, and its successor
                    // is the end of the list
                    typename T::node_type* p = m_node->parent;

                    while (p && m_node == p->left)
                    {
                        m_node = p;
                        p = p->parent;
                    }

                    m_node = p;

                    //Check again if node is null, in which case we should try and go to the previous fraction
                    if (!m_node)
                    {
                        --m_iterator;
                        m_node = TreapUtil<key_type, value_type, node_type>::findLastNode(root());
                    }
                }

                m_cur = m_node ? m_node->last : nullptr;
            }

            return *this;
        }

        FragmentedTreapMapIterator operator++ (int)  // postfix ++
        {
           FragmentedTreapMapIterator result(*this);
           ++(*this);
           return result;
        }

        FragmentedTreapMapIterator operator-- (int)  // postfix --
        {
           FragmentedTreapMapIterator result(*this);
           --(*this);
           return result;
        }

        friend class FragmentedTreapMap<class T::key_type, class T::value_type, class T::ratio_type, class T::node_type>;
    };

    //DESIGN CONCERNS

    //Iterator should be modified to be able to jump to other treaps in the structure, only becoming end() if no
    //more treap after/before. Subclass and make op++ op-- virtual?

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::fnode_iterator
    FragmentedTreapMap<K,V, R, Node>::iterForKey(const_key _key)
    {
        auto next = m_nodes.begin();
        auto prev = next;

        do
        {
            prev = next;
            ++next;
        }
        while (next != m_nodes.end() && (*next)->start <= _key);

        return prev;
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::const_fnode_iterator
    FragmentedTreapMap<K,V, R, Node>::iterForKey(const_key _key) const
    {
        auto next = m_nodes.begin();
        auto prev = next;

        do
        {
            prev = next;
            ++next;
        }
        while (next != m_nodes.end() && (*next)->start <= _key);

        return prev;
    }

    template<class K, class V, class R, class Node>
    void FragmentedTreapMap<K,V, R, Node>::checkAndClean(Node* root)
    {
        if (!root) //A fraction will only be empty if the root is null.
        {
            auto i = m_nodes.begin(); ++i; //Start at 2nd node

            while (i != m_nodes.end())
            {
                if (!(*i)->root)
                {
                    delete *i;
                    i = m_nodes.erase(i);
                }
                else
                {
                    ++i;
                }
            }
        }
    }

    template<class K, class V, class R, class Node>
    void FragmentedTreapMap<K,V, R, Node>::insert(const_key _key, const_reference _value, const_weight _weight)
    {
        //Construct the node
        Node* newNode = new Node(_key, _value, _weight);
        TreapUtil<K,V,Node>::doInsert(newNode, rootForKey(_key));
    }

    template<class K, class V, class R, class Node>
    bool FragmentedTreapMap<K,V, R, Node>::remove(iterator i)
    {
        if (i == end())
            return false;

        TreapUtil<K,V,Node>::doRemove(i.m_node, i.m_root, i.m_cur);
        return true;
    }

    template<class K, class V, class R, class Node>
    bool FragmentedTreapMap<K,V, R, Node>::remove(const_key _key, const_reference _value)
    {
        //Try to find the node
        Node** root = rootForKey(_key);
        Node* n = TreapUtil<K,V,Node>::findEQ(_key, *root);

        if (!n)
            return false;

        typename Node::Node* v = n->find(_value);

        if (!v)
            return false;

        TreapUtil<K,V,Node>::doRemove(n, root, v);
        checkAndClean(*root);
        return true;
    }

    template<class K, class V, class R, class Node>
    bool FragmentedTreapMap<K,V, R, Node>::removeAll(const_key _key)
    {
        Node** root = rootForKey(_key);
        Node* n = TreapUtil<K,V,Node>::findEQ(_key, *root);

        if (!n)
            return false;

        TreapUtil<K,V,Node>::doRemove(n, root);
        checkAndClean(*root);
        return true;
    }

    template<class K, class V, class R, class Node>
    bool FragmentedTreapMap<K,V, R, Node>::move(const_key _currentKey, const_reference _value, const_key _newKey)
    {
        Node* cur = TreapUtil<K,V,Node>::findEQ(_currentKey, *rootForKey(_currentKey));

        if (cur)
        {
            auto sub = cur->find(_value);

            if (sub)
            {
                weight w = sub->weight;

                if (remove(_currentKey, _value))
                {
                    insert(_newKey, _value, w);
                    return true;
                }
            }
        }

        return false;
    }

    template<class K, class V, class R, class Node>
    bool FragmentedTreapMap<K,V, R, Node>::splitFragmentAt(const_key _key, const_ratio _val)
    {
        auto i = iterForKey(_key);

        if ((*i)->start == _key) //Already such a fragment
        {
            return false;
        }

        //Otherwise, split the current root at _key.
        fnode* n = new fnode(_key, _val, TreapUtil<K,V,Node>::doSplit(_key, &((*i)->root)));
        ++i;
        m_nodes.insert(i, n);
        return true;
    }

    template<class K, class V, class R, class Node>
    bool FragmentedTreapMap<K,V, R, Node>::setFragmentRatio(const_key _key, const_ratio _val)
    {
        for (auto i = m_nodes.begin(); i != m_nodes.end(); ++i)
        {
            if ((*i)->start == _key)
            {
                (*i)->ratio = _val;
                return true;
            }
        }

        return false;
    }

    template<class K, class V, class R, class Node>
    bool FragmentedTreapMap<K,V, R, Node>::joinFragmentsAt(const_key _key)
    {
        auto prev = m_nodes.begin();
        auto cur = prev;
        ++cur; //Start at second element

        for (; cur != m_nodes.end(); ++cur, ++prev)
        {
            if ((*cur)->start == _key)
            {
                if (!TreapUtil<K,V,Node>::doMerge(&((*prev)->root), &((*cur)->root)))
                    return false;

                //Otherwise, it worked, so remove the cur node
                delete *cur;
                m_nodes.erase(cur);
                return true;
            }
        }

        return false;
    }

    template<class K, class V, class R, class Node>
    bool FragmentedTreapMap<K,V, R, Node>::setWeight(const_key _key, const_reference _value, const_weight _weight)
    {
        Node* n = TreapUtil<K,V,Node>::findEQ(_key, *rootForKey(_key));

        if (!n)
            return false;

        //Find _value in the node
        weight diff = n->sum;

        if (!n->setWeight(_value, _weight))
        {
            return false;
        }

        diff -= n->sum;

        if (!AugmentedTreapSum::isEmpty<weight>(diff))
        {
            //Modify the weights of the parents
            while (n->parent)
            {
                n = n->parent;
                n->sum -= diff;
            }
        }

        return true;
    }

    template<class K, class V, class R, class Node>
    void FragmentedTreapMap<K,V, R, Node>::clear()
    {
        //Remove all fractions except first
        while (m_nodes.back() != m_nodes.front())
        {
            TreapUtil<K,V,Node>::doClear(&(m_nodes.back()->root)); //Will delete the root
            delete m_nodes.back();
            m_nodes.pop_back();
        }

        //Clear the first fraction
        TreapUtil<K,V,Node>::doClear(&(m_nodes.front()->root));
    }

    template<class K, class V, class R, class Node>
    int FragmentedTreapMap<K,V, R, Node>::count() const
    {
        int count = 0;

        for (fnode* n : m_nodes)
        {
            if (n->root)
            {
                count += n->root->count;
            }
        }

        return count;
    }

    template<class K, class V, class R, class Node>
    int FragmentedTreapMap<K,V, R, Node>::isEmpty() const
    {
        return m_nodes.front() == m_nodes.back()
                && !m_nodes.front()->root;
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::weight
    FragmentedTreapMap<K,V, R, Node>::sumBefore(const_key _key) const
    {
        weight w = AugmentedTreapSum::makeEmpty<weight>();

        for (auto i = m_nodes.begin(); i != m_nodes.end(); ++i)
        {
            if (!(*i)->root)
                continue;

            if (i != m_nodes.begin() && (*i)->start >= _key)
                break;

            w = FragmentedTreap::transform<R, weight>((*i)->ratio, w)
              + TreapUtil<K,V,Node>::leftSum(_key, false, (*i)->root);
        }

        return w;
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::weight
    FragmentedTreapMap<K,V, R, Node>::sumBefore(const_iterator i) const
    {
        //If invalid, return sum
        if (i.m_iterator == m_nodes.end()) return sum();

        //Otherwise, add everything before, ending with the tree pointed by the iterator.
        weight w = AugmentedTreapSum::makeEmpty<weight>();

        for (auto j = m_nodes.begin(); j != i.m_iterator && j != m_nodes.end(); ++j)
        {
            if (!(*j)->root)
                continue;

            w = FragmentedTreap::transform<R, weight>((*j)->ratio, w)
              + (*j)->root->sum;
        }

        //Now, add the left sum of the current tree
        if (i.m_iterator == m_nodes.begin() || (*i.m_iterator)->start < i.m_node->key)
        {
            w = FragmentedTreap::transform<R, weight>((*i.m_iterator)->ratio, w)
              + TreapUtil<K,V,Node>::leftSum(i.m_node, i.m_cur, false, (*i.m_iterator)->root);
        }

        return w;
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::weight
    FragmentedTreapMap<K,V, R, Node>::sumTo(const_key _key) const
    {
        weight w = AugmentedTreapSum::makeEmpty<weight>();

        for (auto i = m_nodes.begin(); i != m_nodes.end(); ++i)
        {
            if (i != m_nodes.begin() && (*i)->start > _key)
                break;

            if (!(*i)->root)
                continue;

            w = FragmentedTreap::transform<R, weight>((*i)->ratio, w)
              + TreapUtil<K,V,Node>::leftSum(_key, true, (*i)->root);
        }

        return w;
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::weight
    FragmentedTreapMap<K,V, R, Node>::sumFrom(const_key _key) const
    {
        weight w = AugmentedTreapSum::makeEmpty<weight>();

        //We still need to visit in order due to the transformed sums
        for (auto i = m_nodes.begin(); i != m_nodes.end(); ++i)
        {
            auto next = i; ++next;

            if (!(*i)->root || (next != m_nodes.end() && (*next)->start <= _key))
                continue; //Don't waist time here!

            w = FragmentedTreap::transform<R, weight>((*i)->ratio, w)
              + TreapUtil<K,V,Node>::rightSum(_key, true, (*i)->root);
        }

        return w;
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::weight
    FragmentedTreapMap<K,V, R, Node>::sumAfter(const_key _key) const
    {
        weight w = AugmentedTreapSum::makeEmpty<weight>();

        //We still need to visit in order due to the transformed sums
        for (auto i = m_nodes.begin(); i != m_nodes.end(); ++i)
        {
            auto next = i; ++next;

            if (!(*i)->root || (next != m_nodes.end() && (*next)->start <= _key))
                continue; //Don't waist time here!

            w = FragmentedTreap::transform<R, weight>((*i)->ratio, w)
              + TreapUtil<K,V,Node>::rightSum(_key, false, (*i)->root);
        }

        return w;
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::weight
    FragmentedTreapMap<K,V, R, Node>::sum() const
    {
        weight w = AugmentedTreapSum::makeEmpty<weight>();

        for (auto i = m_nodes.begin(); i != m_nodes.end(); ++i)
        {
            if (!(*i)->root)
                continue;

            w = FragmentedTreap::transform<R, weight>((*i)->ratio, w)
              + (*i)->root->sum;
        }

        return w;
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::const_iterator //First node with value >= w, nullptr if none.
    FragmentedTreapMap<K,V, R, Node>::lowerBound(const_key _key) const
    {
        //Find first treap with value at least _key.
        auto i = iterForKey(_key);

        //Now, the target is either in this tree, or it is the first node of the next tree
        //if that node exists.

        Node* n = TreapUtil<K,V,Node>::findLowerBound(_key, (*i)->root);

        if (!n && *i != m_nodes.back())
        {
            ++i;
            n = TreapUtil<K,V,Node>::findFirstNode((*i)->root);
        }
        else if (!n)
        {
            return end();
        }

        return const_iterator(n, i, m_nodes);
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::const_iterator //First node with value > w, nullptr if none.
    FragmentedTreapMap<K,V, R, Node>::upperBound(const_key _key) const
    {
        //Find first treap with value at least _key.
        auto i = iterForKey(_key);

        //Now, the target is either in this tree, or it is the first node of the next tree
        //if that node exists.

        Node* n = TreapUtil<K,V,Node>::findUpperBound(_key, (*i)->root);

        if (!n && *i != m_nodes.back())
        {
            ++i;
            n = TreapUtil<K,V,Node>::findFirstNode((*i)->root);
        }
        else if (!n)
        {
            return end();
        }

        return const_iterator(n, i, m_nodes);
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::const_iterator
    FragmentedTreapMap<K,V, R, Node>::find(const_key _key) const
    {
        auto i = iterForKey(_key);
        return const_iterator(TreapUtil<K,V,Node>::findEQ(_key, (*i)->root), i, m_nodes);
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::const_iterator
    FragmentedTreapMap<K,V, R, Node>::find(const_key _key, const_reference _val) const
    {
        auto i = iterForKey(_key);
        auto p = TreapUtil<K,V,Node>::findEQ(_key, _val, (*i)->root);

        if (p.first)
        {
            return iterator(p.first, p.second, i, m_nodes);
        }
        else
        {
            return end();
        }
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::const_iterator
    FragmentedTreapMap<K,V, R, Node>::begin() const
    {
        auto i = m_nodes.begin();

        if (!(*i)->root && m_nodes.size() > 1)
            ++i;

        return const_iterator(TreapUtil<K,V,Node>::findFirstNode((*i)->root), i, m_nodes);
    }

    template<class K, class V, class R, class Node>
    typename FragmentedTreapMap<K,V, R, Node>::iterator
    FragmentedTreapMap<K,V, R, Node>::unconstIter(const_iterator ci) const
    {
        return iterator(ci.m_node, ci.m_cur, ci.m_iterator, m_nodes);
    }
}

#endif // FRAGMENTEDTREAPMAP_H


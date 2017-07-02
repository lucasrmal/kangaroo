/*
 * Augmented Treap Map
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

#ifndef AUGMENTEDTREAPMAP_H
#define AUGMENTEDTREAPMAP_H

#include "treaputil.h"

namespace KLib
{
    template<class T, class V> class AugmentedTreapMapIterator;

    /**
     * K: Key
     * V: Value
     * S: Sum
     */
    template<class K, class V, class Node>
    class AugmentedTreapMap
    {
        public:
            typedef V&        reference;
            typedef const V&  const_reference;
            typedef const K&  const_key;
            typedef typename  Node::weight_type weight;
            typedef const weight& const_weight;
            typedef Node      node_type;
            typedef V         value_type;
            typedef K         key_type;

            typedef AugmentedTreapMapIterator<AugmentedTreapMap<K,V, Node>, reference> iterator;
            typedef AugmentedTreapMapIterator<const AugmentedTreapMap<K,V, Node>, const_reference> const_iterator;


            AugmentedTreapMap() :
                m_root(nullptr) {}

            AugmentedTreapMap(const AugmentedTreapMap&) = delete;
            AugmentedTreapMap& operator=(const AugmentedTreapMap&) = delete;

            /**
             * @brief Inserts an element with key _key, value _value and weight _weight
             *
             * Complexity: E[O(logn)]
             */
            virtual void insert(const_key _key, const_reference _value, const_weight _weight);

            /**
             * @brief Removes the element with key _key and value _value
             *
             * Complexity: E[O(logn + k)] where k is the number of elements with key _key
             *
             * @return True if element was found and removed, False if no such element exists.
             */
            virtual bool remove(const_key _key, const_reference _value);

            virtual bool remove(iterator i);
            bool erase(iterator i)  { return remove(i); }

            /**
             * @brief Removes all the elements with _key.
             *
             * Complexity: E[O(logn + k)] where k is the number of elements with key _key
             *
             * @return Return true if _key was found and removed, false if no such key exists.
             */
            virtual bool removeAll(const_key _key);

            virtual bool move(const_key _currentKey, const_reference _value, const_key _newKey);

            /**
             * @brief Changes the weight of the element with key _key and value _value
             *
             * Complexity: E[O(logn + k)] where k is the number of elements with key _key
             *
             * @return Return true if the element was found and its weight was changed, false if no such element exists.
             */
            virtual bool setWeight(const_key _key, const_reference _value, const_weight _weight);

            /**
             * @brief Clears the treap.
             *
             * Complexity: O(n)
             */
            virtual void clear()    { TreapUtil<K,V,Node>::doClear(&m_root); }

            /**
             * @brief Number of elements
             *
             * Complexity: O(1)
             */
            int size() const { return count(); }

            /**
             * @brief Number of elements
             *
             * Complexity: O(1)
             */
            virtual int count() const { return m_root ? m_root->count : 0; }

            /**
             * @brief Checks if the treap is empty
             *
             * Complexity: O(1)
             */
            virtual int isEmpty() const { return !m_root; }

            /**
             * @brief Splits the treap.
             *
             * This treap will now contain all elements with key < _key,
             * while the new returned treap will contain all elements with key >= _key.
             */
            virtual AugmentedTreapMap<K,V, Node>* split(const_key _key);

            /**
             * @brief Merges two treaps
             * @param _other The treap to merge with this treap. MUST have all keys > than all keys in this treap.
             *
             * @return True if successfull, false if other treap keys are smaller or equal to current treap.
             *
             * The other treap WILL BE CLEARED if this function is successfull.
             */
            virtual bool merge(AugmentedTreapMap<K,V, Node>& _other);

            virtual const_key firstKey() const
            {
                if (!m_root)
                {
                    throw std::underflow_error("AugmentedTreapMap firstKey (): tree empty");
                }

                return TreapUtil<K,V,Node>::findFirstNode(m_root)->key;
            }

            virtual const_key lastKey() const
            {
                if (!m_root)
                {
                    throw std::underflow_error("AugmentedTreapMap lastKey (): tree empty");
                }

                return TreapUtil<K,V,Node>::findLastNode(m_root)->key;
            }

            /**
             * @brief Sum of elements until _key (exclusive of _key).
             * @param _key
             *
             * Complexity: E[O(logn)]
             */
            virtual weight sumBefore(const_key _key) const { return TreapUtil<K,V,Node>::leftSum(_key, false, m_root); }

            /**
             * @brief Sum of elements until the element at i (exclusive of the element).
             *
             * If i == end(), returns sum().
             *
             * Complexity: E[O(logn + k)] where k is the number of elements with key _key
             */
            virtual weight sumBefore(const_iterator i) const { return TreapUtil<K,V,Node>::leftSum(i.m_node, i.m_cur, false, m_root); }

            /**
             * @brief Sum of elements until _key (inclusive of _key).
             * @param _key
             *
             * Complexity: E[O(logn)]
             */
            virtual weight sumTo(const_key _key) const { return TreapUtil<K,V,Node>::leftSum(_key, true, m_root); }

            /**
             * @brief Sum of elements after _key (inclusive of _key).
             * @param _key
             *
             * Complexity: E[O(logn)]
             */
            virtual weight sumFrom(const_key _key) const { return TreapUtil<K,V,Node>::rightSum(_key, true, m_root); }

            /**
             * @brief Sum of elements after _key (exclusive of _key).
             * @param _key
             *
             * Complexity: E[O(logn)]
             */
            virtual weight sumAfter(const_key _key) const { return TreapUtil<K,V,Node>::rightSum(_key, false, m_root); }

            /**
             * @brief Sum of elements between _from and _to (inclusive).
             * @param _key
             *
             * Complexity: E[O(logn)]
             */
            virtual weight sumBetween(const_key _from, const_key _to) const { return sumTo(_to) - sumBefore(_from); }

            /**
             * @brief Sum of weights of all elements
             *
             * Complexity: O(1)
             */
            virtual weight sum() const { return m_root ? m_root->sum : AugmentedTreapSum::makeEmpty<weight>(); }

            bool contains(const_key _key, const_reference _val) const { return find(_key, _val) != end(); }

            virtual const_iterator  lowerBound(const_key _key) const    { return const_iterator(TreapUtil<K,V,Node>::findLowerBound(_key, m_root), m_root); }
            virtual iterator        lowerBound(const_key _key)          { return iterator(TreapUtil<K,V,Node>::findLowerBound(_key, m_root), m_root); }

            virtual const_iterator  upperBound(const_key _key) const    { return const_iterator(TreapUtil<K,V,Node>::findUpperBound(_key, m_root), m_root); }
            virtual iterator        upperBound(const_key _key)          { return iterator(TreapUtil<K,V,Node>::findUpperBound(_key, m_root), m_root); }

            virtual iterator        find(const_key _key)                { return iterator(TreapUtil<K,V,Node>::findEQ(_key, m_root), m_root); }
            virtual const_iterator  find(const_key _key) const          { return const_iterator(TreapUtil<K,V,Node>::findEQ(_key, m_root), m_root); }

            virtual iterator        find(const_key _key, const_reference _val);
            virtual const_iterator  find(const_key _key, const_reference _val) const;

            virtual iterator        end()           { return iterator(nullptr, m_root); }
            virtual const_iterator  end() const     { return const_iterator(nullptr,  m_root); }

            virtual iterator        begin()         { return iterator(TreapUtil<K,V,Node>::findFirstNode(m_root), m_root); }
            virtual const_iterator  begin() const   { return const_iterator(TreapUtil<K,V,Node>::findFirstNode(m_root), m_root); }


        protected:
            Node* m_root;
    };

    template<class K, class V, class S>
    class AugmentedTreapMap1 : public AugmentedTreapMap<K, V, AugmentedTreapNode<K,V,S> >
    {};

    /* ####################################################################### */
    /* Iterator */

    template<typename T, typename valuetype>
    class AugmentedTreapMapIterator
    {
        friend class T::const_iterator;

        private:
            typedef typename T::node_type           node_type;
            typedef typename T::node_type::Node     subnode_type;
            typedef typename T::key_type            key_type;
            typedef typename T::value_type          value_type;
            typedef typename T::weight              weight_type;

            node_type* m_node;
            node_type* m_root;
            subnode_type* m_cur;

    public:
        AugmentedTreapMapIterator(const typename T::iterator& i) :
            m_node(i.m_node),
            m_root(i.m_root),
            m_cur(i.m_cur)
        {
        }

        AugmentedTreapMapIterator(node_type* _node, node_type* _root) :
            m_node(_node),
            m_root(_root),
            m_cur(_node ? _node->first : nullptr)
        {
        }

        AugmentedTreapMapIterator(node_type* _node,
                                  subnode_type* _cur,
                                  node_type* _root) :
            m_node(_node),
            m_root(_root),
            m_cur(_cur)
        {
        }

        bool operator==(const AugmentedTreapMapIterator<T, valuetype>& _other) const
        {
            return m_node == _other.m_node
                   && m_cur == _other.m_cur;
        }

        bool operator!=(const AugmentedTreapMapIterator<T, valuetype>& _other) const
        {
            return m_node != _other.m_node
                   || m_cur != _other.m_cur;
        }

        typename T::const_key key() const { return m_node->key; }
        valuetype value() const { return m_cur->value; }

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
        weight_type sum() const { return m_node ? m_node->sum
                                                : AugmentedTreapSum::makeEmpty<weight_type>(); }

        valuetype& operator*()  { return m_cur->value; }

        AugmentedTreapMapIterator& operator++() // prefix ++
        {
            //Check first if we can go forward in the same node
            if (m_cur && m_cur->next)
            {
                m_cur = m_cur->next;
            }
            else
            {
                if (!m_node)  // ++ from end(). get the first node of the tree
                {
                    m_node = TreapUtil<key_type, value_type, node_type>::findFirstNode(m_root);

                    // error! ++ requested for an empty tree
                    if (!m_node)
                        throw std::underflow_error("AugmentedTreapMapIterator operator++ (): tree empty");
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
                    node_type* p = m_node->parent;

                    while (p && m_node == p->right)
                    {
                        m_node = p;
                        p = p->parent;
                    }

                    // if we were previously at the right-most node in
                    // the tree, m_node = nullptr, and the iterator specifies
                    // the end of the list
                    m_node = p;
                }

                m_cur = m_node ? m_node->first : nullptr;
            }

            return *this;
        }

        AugmentedTreapMapIterator& operator--() // prefix --
        {
            //Check first if we can go forward in the same node
            if (m_cur && m_cur->previous)
            {
                m_cur = m_cur->previous;
            }
            else
            {
                if (!m_node)
                {
                    // ++ from end(). get the root of the tree
                    m_node = TreapUtil<key_type, value_type, node_type>::findLastNode(m_root);

                    // error! ++ requested for an empty tree
                    if (!m_node)
                        throw std::underflow_error("AugmentedTreapMapIterator operator-- (): tree empty");
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
                    node_type* p = m_node->parent;

                    while (p && m_node == p->left)
                    {
                        m_node = p;
                        p = p->parent;
                    }

                    // if we were previously at the left-most node in
                    // the tree, m_node = nullptr, and the iterator specifies
                    // the end of the list
                    m_node = p;
                }

                m_cur = m_node ? m_node->last : nullptr;
            }

            return *this;
        }

        AugmentedTreapMapIterator operator++ (int)  // postfix ++
        {
           AugmentedTreapMapIterator result(*this);
           ++(*this);
           return result;
        }

        AugmentedTreapMapIterator operator-- (int)  // postfix --
        {
           AugmentedTreapMapIterator result(*this);
           --(*this);
           return result;
        }

        friend class AugmentedTreapMap<class T::key_type, class T::value_type, class T::node_type>;
    };

    template<class K, class V, class Node>
    void AugmentedTreapMap<K,V, Node>::insert(const_key _key, const_reference _value, const_weight _weight)
    {
        //Construct the node
        Node* newNode = new Node(_key, _value, _weight);
        TreapUtil<K,V,Node>::doInsert(newNode, &m_root);
    }

    template<class K, class V, class Node>
    bool AugmentedTreapMap<K,V, Node>::remove(iterator i)
    {
        if (i == end())
            return false;

        TreapUtil<K,V,Node>::doRemove(i.m_node, &m_root, i.m_cur);
        return true;
    }

    template<class K, class V, class Node>
    bool AugmentedTreapMap<K,V, Node>::remove(const_key _key, const_reference _value)
    {
        //Try to find the node
        Node* n = TreapUtil<K,V,Node>::findEQ(_key, m_root);

        if (!n)
            return false;

        typename Node::Node* v = n->find(_value);

        if (!v)
            return false;

        TreapUtil<K,V,Node>::doRemove(n, &m_root, v);
        return true;
    }

    template<class K, class V, class Node>
    bool AugmentedTreapMap<K, V, Node>::removeAll(const_key _key)
    {
        Node* n = TreapUtil<K,V,Node>::findEQ(_key, m_root);

        if (!n)
            return false;

        TreapUtil<K,V,Node>::doRemove(n, &m_root);
        return true;
    }

    template<class K, class V, class Node>
    bool AugmentedTreapMap<K,V, Node>::move(const_key _currentKey, const_reference _value, const_key _newKey)
    {
        Node* cur = TreapUtil<K,V,Node>::findEQ(_currentKey, m_root);

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

    template<class K, class V, class Node>
    bool AugmentedTreapMap<K,V, Node>::setWeight(const_key _key, const_reference _value, const_weight _weight)
    {
        Node* n = TreapUtil<K,V,Node>::findEQ(_key, m_root);

        if (!n)
            return false;

        //Find _value in the node
        weight diff = n->sum;

        if (!n->setWeight(_value, _weight))
        {
            return false;
        }

        diff -= n->sum;

        if (AugmentedTreapSum::isEmpty<weight>(diff))
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

    template<class K, class V, class Node>
    AugmentedTreapMap<K,V, Node>* AugmentedTreapMap<K,V, Node>::split(const_key _key)
    {
        AugmentedTreapMap<K,V, Node>* oth = new AugmentedTreapMap<K,V, Node>();
        oth->m_root = TreapUtil<K,V,Node>::doSplit(_key, &m_root);
        return oth;
    }

    /**
     * @brief Merges two treaps
     * @param _other The treap to merge with this treap. MUST have all keys > than all keys in this treap.
     *
     * @return True if successfull, false if other treap keys are smaller or equal to current treap.
     *
     * The other treap WILL BE CLEARED if this function is successfull.
     */
    template<class K, class V, class Node>
    bool AugmentedTreapMap<K,V, Node>::merge(AugmentedTreapMap<K,V, Node>& _other)
    {
        return TreapUtil<K,V,Node>::doMerge(&m_root, &(_other.m_root));
    }

    template<class K, class V, class Node> //First node with value >= w, nullptr if none.
    typename AugmentedTreapMap<K,V, Node>::iterator AugmentedTreapMap<K,V, Node>::find(const_key _key,
                                                                                       const_reference _val)
    {
        auto p = TreapUtil<K,V,Node>::findEQ(_key, _val, m_root);

        if (p.first)
        {
            return iterator(p.first, p.second, m_root);
        }
        else
        {
            return end();
        }
    }

    template<class K, class V, class Node> //First node with value >= w, nullptr if none.
    typename AugmentedTreapMap<K,V, Node>::const_iterator
    AugmentedTreapMap<K,V, Node>::find(const_key _key, const_reference _val) const
    {
        auto p = TreapUtil<K,V,Node>::findEQ(_key, _val, m_root);

        if (p.first)
        {
            return const_iterator(p.first, p.second, m_root);
        }
        else
        {
            return end();
        }
    }

}

#endif // AUGMENTEDTREAPMAP_H


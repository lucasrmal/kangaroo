/*
 * Treap Util - Utility functions for AugmentedTreapMap and FractionalTreapMap
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

#ifndef TREAPUTIL_H
#define TREAPUTIL_H

#include <utility>
#include <assert.h>

namespace KLib
{
    namespace AugmentedTreapSum
    {
        template<class S>
        bool isEmpty(const S& _s) {
            return _s != 0;
        }

        template<class S>
        S makeEmpty() {
            return S(0);
        }

    }

    template<class K, class V, class S>
    struct AugmentedTreapNode
    {
        struct Node
        {
            Node(const V& _v, const S& _w, Node* _next = nullptr) :
                value(_v),
                weight(_w),
                next(_next),
                previous(nullptr)
            {
                if (_next)
                {
                    _next->previous = this;
                }
            }

            V value;
            S weight;
            Node* next;
            Node* previous;
        };

        typedef S weight_type;

        AugmentedTreapNode(const K& _key) :
            key(_key),
            first(nullptr),
            last(nullptr),
            sum(AugmentedTreapSum::makeEmpty<S>()),
            count(0),
            parent(nullptr),
            left(nullptr),
            right(nullptr)
        {

        }

        AugmentedTreapNode(const K& _key, const V& _value, const S& _weight) :
            key(_key),
            first(new Node(_value, _weight)),
            last(first),
            sum(_weight),
            count(1),
            p(rand()),
            parent(nullptr),
            left(nullptr),
            right(nullptr)
        {
        }

        ~AugmentedTreapNode()
        {
            Node* next = first;
            Node* cur = nullptr;

            while (next)
            {
                cur = next;
                next = cur->next;
                delete cur;
            }
        }

        bool empty() const
        {
            return !first;
        }

        /**
         * @brief compare
         * @param _other
         * @return -1 if THIS < OTHER, 0 if THIS == OTHER, 1 if THIS > OTHER
         */
        int compare(AugmentedTreapNode& _other) const
        {
            return key < _other.key ? -1
                                    : (key == _other.key ? 0
                                                         : 1);
        }

        weight_type sumLeft() const
        {
            return left ? left->sum
                        : AugmentedTreapSum::makeEmpty<weight_type>();
        }

        weight_type sumRight() const
        {
            return right ? right->sum
                         : AugmentedTreapSum::makeEmpty<weight_type>();
        }

        Node* find(const V& _value) const
        {
            Node* cur = first;

            while (cur && cur->value != _value)
            {
                cur = cur->next;
            }

            return cur;
        }

        bool setWeight(const V& _value, const S& _weight)
        {
            Node* cur = find(_value);

            if (cur)
            {
                sum += _weight-cur->weight;
                cur->weight = _weight;
                return true;
            }
            else
            {
                return false;
            }
        }

        void add(const V& _value, const S& _weight)
        {
            first = new Node(_value, _weight, first); //This takes care of the previous and next pointers.
            sum += _weight;

            if (!last)
            {
                last = first;
            }

            ++count;
        }

        void removeFirst(S& _weightRemoved)
        {
            Node* temp = first;

            if (temp == last)
            {
                last = nullptr;
            }

            sum -= temp->weight;
            _weightRemoved = temp->weight;

            first = temp->next;
            first->previous = nullptr;
            delete temp;

            --count;
        }

        bool removeOne(const V& _value, S& _weightRemoved)
        {
            Node* cur = first;
            Node* prev = nullptr;

            while (cur && cur->value != _value)
            {
                cur = cur->next;
                prev = cur;
            }

            if (cur)
            {
                if (cur->next)
                {
                    cur->next->previous = prev;
                }
                else //Removing the last one...
                {
                    last = prev;
                }

                if (prev)
                {
                    prev->next = cur->next;
                }
                else //If cur == first
                {
                    first = cur->next;
                }

                _weightRemoved = cur->weight;
                sum -= cur->weight;

                --count;

                delete cur;
                return true;
            }
            else
            {
                return false;
            }
        }

        void remove(Node* v)
        {
            //Pointers
            if (v->previous)
            {
                v->previous->next = v->next;
            }

            if (v->next)
            {
                v->next->previous = v->previous;
            }

            if (v == first)
            {
                first = v->next;
            }

            if (v == last)
            {
                last = v->previous;
            }

            //Sum
            sum -= v->weight;
            --count;
        }

        K key;
        Node* first;
        Node* last;
        S sum;
        int count;
        int p;

        AugmentedTreapNode<K,V,S>* parent;
        AugmentedTreapNode<K,V,S>* left;
        AugmentedTreapNode<K,V,S>* right;
    };

    template<class K,class V, class Node>
    struct TreapUtil
    {
        typedef const K&  const_key;
        typedef typename  Node::weight_type weight_type;
        typedef const V&  const_reference;
        typedef std::pair<Node*, typename Node::Node*> node_sub_pair;

        static Node* findFirstNode(Node* root);
        static Node* findLastNode(Node* root);
        static Node* findUpperBound(const_key _key, Node* root);
        static Node* findLowerBound(const_key _key, Node* root);
        static Node* findEQ(const_key _key, Node* root);
        static node_sub_pair findEQ(const_key _key, const_reference _val, Node* root);

        static Node* doSplit(const_key _key, Node** root);
        static bool  doMerge(Node** root, Node** otherRoot);

        static void rotateRight(Node *u, Node** root);
        static void rotateLeft(Node *u, Node** root);

        static void bubbleUp(Node *u, Node** root);
        static void trickleDown(Node *u, Node** root);
        static void splice(Node *u, Node** root);

        static void doInsert(Node* u, Node** root);
        static void doRemove(Node* u, Node** root, typename Node::Node* v = nullptr);

        static void doClear(Node** root);

        static weight_type leftSum(const_key _to, bool inclusive, Node* root);
        static weight_type leftSum(Node* cur, typename Node::Node* _sub, bool inclusive, Node* root);
        static weight_type rightSum(const_key _from, bool inclusive, Node* root);

        enum Direction
        {
            Left,
            Right
        };
    };

    template<class K, class V, class Node>
    Node* TreapUtil<K,V, Node>::findFirstNode(Node* root)
    {
        Node* cur = root;

        while (cur && cur->left)
        {
            cur = cur->left;
        }

        return cur;
    }

    template<class K, class V, class Node>
    Node* TreapUtil<K,V, Node>::findLastNode(Node* root)
    {
        Node* cur = root;

        while (cur && cur->right)
        {
            cur = cur->right;
        }

        return cur;
    }

    template<class K, class V, class Node> //First node with value > w, nullptr if none.
    Node* TreapUtil<K,V, Node>::findUpperBound(const_key _key, Node* root)
    {
        Node* cur = root;
        Node* bestUB = nullptr;

        while (cur)
        {
            if (_key < cur->key)
            {
                bestUB = cur;
                cur = cur->left;
            }
            else //(_key >= cur->key) do not modify the best successor in this case...
            {
                cur = cur->right;
            }
        }

        return bestUB;
    }

    template<class K, class V, class Node> //First node with value >= w, nullptr if none.
    Node* TreapUtil<K,V, Node>::findLowerBound(const_key _key, Node* root)
    {
        Node* cur = root;
        Node* bestLB = nullptr;

        while (cur)
        {
            if (_key < cur->key)
            {
                bestLB = cur;
                cur = cur->left;
            }
            else if (_key > cur->key)
            {
                cur = cur->right;
            }
            else //We found it
            {
                return cur;
            }
        }

        return bestLB;
    }

    template<class K, class V, class Node> //First node with value >= w, nullptr if none.
    Node* TreapUtil<K,V, Node>::findEQ(const_key _key, Node* root)
    {
        Node* cur = root;

        while (cur)
        {
            if (_key < cur->key)
            {
                cur = cur->left;
            }
            else if (_key > cur->key)
            {
                cur = cur->right;
            }
            else //We found it
            {
                return cur;
            }
        }

        return cur;
    }

    template<class K, class V, class Node> //First node with value >= w, nullptr if none.
    typename TreapUtil<K,V, Node>::node_sub_pair
    TreapUtil<K,V, Node>::findEQ(const_key _key, const_reference _val, Node* root)
    {
        Node* cur = findEQ(_key, root);

        if (cur)
        {
            auto sub = cur->find(_val);

            if (sub)
            {
                return node_sub_pair(cur, sub);
            }
        }

        return node_sub_pair(nullptr, nullptr);
    }

    template<class K, class V, class Node>
    void TreapUtil<K,V, Node>::doClear(Node** root)
    {
        Node* u = *root, *prev = nullptr, *next;

        while (u)
        {
            if (prev == u->parent)
            {
                if (u->left)
                {
                    next = u->left;
                }
                else if (u->right)
                {
                    next = u->right;
                }
                else
                {
                    next = u->parent;
                }

            }
            else if (prev == u->left)
            {
                if (u->right)
                {
                    next = u->right;
                }
                else
                {
                    next = u->parent;
                }
            }
            else
            {
                next = u->parent;
            }

            prev = u;

            if (next == u->parent)
            {
                delete u;
            }

            u = next;
        }

        *root = nullptr;
    }

    /**
     * Splits in two parts: Treap with keys < than x, and treap with keys >= x.
     */
    template<class K, class V, class Node>
    Node* TreapUtil<K,V, Node>::doSplit(const_key _key, Node** root)
    {
        Node* u = findLowerBound(_key, *root);
        Node* otherRoot = nullptr;

        if (u)
        {
            //Change priority to lowest
            u->p = -1;

            //Bubble it up to the top!
            bubbleUp(u, root);

            //Now u must be the root.
            assert(u == *root);

            *root = u->left;
            otherRoot = u->right;

            //Adjust the new roots
            if (*root) //If not empty!
            {
                (*root)->parent = nullptr;
                u->count -= (*root)->count;
                u->sum -= (*root)->sum;
            }

            if (otherRoot)
            {
                otherRoot->parent = nullptr;
                u->count -= otherRoot->count;
                u->sum -= otherRoot->sum;
            }

            //Cut off u
            u->parent = u->left = u->right = nullptr;

            //Get back standard priority for u
            u->p = rand();

            //Insert it back in the new treap
            doInsert(u, &otherRoot);
        }
        //Else, no value at least x, new treap is empty, so we return nullptr.

        return otherRoot;
    }

    template<class K, class V, class Node>
    bool TreapUtil<K,V, Node>::doMerge(Node** root, Node** otherRoot)
    {
        if (!*otherRoot)
        {
            return true;
        }

        const_key x = findFirstNode(*otherRoot)->key;

        if (!*root)
        {
            *root = *otherRoot;
            *otherRoot = nullptr;
            return true;
        }
        else if (findLastNode(*root)->key >= x)
        {
            return false;
        }

        //Create new root with any value, put two treaps as subchilds
        Node* newRoot = new Node(x);
        newRoot->p = -1;
        newRoot->left = *root;
        newRoot->right = *otherRoot;
        newRoot->left->parent = newRoot;
        newRoot->right->parent = newRoot;
        newRoot->sum = newRoot->left->sum + newRoot->right->sum;
        newRoot->count = newRoot->left->count + newRoot->right->count + 1;

        //New roots, clear other treap
        *root = newRoot;
        *otherRoot = nullptr;

        //Trickle down this new root, splice it and delete it.
        doRemove(newRoot, root);

        return true;
    }

    template<class K, class V, class Node>
    void TreapUtil<K,V, Node>::splice(Node *u, Node** root)
    {
        assert(u != nullptr);

        Node *s, *p;

        if (u->left)
        {
            s = u->left;
        }
        else
        {
            s = u->right;
        }

        if (u == *root)
        {
            *root = s;
            p = nullptr;
        }
        else
        {
            p = u->parent;

            if (p->left == u)
            {
                p->left = s;
            }
            else
            {
                p->right = s;
            }
        }

        if (s)
        {
            s->parent = p;
        }
    }

    template<class K, class V, class Node>
    void TreapUtil<K,V, Node>::rotateRight(Node *u, Node** root)
    {
        assert(u != nullptr);

        Node *w = u->left;
        w->parent = u->parent;

        auto newSumU = u->sum
                       - w->sum
                       + (w->right ? w->right->sum : AugmentedTreapSum::makeEmpty<weight_type>());

        auto newSumW = w->sum
                       - (w->right ? w->right->sum : AugmentedTreapSum::makeEmpty<weight_type>())
                       + newSumU;

        if (w->parent)
        {
            if (w->parent->left == u)
            {
                w->parent->left = w;
            }
            else
            {
                w->parent->right = w;
            }
        }
        u->left = w->right;

        if (u->left)
        {
            u->left->parent = u;
        }

        u->parent = w;
        w->right = u;

        u->sum = newSumU;
        w->sum = newSumW;


        u->count = u->count - w->count + (u->left ? u->left->count : 0);
        w->count = w->count - (u->left ? u->left->count : 0) + u->count;

        if (u == *root)
        {
            *root = w;
            (*root)->parent = nullptr;
        }
    }

    template<class K, class V, class Node>
    void TreapUtil<K,V, Node>::rotateLeft(Node *u, Node** root)
    {
        assert(u != nullptr);
        assert(*root != nullptr);

        Node *w = u->right;
        w->parent = u->parent;

        auto newSumU = u->sum
                       - w->sum
                       + (w->left ? w->left->sum : AugmentedTreapSum::makeEmpty<weight_type>());

        auto newSumW = w->sum
                       - (w->left ? w->left->sum : AugmentedTreapSum::makeEmpty<weight_type>())
                       + newSumU;

        if (w->parent)
        {
            if (w->parent->left == u)
            {
                w->parent->left = w;
            }
            else
            {
                w->parent->right = w;
            }
        }
        u->right = w->left;

        if (u->right)
        {
            u->right->parent = u;
        }

        u->parent = w;
        w->left = u;

        u->sum = newSumU;
        w->sum = newSumW;


        u->count = u->count - w->count + (u->right ? u->right->count : 0);
        w->count = w->count - (u->right ? u->right->count : 0) + u->count;

        if (u == *root)
        {
            *root = w;
            (*root)->parent = nullptr;
        }
    }

    template<class K, class V, class Node>
    void TreapUtil<K,V, Node>::doInsert(Node* u, Node** root)
    {
        assert(u != nullptr);

        if (!*root)
        {
            *root = u;
        }
        else
        {
            Node* cur = *root;
            Node* newParent = nullptr;

            //================= HELPER FUNCTIONS =================

            auto attachNode = [u] (Node* _parent, Direction _dir) {

                if (_dir == Left)
                {
                    _parent->left = u;
                }
                else
                {
                    _parent->right = u;
                }

                u->parent = _parent;
            };

            auto moveDown = [u] (Node* _parent, Direction _dir) {

                _parent->sum += u->sum;
                ++_parent->count;

                return _dir == Left ? _parent->left
                                    : _parent->right;
            };


            //================= WALK DOWN TREE =================

            while (cur)
            {
                newParent = cur;

                int comp = u->compare(*cur);

                if (comp < 0)
                {
                    cur = moveDown(cur, Left);
                }
                else if (comp > 0)
                {
                    cur = moveDown(cur, Right);
                }
                else
                {
                    break;
                }
            }

            //Add it as a child
            int comp = u->compare(*newParent);

            if (comp < 0)
            {
                attachNode(newParent, Left);
            }
            else if (comp > 0)
            {
                attachNode(newParent, Right);
            }
            else //Already exists
            {
                auto n = u->first;

                while (n)
                {
                    newParent->add(n->value, n->weight);
                    n = n->next;
                }

                delete u;
                u = nullptr;
            }

            //================= CORRECT THE P's =================
            if (u)
            {
                bubbleUp(u, root);
            }
        }
    }

    template<class K, class V, class Node>
    void TreapUtil<K, V, Node>::doRemove(Node* u, Node** root, typename Node::Node* v)
    {
        assert(u != nullptr);

        bool removeFull = !v || (!v->previous && !v->next);
        weight_type w;
        Node* p;

        if (!removeFull)
        {
            w = v->weight;
            u->remove(v);
            p = u->parent;
        }
        else
        {
            trickleDown(u, root);
            w = u->sum - u->sumLeft() - u->sumRight();
            p = u->parent;
            splice(u, root);
            delete u;
        }

        while (p)
        {
            --p->count;
            p->sum -= w;
            p = p->parent;
        }
    }

    template<class K, class V, class Node>
    void TreapUtil<K,V, Node>::bubbleUp(Node *u, Node** root)
    {
        assert(u != nullptr);

        while (u->parent && u->parent->p > u->p)
        {
            if (u->parent->right == u)
            {
                rotateLeft(u->parent, root);
            }
            else
            {
                rotateRight(u->parent, root);
            }
        }

        if (!u->parent)
        {
            *root = u;
        }
    }

    template<class K, class V, class Node>
    void TreapUtil<K,V, Node>::trickleDown(Node *u, Node** root)
    {
        assert(u != nullptr);

        while (u->left || u->right)
        {
            if (!u->left)
            {
                rotateLeft(u, root);
            }
            else if (!u->right)
            {
                rotateRight(u, root);
            }
            else if (u->left->p < u->right->p)
            {
                rotateRight(u, root);
            }
            else
            {
                rotateLeft(u, root);
            }

            if (*root == u)
            {
                *root = u->parent;
            }
        }
    }

    template<class K, class V, class Node>
    typename TreapUtil<K,V, Node>::weight_type
    TreapUtil<K,V, Node>::leftSum(const_key _to, bool inclusive, Node* root)
    {
        Node* cur = root;
        weight_type sum = AugmentedTreapSum::makeEmpty<weight_type>();

        while (cur)
        {
            if (_to < cur->key)
            {
                cur = cur->left;
            }
            else if (inclusive)// (_cur->key <= _to)
            {
                sum += cur->sum - cur->sumRight();

                if (cur->key == _to)
                {
                    break;
                }
                cur = cur->right;

            }
            else if (cur->key < _to) //Not inclusive
            {
                sum += cur->sum - cur->sumRight();
                cur = cur->right;
            }
            else //Found the node, Not inclusive
            {
                sum += cur->sumLeft();
                break;
            }
        }

        return sum;
    }

    template<class K, class V, class Node>
    typename TreapUtil<K,V, Node>::weight_type
    TreapUtil<K,V, Node>::leftSum(Node* cur, typename Node::Node* sub, bool inclusive, Node* root)
    {
        if (!root)
            return AugmentedTreapSum::makeEmpty<weight_type>();

        if (!cur || !sub)
            return root->sum;

        weight_type sum = AugmentedTreapSum::makeEmpty<weight_type>();

        //Compute the sum until the element pointed by the iterator
        auto n = cur->first;

        while (n && (n != sub || inclusive))
        {
            sum += n->weight;
            n = n->next;

            if (n == sub)
                break;
        }

        //Compute the sum until the root
        sum += cur->sumLeft();

        while (cur->parent)
        {
            Node* prev = cur;
            cur = cur->parent;

            if (prev == cur->right)
            {
                sum += cur->sum - cur->sumRight();
            }
        }

        return sum;
    }

    template<class K, class V, class Node>
    typename TreapUtil<K,V, Node>::weight_type TreapUtil<K,V, Node>::rightSum(const_key _from, bool inclusive, Node* root)
    {
        Node* cur = root;
        weight_type sum = AugmentedTreapSum::makeEmpty<weight_type>();

        while (cur)
        {
            if (_from > cur->key)
            {
                cur = cur->right;
            }
            else if (inclusive) // (_cur->key <= _key)
            {
                sum += cur->sum - cur->sumLeft();

                if (cur->key == _from)
                {
                    break;
                }
                cur = cur->left;

            }
            else if (_from < cur->key) //Not inclusive
            {
                sum += cur->sum - cur->sumLeft();
                cur = cur->left;
            }
            else //Found the node, Not inclusive
            {
                sum += cur->sumRight();
                break;
            }
        }

        return sum;
    }

}

#endif // TREAPUTIL_H


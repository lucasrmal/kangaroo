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

#include "util/treap/treap-util.h"

template <class K, class V, class S>
struct AugmentedTreapNode;

namespace kangaroo {
template <class Map>
class AugmentedTreapMapIterator;

/**
 * K: Key
 * V: Value
 * S: Weight
 */
template <class K, class V, class S>
class AugmentedTreapMap {
 public:
  using Node = AugmentedTreapNode<K, V, S>;
  using key_type = K;
  using value_type = V;
  using weight_type = S;

  using reference = V&;
  using const_reference = const V&;
  using const_key = const K&;
  using weight = typename Node::weight_type;
  using const_weight = const weight&;

  using iterator = AugmentedTreapMapIterator<AugmentedTreapMap<K, V, S>>;

  AugmentedTreapMap() : m_root(nullptr) {}

  AugmentedTreapMap(const AugmentedTreapMap&) = delete;
  AugmentedTreapMap& operator=(const AugmentedTreapMap&) = delete;

  /**
   * @brief Inserts an element with key _key, value _value and weight _weight
   *
   * Complexity: E[O(logn)]
   */
  virtual void Insert(const_key _key, const_reference _value,
                      const_weight _weight);

  /**
   * @brief Removes the element with key _key and value _value
   *
   * Complexity: E[O(logn + k)] where k is the number of elements with key _key
   *
   * @return True if element was found and removed, False if no such element
   * exists.
   */
  virtual bool Remove(const_key _key, const_reference _value);

  virtual bool Remove(iterator i);
  bool erase(iterator i) { return remove(i); }

  /**
   * @brief Removes all the elements with _key.
   *
   * Complexity: E[O(logn + k)] where k is the number of elements with key _key
   *
   * @return Return true if _key was found and removed, false if no such key
   * exists.
   */
  virtual bool RemoveAll(const_key _key);

  virtual bool Move(const_key _currentKey, const_reference _value,
                    const_key _newKey);

  /**
   * @brief Changes the weight of the element with key _key and value _value
   *
   * Complexity: E[O(logn + k)] where k is the number of elements with key _key
   *
   * @return Return true if the element was found and its weight was changed,
   * false if no such element exists.
   */
  virtual bool SetWeight(const_key _key, const_reference _value,
                         const_weight _weight);

  /**
   * @brief Clears the treap.
   *
   * Complexity: O(n)
   */
  virtual void Clear() { TreapUtil<Node>::doClear(&m_root); }

  /**
   * @brief Number of elements
   *
   * Complexity: O(1)
   */
  virtual int size() const { return m_root ? m_root->count : 0; }

  /**
   * @brief Checks if the treap is empty
   *
   * Complexity: O(1)
   */
  virtual int empty() const { return !m_root; }

  /**
   * @brief Splits the treap.
   *
   * This treap will now contain all elements with key < _key,
   * while the new returned treap will contain all elements with key >= _key.
   */
  virtual AugmentedTreapMap<K, V, S>* Split(const_key _key);

  /**
   * @brief Merges two treaps
   * @param _other The treap to merge with this treap. MUST have all keys > than
   * all keys in this treap.
   *
   * @return True if successfull, false if other treap keys are smaller or equal
   * to current treap.
   *
   * The other treap WILL BE CLEARED if this function is successfull.
   */
  virtual bool Merge(AugmentedTreapMap<K, V, S>& _other);

  virtual const_key first_key() const {
    if (!m_root) {
      throw std::underflow_error("AugmentedTreapMap first_key (): tree empty");
    }

    return TreapUtil<Node>::findFirstNode(m_root)->key;
  }

  virtual const_key last_key() const {
    if (!m_root) {
      throw std::underflow_error("AugmentedTreapMap last_key (): tree empty");
    }

    return TreapUtil<Node>::findLastNode(m_root)->key;
  }

  /**
   * @brief Weight of elements until _key (exclusive of _key).
   * @param _key
   *
   * Complexity: E[O(logn)]
   */
  virtual weight_type WeightBefore(const_key _key) const {
    return TreapUtil<Node>::leftWeight(_key, false, m_root);
  }

  /**
   * @brief Weight of elements until the element at i (exclusive of the
   * element).
   *
   * If i == end(), returns weight().
   *
   * Complexity: E[O(logn + k)] where k is the number of elements with key _key
   */
  virtual weight_type WeightBeforeIterator(iterator i) const {
    return TreapUtil<Node>::leftWeight(i.m_node, i.m_cur, false, m_root);
  }

  /**
   * @brief Weight of elements until _key (inclusive of _key).
   * @param _key
   *
   * Complexity: E[O(logn)]
   */
  virtual weight_type WeightTo(const_key _key) const {
    return TreapUtil<Node>::leftWeight(_key, true, m_root);
  }

  /**
   * @brief Weight of elements after _key (inclusive of _key).
   * @param _key
   *
   * Complexity: E[O(logn)]
   */
  virtual weight_type WeightFrom(const_key _key) const {
    return TreapUtil<Node>::rightWeight(_key, true, m_root);
  }

  /**
   * @brief Weight of elements after _key (exclusive of _key).
   * @param _key
   *
   * Complexity: E[O(logn)]
   */
  virtual weight_type WeightAfter(const_key _key) const {
    return TreapUtil<Node>::rightWeight(_key, false, m_root);
  }

  /**
   * @brief Weight of elements between _from and _to (inclusive).
   * @param _key
   *
   * Complexity: E[O(logn)]
   */
  virtual weight_type WeightBetween(const_key _from, const_key _to) const {
    return WeightTo(_to) - WeightBefore(_from);
  }

  /**
   * @brief Weight of weights of all elements
   *
   * Complexity: O(1)
   */
  virtual weight_type Weight() const {
    return m_root ? m_root->weight : AugmentedTreapWeight::makeEmpty<weight>();
  }

  bool Contains(const_key _key, const_reference _val) const {
    return find(_key, _val) != end();
  }

  virtual iterator LowerBound(const_key _key) const {
    return iterator(TreapUtil<Node>::findLowerBound(_key, m_root), m_root);
  }
  virtual iterator LowerBound(const_key _key) {
    return iterator(TreapUtil<Node>::findLowerBound(_key, m_root), m_root);
  }

  virtual iterator UpperBound(const_key _key) const {
    return iterator(TreapUtil<Node>::findUpperBound(_key, m_root), m_root);
  }
  virtual iterator UpperBound(const_key _key) {
    return iterator(TreapUtil<Node>::findUpperBound(_key, m_root), m_root);
  }

  virtual iterator find(const_key _key) {
    return iterator(TreapUtil<Node>::findEQ(_key, m_root), m_root);
  }
  virtual iterator find(const_key _key) const {
    return iterator(TreapUtil<Node>::findEQ(_key, m_root), m_root);
  }

  virtual iterator find(const_key _key, const_reference _val) const;

  virtual iterator end() const { return iterator(nullptr, m_root); }

  virtual iterator begin() const {
    return iterator(TreapUtil<Node>::findFirstNode(m_root), m_root);
  }

 protected:
  Node* m_root;
};

/* ####################################################################### */
/* Iterator */

template <class Map>
class AugmentedTreapMapIterator {
  friend Map;

  using node_type = typename Map::Node;
  using subnode_type = typename Map::Node::Node;
  using key_type = typename Map::key_type;
  using value_type = typename Map::value_type;
  using weight_type = typename Map::weight;

  node_type* m_node;
  node_type* m_root;
  subnode_type* m_cur;

 public:
  AugmentedTreapMapIterator(const typename Map::iterator& i)
      : m_node(i.m_node), m_root(i.m_root), m_cur(i.m_cur) {}

  AugmentedTreapMapIterator(node_type* _node, node_type* _root)
      : m_node(_node), m_root(_root), m_cur(_node ? _node->first : nullptr) {}

  AugmentedTreapMapIterator(node_type* _node, subnode_type* _cur,
                            node_type* _root)
      : m_node(_node), m_root(_root), m_cur(_cur) {}

  bool operator==(const AugmentedTreapMapIterator<Map>& _other) const {
    return m_node == _other.m_node && m_cur == _other.m_cur;
  }

  bool operator!=(const AugmentedTreapMapIterator<Map>& _other) const {
    return m_node != _other.m_node || m_cur != _other.m_cur;
  }

  key_type key() const { return m_node->key; }
  value_type value() const { return m_cur->value; }

  /**
   * @brief Weight of current element
   */
  weight_type weight() const {
    return m_cur ? m_cur->weight
                 : AugmentedTreapWeight::makeEmpty<weight_type>();
  }

  /**
   * @brief Weight of all elements with current key
   */
  weight_type weightCurrent() const {
    return m_node
               ? m_node->weight - m_node->weightLeft() - m_node->weightRight()
               : AugmentedTreapWeight::makeEmpty<weight_type>();
  }

  // /**
  //  * @brief Weight of all elements with current key
  //  */
  // weight_type weight() const {
  //   return m_node ? m_node->weight
  //                 : AugmentedTreapWeight::makeEmpty<weight_type>();
  // }

  value_type& operator*() { return m_cur->value; }

  AugmentedTreapMapIterator& operator++()  // prefix ++
  {
    // Check first if we can go forward in the same node
    if (m_cur && m_cur->next) {
      m_cur = m_cur->next;
    } else {
      if (!m_node)  // ++ from end(). get the first node of the tree
      {
        m_node = TreapUtil<node_type>::findFirstNode(m_root);

        // error! ++ requested for an empty tree
        if (!m_node)
          throw std::underflow_error(
              "AugmentedTreapMapIterator operator++ (): tree empty");
      } else if (m_node->right) {
        // successor is the furthest left node of
        // right subtree
        m_node = TreapUtil<node_type>::findFirstNode(m_node->right);
      } else {
        // have already processed the left subtree, and
        // there is no right subtree. move up the tree,
        // looking for a parent for which m_node is a left child,
        // stopping if the parent becomes nullptr. a non-nullptr parent
        // is the successor. if parent is nullptr, the original node
        // was the last node inorder, and its successor
        // is the end of the list
        node_type* p = m_node->parent;

        while (p && m_node == p->right) {
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

  AugmentedTreapMapIterator& operator--()  // prefix --
  {
    // Check first if we can go forward in the same node
    if (m_cur && m_cur->previous) {
      m_cur = m_cur->previous;
    } else {
      if (!m_node) {
        // ++ from end(). get the root of the tree
        m_node = TreapUtil<node_type>::findLastNode(m_root);

        // error! ++ requested for an empty tree
        if (!m_node)
          throw std::underflow_error(
              "AugmentedTreapMapIterator operator-- (): tree empty");
      } else if (m_node->left) {
        // successor is the furthest right node of
        // right subtree
        m_node = TreapUtil<node_type>::findLastNode(m_node->left);
      } else {
        // have already processed the right subtree, and
        // there is no left subtree. move up the tree,
        // looking for a parent for which m_node is a right child,
        // stopping if the parent becomes nullptr. a non-nullptr parent
        // is the successor. if parent is nullptr, the original node
        // was the last node inorder, and its successor
        // is the end of the list
        node_type* p = m_node->parent;

        while (p && m_node == p->left) {
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

  AugmentedTreapMapIterator operator++(int)  // postfix ++
  {
    AugmentedTreapMapIterator result(*this);
    ++(*this);
    return result;
  }

  AugmentedTreapMapIterator operator--(int)  // postfix --
  {
    AugmentedTreapMapIterator result(*this);
    --(*this);
    return result;
  }
};

template <class K, class V, class S>
void AugmentedTreapMap<K, V, S>::Insert(const_key _key, const_reference _value,
                                        const_weight _weight) {
  // Construct the node
  Node* newNode = new Node(_key, _value, _weight);
  TreapUtil<Node>::doInsert(newNode, &m_root);
}

template <class K, class V, class S>
bool AugmentedTreapMap<K, V, S>::Remove(iterator i) {
  if (i == end()) return false;

  TreapUtil<Node>::doRemove(i.m_node, &m_root, i.m_cur);
  return true;
}

template <class K, class V, class S>
bool AugmentedTreapMap<K, V, S>::Remove(const_key _key,
                                        const_reference _value) {
  // Try to find the node
  Node* n = TreapUtil<Node>::findEQ(_key, m_root);

  if (!n) return false;

  typename Node::Node* v = n->find(_value);

  if (!v) return false;

  TreapUtil<Node>::doRemove(n, &m_root, v);
  return true;
}

template <class K, class V, class S>
bool AugmentedTreapMap<K, V, S>::RemoveAll(const_key _key) {
  Node* n = TreapUtil<Node>::findEQ(_key, m_root);

  if (!n) return false;

  TreapUtil<Node>::doRemove(n, &m_root);
  return true;
}

template <class K, class V, class S>
bool AugmentedTreapMap<K, V, S>::Move(const_key _currentKey,
                                      const_reference _value,
                                      const_key _newKey) {
  Node* cur = TreapUtil<Node>::findEQ(_currentKey, m_root);

  if (cur) {
    auto sub = cur->find(_value);

    if (sub) {
      weight w = sub->weight;

      if (Remove(_currentKey, _value)) {
        Insert(_newKey, _value, w);
        return true;
      }
    }
  }

  return false;
}

template <class K, class V, class S>
bool AugmentedTreapMap<K, V, S>::SetWeight(const_key _key,
                                           const_reference _value,
                                           const_weight _weight) {
  Node* n = TreapUtil<Node>::findEQ(_key, m_root);

  if (!n) return false;

  // Find _value in the node
  weight diff = n->weight;

  if (!n->setWeight(_value, _weight)) {
    return false;
  }

  diff -= n->weight;

  if (AugmentedTreapWeight::isEmpty<weight>(diff)) {
    // Modify the weights of the parents
    while (n->parent) {
      n = n->parent;
      n->weight -= diff;
    }
  }

  return true;
}

template <class K, class V, class S>
AugmentedTreapMap<K, V, S>* AugmentedTreapMap<K, V, S>::Split(const_key _key) {
  AugmentedTreapMap<K, V, S>* oth = new AugmentedTreapMap<K, V, S>();
  oth->m_root = TreapUtil<Node>::doSplit(_key, &m_root);
  return oth;
}

/**
 * @brief Merges two treaps
 * @param _other The treap to merge with this treap. MUST have all keys > than
 * all keys in this treap.
 *
 * @return True if successfull, false if other treap keys are smaller or equal
 * to current treap.
 *
 * The other treap WILL BE CLEARED if this function is successfull.
 */
template <class K, class V, class S>
bool AugmentedTreapMap<K, V, S>::Merge(AugmentedTreapMap<K, V, S>& _other) {
  return TreapUtil<Node>::doMerge(&m_root, &(_other.m_root));
}

// First node with value >= w, nullptr if none.
template <class K, class V, class S>
typename AugmentedTreapMap<K, V, S>::iterator AugmentedTreapMap<K, V, S>::find(
    const_key _key, const_reference _val) const {
  auto p = TreapUtil<Node>::findEQ(_key, _val, m_root);

  if (p.first) {
    return iterator(p.first, p.second, m_root);
  } else {
    return end();
  }
}

}  // namespace kangaroo

#endif  // AUGMENTEDTREAPMAP_H

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

#include <assert.h>

#include <utility>

namespace kangaroo {
namespace AugmentedTreapWeight {
template <class W>
bool isEmpty(const W& weight) {
  return weight != 0;
}

template <class W>
W makeEmpty() {
  return W(0);
}

}  // namespace AugmentedTreapWeight

template <class K, class V, class W>
struct AugmentedTreapNode {
  using key_type = K;
  using value_type = V;
  using weight_type = W;

  struct Node {
    Node(const V& _v, const W& _w, Node* _next = nullptr)
        : value(_v), weight(_w), next(_next), previous(nullptr) {
      if (_next) {
        _next->previous = this;
      }
    }

    V value;
    W weight;
    Node* next = nullptr;
    Node* previous = nullptr;
  };

  AugmentedTreapNode(const K& _key)
      : key(_key),
        first(nullptr),
        last(nullptr),
        weight(AugmentedTreapWeight::makeEmpty<W>()),
        count(0),
        parent(nullptr),
        left(nullptr),
        right(nullptr) {}

  AugmentedTreapNode(const K& _key, const V& _value, const W& _weight)
      : key(_key),
        first(new Node(_value, _weight)),
        last(first),
        weight(_weight),
        count(1),
        p(rand()),
        parent(nullptr),
        left(nullptr),
        right(nullptr) {}

  ~AugmentedTreapNode() {
    Node* next = first;
    Node* cur = nullptr;

    while (next) {
      cur = next;
      next = cur->next;
      delete cur;
    }
  }

  bool empty() const { return !first; }

  /**
   * @brief compare
   * @param _other
   * @return -1 if THIS < OTHER, 0 if THIS == OTHER, 1 if THIS > OTHER
   */
  int compare(AugmentedTreapNode& _other) const {
    return key < _other.key ? -1 : (key == _other.key ? 0 : 1);
  }

  weight_type weightLeft() const {
    return left ? left->weight : AugmentedTreapWeight::makeEmpty<W>();
  }

  weight_type weightRight() const {
    return right ? right->weight : AugmentedTreapWeight::makeEmpty<W>();
  }

  Node* find(const V& _value) const {
    Node* cur = first;

    while (cur && cur->value != _value) {
      cur = cur->next;
    }

    return cur;
  }

  bool setWeight(const V& _value, const W& _weight) {
    Node* cur = find(_value);

    if (cur) {
      weight += _weight - cur->weight;
      cur->weight = _weight;
      return true;
    } else {
      return false;
    }
  }

  void add(const V& _value, const W& _weight) {
    first =
        new Node(_value, _weight,
                 first);  // This takes care of the previous and next pointers.
    weight += _weight;

    if (!last) {
      last = first;
    }

    ++count;
  }

  void removeFirst(W& _weightRemoved) {
    Node* temp = first;

    if (temp == last) {
      last = nullptr;
    }

    weight -= temp->weight;
    _weightRemoved = temp->weight;

    first = temp->next;
    first->previous = nullptr;
    delete temp;

    --count;
  }

  bool removeOne(const V& _value, W& _weightRemoved) {
    Node* cur = first;
    Node* prev = nullptr;

    while (cur && cur->value != _value) {
      cur = cur->next;
      prev = cur;
    }

    if (cur) {
      if (cur->next) {
        cur->next->previous = prev;
      } else  // Removing the last one...
      {
        last = prev;
      }

      if (prev) {
        prev->next = cur->next;
      } else  // If cur == first
      {
        first = cur->next;
      }

      _weightRemoved = cur->weight;
      weight -= cur->weight;

      --count;

      delete cur;
      return true;
    } else {
      return false;
    }
  }

  void remove(Node* v) {
    // Pointers
    if (v->previous) {
      v->previous->next = v->next;
    }

    if (v->next) {
      v->next->previous = v->previous;
    }

    if (v == first) {
      first = v->next;
    }

    if (v == last) {
      last = v->previous;
    }

    // Weight
    weight -= v->weight;
    --count;
  }

  K key;
  Node* first;
  Node* last;
  W weight;
  int count;
  int p;

  AugmentedTreapNode<K, V, W>* parent;
  AugmentedTreapNode<K, V, W>* left;
  AugmentedTreapNode<K, V, W>* right;
};

template <class Node>
struct TreapUtil {
  using key_type = typename Node::key_type;
  using value_type = typename Node::value_type;
  using weight_type = typename Node::weight_type;
  using subnode_type = typename Node::Node;

  using const_key = const key_type&;
  using node_sub_pair = std::pair<Node*, subnode_type*>;

  static Node* findFirstNode(Node* root);
  static Node* findLastNode(Node* root);
  static Node* findUpperBound(const_key _key, Node* root);
  static Node* findLowerBound(const_key _key, Node* root);
  static Node* findEQ(const_key _key, Node* root);
  static node_sub_pair findEQ(const_key _key, const value_type& _val,
                              Node* root);

  static Node* doSplit(const_key _key, Node** root);
  static bool doMerge(Node** root, Node** otherRoot);

  static void rotateRight(Node* u, Node** root);
  static void rotateLeft(Node* u, Node** root);

  static void bubbleUp(Node* u, Node** root);
  static void trickleDown(Node* u, Node** root);
  static void splice(Node* u, Node** root);

  static void doInsert(Node* u, Node** root);
  static void doRemove(Node* u, Node** root, subnode_type* v = nullptr);

  static void doClear(Node** root);

  static weight_type leftWeight(const_key _to, bool inclusive, Node* root);
  static weight_type leftWeight(Node* cur, subnode_type* _sub, bool inclusive,
                                Node* root);
  static weight_type rightWeight(const_key _from, bool inclusive, Node* root);

  enum Direction { Left, Right };
};

template <class Node>
Node* TreapUtil<Node>::findFirstNode(Node* root) {
  Node* cur = root;

  while (cur && cur->left) {
    cur = cur->left;
  }

  return cur;
}

template <class Node>
Node* TreapUtil<Node>::findLastNode(Node* root) {
  Node* cur = root;

  while (cur && cur->right) {
    cur = cur->right;
  }

  return cur;
}

// First node with value > w, nullptr if none.
template <class Node>
Node* TreapUtil<Node>::findUpperBound(const_key _key, Node* root) {
  Node* cur = root;
  Node* bestUB = nullptr;

  while (cur) {
    if (_key < cur->key) {
      bestUB = cur;
      cur = cur->left;
    } else  //(_key >= cur->key) do not modify the best successor in this
            // case...
    {
      cur = cur->right;
    }
  }

  return bestUB;
}

// First node with value >= w, nullptr if none.
template <class Node>
Node* TreapUtil<Node>::findLowerBound(const_key _key, Node* root) {
  Node* cur = root;
  Node* bestLB = nullptr;

  while (cur) {
    if (_key < cur->key) {
      bestLB = cur;
      cur = cur->left;
    } else if (_key > cur->key) {
      cur = cur->right;
    } else  // We found it
    {
      return cur;
    }
  }

  return bestLB;
}

template <class Node>  // First node with value >= w, nullptr if none.
Node* TreapUtil<Node>::findEQ(const_key _key, Node* root) {
  Node* cur = root;

  while (cur) {
    if (_key < cur->key) {
      cur = cur->left;
    } else if (_key > cur->key) {
      cur = cur->right;
    } else  // We found it
    {
      return cur;
    }
  }

  return cur;
}

template <class Node>  // First node with value >= w, nullptr if none.
typename TreapUtil<Node>::node_sub_pair TreapUtil<Node>::findEQ(
    const_key _key, const value_type& _val, Node* root) {
  Node* cur = findEQ(_key, root);

  if (cur) {
    auto sub = cur->find(_val);

    if (sub) {
      return node_sub_pair(cur, sub);
    }
  }

  return node_sub_pair(nullptr, nullptr);
}

template <class Node>
void TreapUtil<Node>::doClear(Node** root) {
  Node *u = *root, *prev = nullptr, *next;

  while (u) {
    if (prev == u->parent) {
      if (u->left) {
        next = u->left;
      } else if (u->right) {
        next = u->right;
      } else {
        next = u->parent;
      }

    } else if (prev == u->left) {
      if (u->right) {
        next = u->right;
      } else {
        next = u->parent;
      }
    } else {
      next = u->parent;
    }

    prev = u;

    if (next == u->parent) {
      delete u;
    }

    u = next;
  }

  *root = nullptr;
}

/**
 * Splits in two parts: Treap with keys < than x, and treap with keys >= x.
 */
template <class Node>
Node* TreapUtil<Node>::doSplit(const_key _key, Node** root) {
  Node* u = findLowerBound(_key, *root);
  Node* otherRoot = nullptr;

  if (u) {
    // Change priority to lowest
    u->p = -1;

    // Bubble it up to the top!
    bubbleUp(u, root);

    // Now u must be the root.
    assert(u == *root);

    *root = u->left;
    otherRoot = u->right;

    // Adjust the new roots
    if (*root)  // If not empty!
    {
      (*root)->parent = nullptr;
      u->count -= (*root)->count;
      u->weight -= (*root)->weight;
    }

    if (otherRoot) {
      otherRoot->parent = nullptr;
      u->count -= otherRoot->count;
      u->weight -= otherRoot->weight;
    }

    // Cut off u
    u->parent = u->left = u->right = nullptr;

    // Get back standard priority for u
    u->p = rand();

    // Insert it back in the new treap
    doInsert(u, &otherRoot);
  }
  // Else, no value at least x, new treap is empty, so we return nullptr.

  return otherRoot;
}

template <class Node>
bool TreapUtil<Node>::doMerge(Node** root, Node** otherRoot) {
  if (!*otherRoot) {
    return true;
  }

  const_key x = findFirstNode(*otherRoot)->key;

  if (!*root) {
    *root = *otherRoot;
    *otherRoot = nullptr;
    return true;
  } else if (findLastNode(*root)->key >= x) {
    return false;
  }

  // Create new root with any value, put two treaps as subchilds
  Node* newRoot = new Node(x);
  newRoot->p = -1;
  newRoot->left = *root;
  newRoot->right = *otherRoot;
  newRoot->left->parent = newRoot;
  newRoot->right->parent = newRoot;
  newRoot->weight = newRoot->left->weight + newRoot->right->weight;
  newRoot->count = newRoot->left->count + newRoot->right->count + 1;

  // New roots, clear other treap
  *root = newRoot;
  *otherRoot = nullptr;

  // Trickle down this new root, splice it and delete it.
  doRemove(newRoot, root);

  return true;
}

template <class Node>
void TreapUtil<Node>::splice(Node* u, Node** root) {
  assert(u != nullptr);

  Node *s, *p;

  if (u->left) {
    s = u->left;
  } else {
    s = u->right;
  }

  if (u == *root) {
    *root = s;
    p = nullptr;
  } else {
    p = u->parent;

    if (p->left == u) {
      p->left = s;
    } else {
      p->right = s;
    }
  }

  if (s) {
    s->parent = p;
  }
}

template <class Node>
void TreapUtil<Node>::rotateRight(Node* u, Node** root) {
  assert(u != nullptr);

  Node* w = u->left;
  w->parent = u->parent;

  auto newSumU = u->weight - w->weight +
                 (w->right ? w->right->weight
                           : AugmentedTreapWeight::makeEmpty<weight_type>());

  auto newSumW = w->weight -
                 (w->right ? w->right->weight
                           : AugmentedTreapWeight::makeEmpty<weight_type>()) +
                 newSumU;

  if (w->parent) {
    if (w->parent->left == u) {
      w->parent->left = w;
    } else {
      w->parent->right = w;
    }
  }
  u->left = w->right;

  if (u->left) {
    u->left->parent = u;
  }

  u->parent = w;
  w->right = u;

  u->weight = newSumU;
  w->weight = newSumW;

  u->count = u->count - w->count + (u->left ? u->left->count : 0);
  w->count = w->count - (u->left ? u->left->count : 0) + u->count;

  if (u == *root) {
    *root = w;
    (*root)->parent = nullptr;
  }
}

template <class Node>
void TreapUtil<Node>::rotateLeft(Node* u, Node** root) {
  assert(u != nullptr);
  assert(*root != nullptr);

  Node* w = u->right;
  w->parent = u->parent;

  auto newSumU = u->weight - w->weight +
                 (w->left ? w->left->weight
                          : AugmentedTreapWeight::makeEmpty<weight_type>());

  auto newSumW = w->weight -
                 (w->left ? w->left->weight
                          : AugmentedTreapWeight::makeEmpty<weight_type>()) +
                 newSumU;

  if (w->parent) {
    if (w->parent->left == u) {
      w->parent->left = w;
    } else {
      w->parent->right = w;
    }
  }
  u->right = w->left;

  if (u->right) {
    u->right->parent = u;
  }

  u->parent = w;
  w->left = u;

  u->weight = newSumU;
  w->weight = newSumW;

  u->count = u->count - w->count + (u->right ? u->right->count : 0);
  w->count = w->count - (u->right ? u->right->count : 0) + u->count;

  if (u == *root) {
    *root = w;
    (*root)->parent = nullptr;
  }
}

template <class Node>
void TreapUtil<Node>::doInsert(Node* u, Node** root) {
  assert(u != nullptr);

  if (!*root) {
    *root = u;
  } else {
    Node* cur = *root;
    Node* newParent = nullptr;

    //================= HELPER FUNCTIONS =================

    auto attachNode = [u](Node* _parent, Direction _dir) {
      if (_dir == Left) {
        _parent->left = u;
      } else {
        _parent->right = u;
      }

      u->parent = _parent;
    };

    auto moveDown = [u](Node* _parent, Direction _dir) {
      _parent->weight += u->weight;
      ++_parent->count;

      return _dir == Left ? _parent->left : _parent->right;
    };

    //================= WALK DOWN TREE =================

    while (cur) {
      newParent = cur;

      int comp = u->compare(*cur);

      if (comp < 0) {
        cur = moveDown(cur, Left);
      } else if (comp > 0) {
        cur = moveDown(cur, Right);
      } else {
        break;
      }
    }

    // Add it as a child
    int comp = u->compare(*newParent);

    if (comp < 0) {
      attachNode(newParent, Left);
    } else if (comp > 0) {
      attachNode(newParent, Right);
    } else  // Already exists
    {
      auto n = u->first;

      while (n) {
        newParent->add(n->value, n->weight);
        n = n->next;
      }

      delete u;
      u = nullptr;
    }

    //================= CORRECT THE P's =================
    if (u) {
      bubbleUp(u, root);
    }
  }
}

template <class Node>
void TreapUtil<Node>::doRemove(Node* u, Node** root, subnode_type* v) {
  assert(u != nullptr);

  bool removeFull = !v || (!v->previous && !v->next);
  weight_type w;
  Node* p;

  if (!removeFull) {
    w = v->weight;
    u->remove(v);
    p = u->parent;
  } else {
    trickleDown(u, root);
    w = u->weight - u->weightLeft() - u->weightRight();
    p = u->parent;
    splice(u, root);
    delete u;
  }

  while (p) {
    --p->count;
    p->weight -= w;
    p = p->parent;
  }
}

template <class Node>
void TreapUtil<Node>::bubbleUp(Node* u, Node** root) {
  assert(u != nullptr);

  while (u->parent && u->parent->p > u->p) {
    if (u->parent->right == u) {
      rotateLeft(u->parent, root);
    } else {
      rotateRight(u->parent, root);
    }
  }

  if (!u->parent) {
    *root = u;
  }
}

template <class Node>
void TreapUtil<Node>::trickleDown(Node* u, Node** root) {
  assert(u != nullptr);

  while (u->left || u->right) {
    if (!u->left) {
      rotateLeft(u, root);
    } else if (!u->right) {
      rotateRight(u, root);
    } else if (u->left->p < u->right->p) {
      rotateRight(u, root);
    } else {
      rotateLeft(u, root);
    }

    if (*root == u) {
      *root = u->parent;
    }
  }
}

template <class Node>
typename TreapUtil<Node>::weight_type TreapUtil<Node>::leftWeight(
    const_key _to, bool inclusive, Node* root) {
  Node* cur = root;
  weight_type weight = AugmentedTreapWeight::makeEmpty<weight_type>();

  while (cur) {
    if (_to < cur->key) {
      cur = cur->left;
    } else if (inclusive)  // (_cur->key <= _to)
    {
      weight += cur->weight - cur->weightRight();

      if (cur->key == _to) {
        break;
      }
      cur = cur->right;

    } else if (cur->key < _to)  // Not inclusive
    {
      weight += cur->weight - cur->weightRight();
      cur = cur->right;
    } else  // Found the node, Not inclusive
    {
      weight += cur->weightLeft();
      break;
    }
  }

  return weight;
}

template <class Node>
typename TreapUtil<Node>::weight_type TreapUtil<Node>::leftWeight(
    Node* cur, subnode_type* sub, bool inclusive, Node* root) {
  if (!root) return AugmentedTreapWeight::makeEmpty<weight_type>();

  if (!cur || !sub) return root->weight;

  weight_type weight = AugmentedTreapWeight::makeEmpty<weight_type>();

  // Compute the weight until the element pointed by the iterator
  auto n = cur->first;

  while (n && (n != sub || inclusive)) {
    weight += n->weight;
    n = n->next;

    if (n == sub) break;
  }

  // Compute the weight until the root
  weight += cur->weightLeft();

  while (cur->parent) {
    Node* prev = cur;
    cur = cur->parent;

    if (prev == cur->right) {
      weight += cur->weight - cur->weightRight();
    }
  }

  return weight;
}

template <class Node>
typename TreapUtil<Node>::weight_type TreapUtil<Node>::rightWeight(
    const_key _from, bool inclusive, Node* root) {
  Node* cur = root;
  weight_type weight = AugmentedTreapWeight::makeEmpty<weight_type>();

  while (cur) {
    if (_from > cur->key) {
      cur = cur->right;
    } else if (inclusive)  // (_cur->key <= _key)
    {
      weight += cur->weight - cur->weightLeft();

      if (cur->key == _from) {
        break;
      }
      cur = cur->left;

    } else if (_from < cur->key)  // Not inclusive
    {
      weight += cur->weight - cur->weightLeft();
      cur = cur->left;
    } else  // Found the node, Not inclusive
    {
      weight += cur->weightRight();
      break;
    }
  }

  return weight;
}

}  // namespace kangaroo

#endif  // TREAPUTIL_H

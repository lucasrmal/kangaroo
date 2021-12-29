#ifndef UTIL_INDEXED_VECTOR_H
#define UTIL_INDEXED_VECTOR_H

#include <memory>
#include <unordered_map>
#include <vector>

namespace kangaroo {

template <class T>
class IndexedVector {
 public:
  using ConstIterator =
      typename std::unordered_map<int64_t, std::unique_ptr<T>>::const_iterator;

  IndexedVector() {}

  int64_t Insert(std::unique_ptr<T> element);
  bool Remove(int64_t id);
  void Load(std::vector<std::unique_ptr<T>>* elements);
  void Clear();

  T* Get(int64_t id);
  const T* Get(int64_t id) const;

  ConstIterator iterator() const { return elements_.begin(); }
  ConstIterator iterator_end() const { return elements_.end(); }
  int64_t size() const { return elements_.size(); }
  int64_t next_id() const { return next_id_; }

 private:
  int64_t next_id_ = 0;
  std::unordered_map<int64_t, std::unique_ptr<T>> elements_;
};

template <class T>
int64_t IndexedVector<T>::Insert(std::unique_ptr<T> element) {
  T* temp = element.get();
  elements_.insert({next_id_, std::move(element)});
  temp->set_id(next_id_++);
  return temp->id();
}

template <class T>
T* IndexedVector<T>::Get(int64_t id) {
  auto it = elements_.find(id);
  return it == elements_.end() ? nullptr : it->second.get();
}

template <class T>
const T* IndexedVector<T>::Get(int64_t id) const {
  auto it = elements_.find(id);
  return it == elements_.end() ? nullptr : it->second.get();
}

template <class T>
bool IndexedVector<T>::Remove(int64_t id) {
  auto removed = elements_.erase(id);
  return removed > 0;
}

template <class T>
void IndexedVector<T>::Load(std::vector<std::unique_ptr<T>>* elements) {
  Clear();

  for (auto& element : *elements) {
    next_id_ = std::max(next_id_, element->id() + 1);
    elements_.insert({element->id(), std::move(element)});
  }
}

template <class T>
void IndexedVector<T>::Clear() {
  next_id_ = 0;
  elements_.clear();
}

}  // namespace kangaroo

#endif  // UTIL_INDEXED_VECTOR_H

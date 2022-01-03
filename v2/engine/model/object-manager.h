#ifndef MODEL_OBJECT_MANAGER_H
#define MODEL_OBJECT_MANAGER_H

#include <memory>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "util/indexed-vector.h"
#include "util/status-util.h"

namespace kangaroo::model {

template <class Object>
class ObjectIndexInterface {
 public:
  virtual ~ObjectIndexInterface() = default;
  virtual void AddToIndex(Object* o) = 0;
  virtual void UpdateIndex(Object* o, const Object& updated) = 0;
  virtual void RemoveFromIndex(Object* o) = 0;
};

template <class Object>
class ObjectValidatorInterface {
 public:
  // If insert, `before` will be nullprt.
  virtual absl::Status Validate(const Object* before,
                                const Object& after) const = 0;
};

template <class Object>
class ObjectManager {
 public:
  using ConstIterator = typename IndexedVector<Object>::ConstIterator;
  using ObjectIndexT = ObjectIndexInterface<Object>;
  using ObjectValidatorT = ObjectValidatorInterface<Object>;

  virtual ~ObjectManager() = default;

  absl::StatusOr<int64_t> Insert(std::unique_ptr<Object> inserted);
  absl::Status Update(const Object& updated);
  absl::Status Remove(int64_t id);

  const Object* Get(int64_t id) const { return objects_.Get(id); }

  int64_t size() const { return objects_.size(); }

  typename std::pair<ConstIterator, ConstIterator> Objects() const {
    return {objects_.iterator(), objects_.iterator_end()};
  }

 protected:
  virtual absl::Status ValidateInsert(const Object& inserted) const {
    return absl::OkStatus();
  }
  virtual absl::Status ValidateUpdate(const Object& existing,
                                      const Object& updated) const {
    return absl::OkStatus();
  }
  virtual absl::Status ValidateRemove(const Object&) const {
    return absl::OkStatus();
  }

  virtual void PostInsert(const Object& inserted) const {}
  virtual void PreUpdate(const Object& existing, const Object& updated) const {}
  virtual void PreRemove(const Object& removed) const {}

  virtual std::vector<ObjectIndexT*> Indexes() = 0;
  virtual std::vector<const ObjectValidatorT*> Validators() const = 0;

  IndexedVector<Object> objects_;

 private:
  virtual absl::Status ValidateInsertSuper(const Object& inserted) const {
    for (const auto* validator : Validators()) {
      RETURN_IF_ERROR(validator->Validate(nullptr, inserted));
    }
    return ValidateInsert(inserted);
  }
  virtual absl::Status ValidateUpdateSuper(const Object& existing,
                                           const Object& updated) const {
    for (const auto* validator : Validators()) {
      RETURN_IF_ERROR(validator->Validate(&existing, updated));
    }
    return ValidateUpdate(existing, updated);
  }
};

// Usage ex: ObjectIndex<Institution, &Institution::name> my_index;
template <class Object, auto Getter>
class ObjectIndex : public ObjectIndexInterface<Object> {
 public:
  using KeyType = typename std::decay<
      typename std::result_of<decltype(Getter)(Object)>::type>::type;

  ~ObjectIndex() override = default;

  void AddToIndex(Object* o) override;
  void UpdateIndex(Object* o, const Object& updated) override;
  void RemoveFromIndex(Object* o) override;

  std::vector<const Object*> FindAll(const KeyType& key) const;
  Object* FindOne(const KeyType& key) const;
  bool Contains(const KeyType& key) const;

 private:
  std::unordered_multimap<KeyType, Object*> index_;
};

// ObjectManager Implementation

template <class Object>
absl::StatusOr<int64_t> ObjectManager<Object>::Insert(
    std::unique_ptr<Object> inserted) {
  RETURN_IF_ERROR(ValidateInsertSuper(*inserted));
  for (auto* index : Indexes()) index->AddToIndex(inserted.get());
  Object* object = inserted.get();
  int64_t id = objects_.Insert(std::move(inserted));
  PostInsert(*object);
  return id;
}

template <class Object>
absl::Status ObjectManager<Object>::Update(const Object& updated) {
  Object* existing = objects_.Get(updated.id());
  if (!existing) {
    return absl::NotFoundError(
        absl::StrCat("Element with id ", updated.id(), " does not exist."));
  }
  RETURN_IF_ERROR(ValidateUpdateSuper(*existing, updated));
  for (auto* index : Indexes()) index->UpdateIndex(existing, updated);
  PreUpdate(*existing, updated);
  *existing = updated;
  return absl::OkStatus();
}

template <class Object>
absl::Status ObjectManager<Object>::Remove(const int64_t id) {
  Object* stored = objects_.Get(id);
  if (!stored) {
    return absl::NotFoundError(
        absl::StrCat("Element with id ", id, " does not exist."));
  }
  RETURN_IF_ERROR(ValidateRemove(*stored));
  for (auto* index : Indexes()) index->RemoveFromIndex(stored);
  PreRemove(stored);
  objects_.Remove(id);
  return absl::OkStatus();
}

// ObjectIndex Implementation

template <class Object, auto Getter>
void ObjectIndex<Object, Getter>::AddToIndex(Object* o) {
  index_.insert({(o->*Getter)(), o});
}
template <class Object, auto Getter>
void ObjectIndex<Object, Getter>::UpdateIndex(Object* o,
                                              const Object& updated) {
  if ((o->*Getter)() != (updated.*Getter)()) {
    RemoveFromIndex(o);
    index_.insert({(updated.*Getter)(), o});
  }
}
template <class Object, auto Getter>
void ObjectIndex<Object, Getter>::RemoveFromIndex(Object* o) {
  auto range = index_.equal_range((o->*Getter)());
  for (auto it = range.first; it != range.second; ++it) {
    if (it->second == o) {
      index_.erase(it);
      break;
    }
  }
}

template <class Object, auto Getter>
std::vector<const Object*> ObjectIndex<Object, Getter>::FindAll(
    const KeyType& key) const {
  std::vector<const Object*> objects;
  auto range = index_.equal_range(key);
  for (auto it = range.first; it != range.second; ++it) {
    objects.push_back(it->second);
  }
  return objects;
}

template <class Object, auto Getter>
Object* ObjectIndex<Object, Getter>::FindOne(const KeyType& key) const {
  auto it = index_.find(key);
  return it == index_.end() ? nullptr : it->second;
}

template <class Object, auto Getter>
bool ObjectIndex<Object, Getter>::Contains(const KeyType& key) const {
  return index_.find(key) != index_.end();
}

}  //  namespace kangaroo::model

#endif  // MODEL_OBJECT_MANAGER_H
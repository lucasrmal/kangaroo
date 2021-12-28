#ifndef MODEL_VALIDATORS_H
#define MODEL_VALIDATORS_H

#include <string>
#include <utility>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "model/object-manager.h"

namespace kangaroo::model {

template <class Object, auto Getter>
class RequiredValidator : public ObjectValidatorInterface<Object> {
 public:
  using PropertyType = typename std::decay<
      typename std::result_of<decltype(Getter)(Object)>::type>::type;

  RequiredValidator(const std::string& property_name)
      : property_name_(property_name) {}

  absl::Status Validate(const Object*, const Object& after) const override {
    if ((after.*Getter)().empty()) {
      return absl::InvalidArgumentError(
          absl::StrCat(property_name_, " is a required field."));
    }
    return absl::OkStatus();
  }

 private:
  const std::string property_name_;
};

template <class Object, auto Getter>
class UniqueValidator : public ObjectValidatorInterface<Object> {
 public:
  using PropertyType = typename std::decay<
      typename std::result_of<decltype(Getter)(Object)>::type>::type;
  using PropertyIndex = ObjectIndex<Object, Getter>;

  UniqueValidator(const std::string& property_name, const PropertyIndex* index)
      : property_name_(property_name), index_(index) {}

  absl::Status Validate(const Object* before,
                        const Object& after) const override {
    if (const Object* o = index_->FindOne((after.*Getter)());
        o != nullptr && o != before) {
      return absl::InvalidArgumentError(
          absl::StrCat("An object with ", property_name_, "=",
                       (after.*Getter)(), " already exists."));
    }
    return absl::OkStatus();
  }

 private:
  const std::string property_name_;
  const PropertyIndex* index_;
};

template <class Object>
class IconIdValidator : public ObjectValidatorInterface<Object> {
  absl::Status Validate(const Object* before,
                        const Object& after) const override {
    return absl::UnimplementedError("Not Yet Implemented!");
  }
};
}  // namespace kangaroo::model

#endif  // MODEL_VALIDATORS_H
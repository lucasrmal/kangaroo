#ifndef MODEL_VALIDATOR_HELPERS_H
#define MODEL_VALIDATOR_HELPERS_H

#include "model/validators.h"

namespace kangaroo::model {

class IconManager;
template <class Object>
class IconIdValidator
    : public OptionalIdValidator<Object, &Object::icon_id, &Object::has_icon_id,
                                 IconManager> {
 public:
  IconIdValidator(IconManager* manager)
      : OptionalIdValidator<Object, &Object::icon_id, &Object::has_icon_id,
                            IconManager>("icon_id", manager) {}
};

class InstitutionManager;
template <class Object>
class InstitutionIdValidator
    : public OptionalIdValidator<Object, &Object::institution_id,
                                 &Object::has_institution_id,
                                 InstitutionManager> {
 public:
  InstitutionIdValidator(InstitutionManager* manager)
      : OptionalIdValidator<Object, &Object::institution_id,
                            &Object::has_institution_id, InstitutionManager>(
            "institution_id", manager) {}
};

class PayeeManager;
template <class Object>
class PayeeIdValidator
    : public OptionalIdValidator<Object, &Object::payee_id,
                                 &Object::has_payee_id, PayeeManager> {
 public:
  PayeeIdValidator(PayeeManager* manager)
      : OptionalIdValidator<Object, &Object::payee_id, &Object::has_payee_id,
                            PayeeManager>("payee_id", manager) {}
};

}  // namespace kangaroo::model

#endif  // MODEL_VALIDATOR_HELPERS_H
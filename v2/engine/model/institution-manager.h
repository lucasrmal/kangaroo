/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  US
 */

#ifndef MODEL_INSTITUTION_MANAGER_H
#define MODEL_INSTITUTION_MANAGER_H

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "model/object-manager.h"
#include "model/proto/institution.pb.h"
#include "model/validators.h"

namespace kangaroo::model {

class InstitutionManager : public ObjectManager<Institution> {
 public:
  std::vector<std::string> CountriesInUse() const;
  /**
   * @brief merge Merge a set of institutions into a single one
   * @param _ids The IDs of institutions to merge together
   * @param _idTo The destination institution id. Must be included in _ids.
   */
  absl::Status Merge(const std::vector<int64_t>& from, int64_t to);

  static InstitutionManager* instance() { return instance_; }

 protected:
  absl::Status ValidateRemove(const Institution& inst) const override;

  std::vector<ObjectIndexT*> Indexes() override { return {}; }
  std::vector<const ObjectValidatorT*> Validators() const override {
    return {&required_name_validator_, &icon_id_validator_};
  }

 private:
  RequiredValidator<Institution, &Institution::name> required_name_validator_ =
      {"name"};
  IconIdValidator<Institution> icon_id_validator_;
  static InstitutionManager* instance_;
};

}  // namespace kangaroo::model

#endif  // MODEL_INSTITUTION_MANAGER_H

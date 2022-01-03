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

#ifndef MODEL_PAYEE_MANAGER_H
#define MODEL_PAYEE_MANAGER_H

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "model/icon-manager.h"
#include "model/object-manager.h"
#include "model/proto/payee.pb.h"
#include "model/proto/transaction.pb.h"
#include "model/validator-helpers.h"
#include "model/validators.h"

namespace kangaroo::model {

class PayeeManager : public ObjectManager<Payee> {
 public:
  explicit PayeeManager(IconManager* icon_manager)
      : icon_id_validator_(icon_manager) {}

  std::vector<const Transaction*> TransactionsForPayee(int64_t payee_id) const;

  std::vector<std::string> CountriesInUse() const;
  const Payee* FindByName(const std::string& name) const;
  /**
   * @brief merge Merge a set of payees into a single one
   * @param _ids The IDs of payees to merge together
   * @param _idTo The destination payee id. Must be included in _ids.
   */
  absl::Status Merge(const std::vector<int64_t>& from, int64_t to);

 protected:
  absl::Status ValidateRemove(const Payee& payee) const override;

  std::vector<ObjectIndexT*> Indexes() override { return {&name_index_}; }
  std::vector<const ObjectValidatorT*> Validators() const override {
    return {&required_name_validator_, &unique_name_validator_,
            &icon_id_validator_};
  }

 private:
  ObjectIndex<Payee, &Payee::name> name_index_;

  RequiredValidator<Payee, &Payee::name> required_name_validator_ = {"name"};
  UniqueValidator<Payee, &Payee::name> unique_name_validator_ = {"name",
                                                                 &name_index_};
  IconIdValidator<Payee> icon_id_validator_;
};

}  // namespace kangaroo::model

#endif  // MODEL_PAYEE_MANAGER_H

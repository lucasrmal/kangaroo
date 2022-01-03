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

#ifndef MODEL_ACCOUNT_MANAGER_H
#define MODEL_ACCOUNT_MANAGER_H

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "model/commodity-manager.h"
#include "model/icon-manager.h"
#include "model/institution-manager.h"
#include "model/ledger.h"
#include "model/object-manager.h"
#include "model/proto/account.pb.h"
#include "model/validator-helpers.h"
#include "model/validators.h"

namespace kangaroo::model {

namespace account_helper {
bool TypeCanBeChild(Account::Type child, Account::Type of);
}  // namespace account_helper

class AccountManager : public ObjectManager<Account> {
 public:
  explicit AccountManager(CommodityManager* commodity_manager,
                          IconManager* icon_manager,
                          InstitutionManager* institution_manager);

  const Account* Root() const { return root_; }

  std::vector<const Account*> GetChildren(const Account& account) const;
  bool IsAncestorOf(int64_t account_id, int64_t of) const;
  bool IsDescendentOf(int64_t account_id, int64_t of) const;
  absl::Status CanBeArchived(const Account& account) const;

  // Ex: "Assets:Banking:Main Checking";
  // `to` is the account the path ends with. `length` is the max length
  // of the path. -1 for no limit (up to root). Root is never added to the path.
  std::string ColonSeparatedPath(const Account& to, int length = -1) const;

 protected:
  absl::Status ValidateInsert(const Account& account) const override;
  absl::Status ValidateUpdate(const Account& existing,
                              const Account& updated) const override;
  absl::Status ValidateRemove(const Account& account) const override;
  std::vector<ObjectIndexT*> Indexes() override { return {&parent_id_index_}; }
  std::vector<const ObjectValidatorT*> Validators() const override {
    return {&required_type_validator_, &parent_id_validator_,
            &icon_id_validator_, &institution_id_validator_};
  }

 private:
  absl::Status ValidateCommon(const Account& account) const;

  CommodityManager* commodity_manager_;
  IconManager* icon_manager_;
  InstitutionManager* institution_manager_;
  Account* root_ = nullptr;

  BoolPropertyValidator<Account, &Account::has_type> required_type_validator_ =
      {"type"};
  RequiredIdValidator<Account, &Account::parent_id, &Account::has_parent_id,
                      AccountManager>
      parent_id_validator_;
  IconIdValidator<Account> icon_id_validator_;
  InstitutionIdValidator<Account> institution_id_validator_;
  ObjectIndex<Account, &Account::parent_id> parent_id_index_;
};

}  // namespace kangaroo::model

#endif  // MODEL_ACCOUNT_MANAGER_H

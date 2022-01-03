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

#include "model/account-manager.h"

#include "absl/strings/str_cat.h"

namespace kangaroo::model {

AccountManager::AccountManager(CommodityManager* commodity_manager,
                               IconManager* icon_manager,
                               InstitutionManager* institution_manager)
    : commodity_manager_(commodity_manager),
      icon_manager_(icon_manager),
      institution_manager_(institution_manager),
      parent_id_validator_("parent_id", this),
      icon_id_validator_(icon_manager),
      institution_id_validator_(institution_manager),
      ledger_manager_(ledger_manager) {
  // Add the Top Level account
  auto root = std::make_unique<Account>();
  root_ = root.get();
  root->set_name("root");
  root->set_type(Account::ROOT);
  objects_.Insert(std::move(root));
}

std::vector<const Account*> AccountManager::GetChildren(
    const Account& account) const {
  return parent_id_index_.FindAll(account.id());
}

bool AccountManager::IsAncestorOf(int64_t account_id, int64_t of) const {
  if (account_id == of) {
    return false;
  }
  const Account* of_account = Get(of);
  while (of_account != nullptr) {
    if (of_account->id() == account_id) {
      return true;
    } else if (of_account->type() == Account::ROOT) {
      break;
    }
    of_account = Get(of_account->parent_id());
  }
  return false;
}
bool AccountManager::IsDescendentOf(int64_t account_id, int64_t of) const {
  return IsAncestorOf(of, account_id);
}

std::string AccountManager::ColonSeparatedPath(const Account& to,
                                               int length) const {
  const Account* current = &to;
  std::string path;
  while (current && current->id() != root_->id() &&
         (length < 0 || length > 0)) {
    path = path.empty() ? current->name()
                        : absl::StrCat(current->name(), ":", path);
    current = Get(current->parent_id());
    --length;
  }
  return path;
}

absl::Status AccountManager::ValidateCommon(const Account& account) const {
  const Account* parent = Get(account.parent_id());
  if (!account_helper::TypeCanBeChild(account.type(), /*of=*/parent->type())) {
    return absl::InvalidArgumentError(
        absl::StrCat("Account with type", Account::Type_Name(account.type()),
                     " cannot be a child of account with child ",
                     Account::Type_Name(parent->type())));
  }
  const Commodity* commodity = commodity_manager_->Get(account.commodity_id());

  if (!commodity) {
    return absl::InvalidArgumentError("Commodity must exist.");
  }
  switch (account.type()) {
    case Account::TRADING:
      return absl::InvalidArgumentError(
          "Trading accounts may not be added manually.");
    case Account::INVESTMENT:
      if (!commodity->has_security()) {
        return absl::InvalidArgumentError(
            "Investment account commodity must be a Security..");
      }

    default:
      if (!commodity->has_currency()) {
        return absl::InvalidArgumentError(
            "Account commodity must be a Currency.");
      }
  }
  return absl::OkStatus();
}

absl::Status AccountManager::ValidateInsert(const Account& account) const {
  if (account.is_archived()) {
    return absl::InvalidArgumentError(
        "Newly created accounts may not be archived.");
  }
  return ValidateCommon(account);
}

absl::Status AccountManager::ValidateUpdate(const Account& existing,
                                            const Account& updated) const {
  if (existing.id() == root_->id()) {
    return absl::InvalidArgumentError("Root account may not be edited.");
  }
  if (!existing.is_archived() && updated.is_archived()) {
    RETURN_IF_ERROR(CanBeArchived(existing));
  }
  if (existing.type() != updated.type()) {
    if ((existing.type() == Account::INVESTMENT ||
         updated.type() == Account::INVESTMENT) /*&&
        LedgerHasTransactions(existing.id())*/) {
      return absl::InvalidArgumentError(
          "Accounts with transactions may not be changed from/to "
          "InvestmentAccount type.");
    }
  }
  if (!existing.is_placeholder() && updated.is_placeholder()) {
    return absl::UnimplementedError("Not Yet Implemented!");
  }
  if (!existing.commodity_id() && updated.commodity_id()) {
    return absl::UnimplementedError("Not Yet Implemented!");
  }
  return ValidateCommon(updated);
}

absl::Status AccountManager::ValidateRemove(const Account& account) const {
  return absl::UnimplementedError("Not Yet Implemented!");
}

}  // namespace kangaroo::model

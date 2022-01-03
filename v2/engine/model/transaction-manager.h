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

#ifndef MODEL_TRANSACTION_MANAGER_H
#define MODEL_TRANSACTION_MANAGER_H

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "model/account-manager.h"
#include "model/commodity-manager.h"
#include "model/ledger.h"
#include "model/object-manager.h"
#include "model/payee-manager.h"
#include "model/proto/transaction.pb.h"
#include "model/validator-helpers.h"

namespace kangaroo::model {

namespace transaction_helper {
absl::Status SplitsBalance(const Transaction& transaction);

int64_t TotalForAccount(int64_t account_id, const Transaction& transaction);

bool AccountInTransaction(int64_t account_id, const Transaction& transaction);

std::vector<int64_t> AccountsInTransaction(const Transaction& transaction);
}  // namespace transaction_helper

class TransactionManager : public ObjectManager<Transaction> {
 public:
  TransactionManager(AccountManager* account_manager,
                     CommodityManager* commodity_manager,
                     LedgerManager* ledger_manager, PayeeManager* payee_manager)
      : account_manager_(account_manager),
        commodity_manager_(commodity_manager),
        ledger_manager_(ledger_manager),
        payee_id_validator_(payee_manager) {}

  static constexpr int kMaxSplits = 100;

 protected:
  void PostInsert(const Transaction& inserted) const override;
  void PreUpdate(const Transaction& existing,
                 const Transaction& updated) const override;
  void PreRemove(const Transaction& removed) const override;

  absl::Status ValidateInsert(const Transaction& transaction) const override;
  absl::Status ValidateUpdate(const Transaction& existing,
                              const Transaction& updated) const override {
    return ValidateInsert(updated);
  }
  std::vector<ObjectIndexT*> Indexes() override { return {}; }
  std::vector<const ObjectValidatorT*> Validators() const override {
    return {&payee_id_validator_};
  }

  AccountManager* account_manager_;
  CommodityManager* commodity_manager_;
  LedgerManager* ledger_manager_;
  BoolPropertyValidator<Transaction, &Transaction::has_date>
      has_date_validator_ = {"date"};
  PayeeIdValidator<Transaction> payee_id_validator_;
};

}  // namespace kangaroo::model

#endif  // MODEL_TRANSACTION_MANAGER_H

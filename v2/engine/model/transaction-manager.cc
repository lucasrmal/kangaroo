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

#include "model/transaction-manager.h"

#include "absl/strings/str_cat.h"
#include "model/types/date.h"

namespace kangaroo::model {

absl::Status SplitsBalance(const Transaction& transaction) {
  // This may not look like the most effient algo, but the vast majority of
  // transactions have a single currency, some have 2 (investment transactions).
  // 3 commodity transactions are exceedling rare and we are optimizing for the
  // common case. Split size is also limited to kMaxSplits.
  for (int i = 0; i < transaction.split_size(); ++i) {
    int64_t amount = transaction.split(i).amount_micros();
    int64_t commodity_id = transaction.split(i).commodity_id();

    for (int j = i + 1; j < transaction.split_size(); ++j) {
      if (transaction.split(j).commodity_id() == commodity_id) {
        amount += transaction.split(j).amount_micros();
        if (j == i + 1) ++i;
      }
    }

    if (amount != 0)
      return absl::InvalidArgumentError(absl::StrCat(
          "Splits with commodity_id=", transaction.split(i).commodity_id(),
          " do not balance."));
  }

  return absl::OkStatus();
}

int64_t TotalForAccount(int64_t account_id, const Transaction& transaction) {
  int64_t amount_micros = 0;

  for (const Split& s : transaction.split()) {
    if (s.account_id() == account_id) amount_micros += s.amount_micros();
  }
  return amount_micros;
}

bool AccountInTransaction(int64_t account_id, const Transaction& transaction) {
  for (const Split& s : transaction.split()) {
    if (s.account_id() == account_id) return true;
  }
  return false;
}

std::vector<int64_t> AccountsInTransaction(const Transaction& transaction) {
  std::vector<int64_t> accounts;

  for (const Split& s : transaction.split()) {
    for (int64_t a : accounts) {
      if (s.account_id() == a) continue;
      accounts.push_back(s.account_id());
    }
  }
  return accounts;
}

void TransactionManager::PostInsert(const Transaction& inserted) const {
  ledger_manager_->InsertTransaction(inserted);
}

void TransactionManager::PreUpdate(const Transaction& existing,
                                   const Transaction& updated) const {
  ledger_manager_->UpdateTransaction(existing, updated);
}

void TransactionManager::PreRemove(const Transaction& removed) const {
  ledger_manager_->RemoveTransaction(removed);
}

absl::Status TransactionManager::ValidateInsert(
    const Transaction& transaction) const {
  if (date::Normalize(transaction.date()) != transaction.date()) {
    return absl::InvalidArgumentError("Invalid transaction date.");
  }
  if (!Transaction::ClearedStatus_IsValid(transaction.cleared_status())) {
    return absl::InvalidArgumentError("Invalid cleared status.");
  }
  if (transaction.split_size() == 0) {
    return absl::InvalidArgumentError("Transaction may not be empty.");
  }
  if (transaction.split_size() > kMaxSplits) {
    return absl::InvalidArgumentError(absl::StrCat(
        "Transaction may contain more than ", kMaxSplits, " splits."));
  }
  std::set<int64_t> accounts;
  for (const Split& s : transaction.split()) {
    const Account* split_account = account_manager_->Get(s.account_id());
    if (!split_account) {
      return absl::NotFoundError(absl::StrCat(
          "Split account with id=", s.account_id(), " does not exist."));
    }
    auto [it, inserted] = accounts.insert(s.account_id());
    if (!inserted) {
      return absl::InvalidArgumentError(
          "Accounts may not be repeated for a single transaction.");
    }
    if (!s.has_commodity_id()) {
      return absl::InvalidArgumentError("All splits must specify a commodity.");
    }
    if (split_account->commodity_id() != s.commodity_id()) {
      return absl::InvalidArgumentError(
          absl::StrCat("Split commodity id=", s.commodity_id(),
                       " not valid for account=", split_account->id()));
    }
  }
  RETURN_IF_ERROR(SplitsBalance(transaction));

  return absl::OkStatus();
}

}  // namespace kangaroo::model

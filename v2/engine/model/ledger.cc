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

#include "model/ledger.h"

namespace kangaroo::model {

int64_t Ledger::BalanceAt(date_t date) const {
  return transactions_.WeightTo(date);
}

int64_t Ledger::BalanceToday() const {
  return transactions_.WeightTo(date::CurrentDate());
}

int64_t Ledger::BalanceFrom(date_t date) const {
  return transactions_.WeightFrom(date);
}

int64_t Ledger::BalanceToEnd() const { return transactions_.Weight(); }

int64_t Ledger::BalanceBetween(date_t from, date_t to) const {
  return transactions_.WeightBetween(from, to);
}

Ledger::LedgerRange Ledger::TransactionsBetween(
    std::optional<date_t> from, std::optional<date_t> to) const {
  return LedgerRange(
      from ? transactions_.LowerBound(from.value()) : transactions_.begin(),
      to ? transactions_.UpperBound(to.value()) : transactions_.end());
}

int64_t Ledger::BalanceBefore(const Transaction* transaction,
                              int64_t account_id) const {
  return transactions_.WeightBeforeIterator(
      transactions_.find(transaction->date(), {transaction, account_id}));
}

date_t Ledger::FirstTransactionDate() const {
  return empty() ? static_cast<date_t>(0) : transactions_.first_key();
}
date_t Ledger::LastTransactionDate() const {
  return empty() ? static_cast<date_t>(0) : transactions_.last_key();
}

// void Ledger::InsertTransaction(const Transaction* transaction,
//                                const Split& split) {
//   transactions_.Insert(transaction->date(), {transaction,
//   split.account_id()},
//                        split.amount_micros());
// }

// void Ledger::UpdateTransaction(const Transaction* transaction,
//                                int64_t account_id) {}
// void Ledger::RemoveTransaction(const Transaction* transaction,
//                                int64_t account_id) {
//   transactions_.Remove(transaction->date(), {transaction, account_id});
// }

void LedgerManager::InsertTransaction(const Transaction& transaction) {
  for (const Split& split : transaction.split()) {
    auto it = ledgers_.find(split.account_id());
    if (it == ledgers_.end()) continue;
    it->second->transactions_.Insert(transaction.date(),
                                     {&transaction, split.account_id()},
                                     split.amount_micros());
  }
}
void LedgerManager::UpdateTransaction(const Transaction& existing,
                                      const Transaction& updated) {
  std::unordered_map<uint64_t, const Split*>
      existing_splits;  // account_id to split.

  for (const Split& split : existing.split()) {
    existing_splits[split.account_id()] = &split;
  }

  // New splits
  for (const Split& split : existing.split()) {
    auto ledger_it = ledgers_.find(split.account_id());
    if (ledger_it == ledgers_.end()) continue;

    auto existing_split_it = existing_splits.find(split.account_id());

    if (existing_split_it == existing_splits.end()) {
      // New split
      ledger_it->second->transactions_.Insert(updated.date(),
                                              {&existing, split.account_id()},
                                              split.amount_micros());
      continue;
    }

    // Must be a modification
    if (existing.date() != updated.date()) {
      ledger_it->second->transactions_.Move(
          existing.date(), {&existing, split.account_id()}, updated.date());
    }
    if (existing_split_it->second->amount_micros() != split.amount_micros()) {
      ledger_it->second->transactions_.SetWeight(
          existing.date(), {&existing, split.account_id()},
          split.amount_micros());
    }

    // Remove the existing split. existing_splits will then only contain removed
    // splits after this loop.
    existing_splits.erase(existing_split_it);
  }

  // Modified splits (same account ID), diff is with price, date, or both.

  // All remaining splits in existing_splits have been removed in the updated
  // transaction.
  for (const auto [account_id, split] : existing_splits) {
    auto it = ledgers_.find(account_id);
    if (it == ledgers_.end()) continue;
    it->second->transactions_.Remove(existing.date(), {&existing, account_id});
  }
}

void LedgerManager::RemoveTransaction(const Transaction& transaction) {
  for (const Split& split : transaction.split()) {
    auto it = ledgers_.find(split.account_id());
    if (it == ledgers_.end()) continue;
    it->second->transactions_.Remove(transaction.date(),
                                     {&transaction, split.account_id()});
  }
}

}  // namespace kangaroo::model

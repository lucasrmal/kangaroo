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

#ifndef MODEL_LEDGER_H
#define MODEL_LEDGER_H

#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "model/proto/transaction.pb.h"
#include "model/types/date.h"
#include "util/augmented-treap-map.h"

namespace kangaroo::model {

class Ledger {
 public:
  using LedgerKey =
      std::pair<const Transaction*, int64_t>;  // <Transaction, AccountId>
  using LedgerMap = AugmentedTreapMap<date_t, LedgerKey, int64_t>;
  using LedgerIterator = LedgerMap::iterator;
  using LedgerRange = std::pair<LedgerIterator, LedgerIterator>;
  Ledger() = default;

  // Returns the balance on `date`. Includes all transactions with date <=
  // `date`.
  int64_t BalanceAt(date_t date) const;

  // Returns the balance today. Includes all transactions with date <= current.
  int64_t BalanceToday() const;

  // Returns the balance including all transactions. Include all future-dated
  // transactions that are present.
  int64_t BalanceToEnd() const;

  // Returns the balance from `date` to the end. Include all future-dated
  // transactions that are present.
  int64_t BalanceFrom(date_t date) const;

  // Equiv to balance(from) - balance(to-1)
  int64_t BalanceBetween(date_t from, date_t to) const;

  LedgerRange TransactionsBetween(std::optional<date_t> from,
                                  std::optional<date_t> to) const;

  // *
  //   @brief costBasisAt
  //   Only relevant for investment account ledgers.

  //   @param[in] _date Only consider transactions at or before _date. If
  //   invalid, all transactions are considered.
  //   @return The cost basis for all the shares in the account at _date

  // int64_t costBasisAt(date_t _date) const;

  // int64_t costBasisBefore(const KLib::Transaction* _tr) const;

  // int64_t costBasis() const { return costBasisBefore(nullptr); }

  // Returns the balance before `transaction`; transactions are ordered by date.
  // This may include some transactions with date equal to
  // `transaction->date()`.
  int64_t BalanceBefore(const Transaction* transaction,
                        int64_t account_id) const;

  date_t FirstTransactionDate() const;
  date_t LastTransactionDate() const;

  size_t size() const { return transactions_.size(); }
  bool empty() const { return transactions_.empty(); }

 private:
  LedgerMap transactions_;

  friend class LedgerManager;
};

class LedgerManager {
 public:
  const Ledger* Find(int64_t account_id) const {
    auto it = ledgers_.find(account_id);
    return it == ledgers_.end() ? nullptr : it->second.get();
  }

 private:
  Ledger* AddLedger(int64_t account_id) {
    auto it = ledgers_.find(account_id);
    if (it == ledgers_.end()) {
      it = ledgers_.insert({account_id, std::make_unique<Ledger>()}).first;
    }
    return it->second.get();
  }
  void RemoveLedger(int64_t account_id) { ledgers_.erase(account_id); }

  void InsertTransaction(const Transaction& transaction);
  void UpdateTransaction(const Transaction& existing,
                         const Transaction& updated);
  void RemoveTransaction(const Transaction& transaction);

  absl::flat_hash_map<int64_t, std::unique_ptr<Ledger>> ledgers_;

  friend class AccountManager;
  friend class TransactionManager;
};

}  // namespace kangaroo::model

#endif  // MODEL_LEDGER_H

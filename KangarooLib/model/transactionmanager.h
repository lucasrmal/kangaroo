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

#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H

#include <QList>
#include <QVector>

#include "../interfaces/scriptable.h"
#include "stored.h"
#include "transaction.h"

namespace KLib {

class TransactionManager : public IStored {
  Q_OBJECT
  K_SCRIPTABLE(TransactionManager)

  Q_PROPERTY(int count READ count)

 public:
  Q_INVOKABLE KLib::Transaction* get(int _id) const;

  int count() const { return m_transactions.size(); }

  Q_INVOKABLE const QHash<int, Transaction*>& transactions() const {
    return m_transactions;
  }

  void reassignTransactions(const std::vector<int>& transaction_ids,
                            int reassign_from, int reassign_to,
                            QStringList* errors);

  static TransactionManager* instance() { return m_instance; }

 protected:
  virtual void load(QXmlStreamReader& _reader) override;
  virtual void save(QXmlStreamWriter& _writer) const override;
  virtual void unload() override;
  virtual void afterLoad() override;

 private:
  void add(Transaction* _transaction);
  void remove(int _id);

  QHash<int, Transaction*> m_transactions;

  static int newId();

  static TransactionManager* m_instance;
  static int m_nextId;

  friend class LedgerManager;
};

}  // namespace KLib

Q_DECLARE_METATYPE(KLib::TransactionManager*)

#endif  // TRANSACTIONMANAGER_H

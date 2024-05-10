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

#ifndef ACCOUNTCONTROLLER_H
#define ACCOUNTCONTROLLER_H

#include <QAbstractItemModel>
#include <QDate>
#include <QSortFilterProxyModel>

namespace KLib {

class Account;

namespace AccountColumn {
enum Columns {
  NAME = 0,
  BALANCE_CURRENT,
  VALUE_CURRENT,
  // BALANCE_FUTURE,
  // VALUE_FUTURE,
  NOTE,
  NumColumns
};
}

class AccountController : public QAbstractItemModel {
  Q_OBJECT

  struct AccountNode {
    AccountNode() : account(nullptr), parent(nullptr) {}
    AccountNode(Account* _a, AccountNode* _parent,
                AccountController* _controller)
        : account(_a), parent(_parent) {
      loadRec(_controller);
    }

    ~AccountNode();

    AccountNode* find(Account* _a);

    Account* account;

    AccountNode* parent;
    QList<AccountNode*> children;

    void loadRec(AccountController* _controller);
  };

 public:
  AccountController(bool _openAccountsOnly, QObject* _parent = nullptr);

  AccountController(Account* _topLevel, bool _openAccountsOnly, int _flags,
                    QObject* _parent = nullptr);

  virtual ~AccountController() { delete m_root; }

  bool showOpenOnly() const { return m_openOnly; }

  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const override;

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex& index) const override;

  int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  int columnCount(const QModelIndex& parent = QModelIndex()) const override;

  Account* accountForIndex(const QModelIndex& _index) const;

 public slots:
  void setShowOpenOnly(bool _show);

  /**
   * @brief Shows the balance between _start and _end, inclusive.
   * @param _start
   * @param _end
   *
   * If start is invalid, shows the balance from the beginning. If end
   * is invalid, shows the balance until the end (may be in the future).
   */
  void setBalancesBetween(const QDate& _start, const QDate& _end);

 private slots:
  void onAccountAdded(KLib::Account* a);
  void onAccountRemoved(KLib::Account* a);
  void onAccountModified(KLib::Account* a);

 private:
  void loadAccounts();

  /**
   * @brief Used by AccountNode
   * @param _a An account
   * @return True if _a should be included in the controller, False otherwise.
   */
  bool canAddAccount(const Account* _a);

  Account* m_topLevel;
  bool m_openOnly;
  int m_flags;
  AccountNode* m_root;

  QDate m_startDate;
  QDate m_endDate;

  friend struct AccountNode;
  friend class AccountSortFilterProxyModel;
};

class AccountSortFilterProxyModel : public QSortFilterProxyModel {
 public:
  AccountSortFilterProxyModel(QObject* _parent);

  bool lessThan(const QModelIndex& _left,
                const QModelIndex& _right) const override;

  bool filterAcceptsRow(int source_row,
                        const QModelIndex& source_parent) const override;

  void setIncludeChildrenInFilter(bool _include);

 private:
  bool m_includeChildrenInFilter;
};

}  // namespace KLib

#endif  // ACCOUNTCONTROLLER_H

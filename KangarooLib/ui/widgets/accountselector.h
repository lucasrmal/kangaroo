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

#ifndef ACCOUNTSELECTOR_H
#define ACCOUNTSELECTOR_H

#include <QAbstractListModel>
#include <QComboBox>
#include <tuple>
#include <vector>

#include "../../klib.h"
#include "../../model/account.h"

namespace KLib {

class AccountSelectorModel;
class Security;
class Currency;

namespace AccountSelectorFlags {
enum Flags {
  // Default selector excludes closed/placeholder accounts and groups
  // multi-currency accounts as one.
  Flag_Default = 0x00,
  Flag_IncludePlaceholders = 0x01,
  Flag_IncludeClosed = 0x02,
  Flag_MultipleCurrencies = 0x04,

  Flag_PlaceholdersClosed = Flag_IncludePlaceholders | Flag_IncludeClosed,
  Flag_All =
      Flag_IncludePlaceholders | Flag_IncludeClosed | Flag_MultipleCurrencies
};
}

struct AccountSelectorParams {
  int selectorFlags = AccountSelectorFlags::Flag_Default;
  int typeFlags = AccountTypeFlags::Flag_All;

  // Only show accounts supporting this currency.
  // Inactive if empty or invalid.
  QString currencyRestrict = "";

  // Only show accounts supporting this security.
  // Inactive if Constants::NO_ID or invalid.
  int idSecurityRestrict = Constants::NO_ID;

  // Set this to an account to exclude from selection
  // Inactive if Constants::NO_ID or invalid.
  int idExcludeAccount = Constants::NO_ID;

  // Custom filter to use in addition to all of the above.
  std::function<bool(const Account*)> customFilter = nullptr;
};

class AccountSelector : public QComboBox {
  Q_OBJECT

 public:
  explicit AccountSelector(AccountSelectorParams _selectorParams,
                           QWidget* parent = nullptr);

  virtual ~AccountSelector() = default;

  void setCurrentAccount(int _id, const QString& _currency = QString());

  Account* currentAccount() const;
  QString currentCurrency() const;

  void resetSelectorParams(AccountSelectorParams _selectorParams);

 protected:
  void focusOutEvent(QFocusEvent* _e);

 signals:
  void currentAccountChanged(KLib::Account* _account);

 private slots:
  void onCurrentChanged(int _i);
  void onEditingFinished();

  void setCompletedIndex(const QString& _name);

 private:
  void init();
  AccountSelectorModel* m_model;

  int m_completedIndex;
};

class AccountSelectorModel : public QAbstractListModel {
  Q_OBJECT

  struct AccountModelElement {
    QString displayString;
    Account* account;
    QString currency;
  };

 public:
  AccountSelectorModel(AccountSelectorParams _selectorParams,
                       QObject* _parent = nullptr);

  QVariant data(const QModelIndex& _index, int _role) const;

  int rowCount(const QModelIndex&) const { return m_accounts.size(); }

  Account* accountAt(int _row) const { return m_accounts[_row].account; }
  QString currencyAt(int _row) const { return m_accounts[_row].currency; }
  int indexFor(const QString& _display) const;

 private:
  void loadData();
  void rec_load(Account* _parent, const QString& _prefix,
                bool (*filterFn)(const Account*,
                                 const AccountSelectorParams& params));

  AccountSelectorParams m_selectorParams;

  std::vector<AccountModelElement> m_accounts;
  QHash<QString, int> m_index;
  QHash<int, int> m_accountIndex;  ///< First: account ID, second: first index
                                   ///< of this account in m_accounts

  friend class AccountSelector;
};

}  // namespace KLib

#endif  // ACCOUNTSELECTOR_H

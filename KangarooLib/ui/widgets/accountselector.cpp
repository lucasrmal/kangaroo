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

#include "accountselector.h"

#include <QAbstractItemView>
#include <QCompleter>
#include <QLineEdit>
#include <QMessageBox>
#include <QtDebug>

#include "../../model/account.h"

namespace KLib {

AccountSelector::AccountSelector(AccountSelectorParams _selectorParams,
                                 QWidget* parent)
    : QComboBox(parent),
      m_model(new AccountSelectorModel(std::move(_selectorParams), this)) {
  init();
}

void AccountSelector::init() {
  m_completedIndex = Constants::NO_ID;

  setModel(m_model);
  setEditable(true);

  QCompleter* completer = new QCompleter(m_model, this);
  completer->setCaseSensitivity(Qt::CaseInsensitive);
  completer->setFilterMode(Qt::MatchContains);
  setCompleter(completer);

  setCurrentText("");

  // Widths of views
  int width = minimumSizeHint().width();
  view()->sizePolicy().setHorizontalPolicy(QSizePolicy::MinimumExpanding);
  view()->setMinimumWidth(width);

  completer->popup()->sizePolicy().setHorizontalPolicy(
      QSizePolicy::MinimumExpanding);
  completer->popup()->setMinimumWidth(width);

  // Connections
  connect(completer,
          static_cast<void (QCompleter::*)(const QString&)>(
              &QCompleter::highlighted),
          this, &AccountSelector::setCompletedIndex);

  connect(this,
          static_cast<void (AccountSelector::*)(int)>(
              &AccountSelector::currentIndexChanged),
          this, &AccountSelector::onCurrentChanged);

  connect(this->lineEdit(), &QLineEdit::editingFinished, this,
          &AccountSelector::onEditingFinished);
}

void AccountSelector::setCompletedIndex(const QString& _name) {
  int row = m_model->indexFor(_name);

  if (row != -1) {
    setCurrentIndex(row);
  }
}

void AccountSelector::onEditingFinished() {
  emit currentAccountChanged(currentAccount());
}

void AccountSelector::setCurrentAccount(int _id, const QString& _currency) {
  int index = m_model->m_accountIndex.value(_id, Constants::NO_ID);

  if (index != Constants::NO_ID) {
    if (!_currency.isEmpty()) {
      while (index + 1 < static_cast<int>(m_model->m_accounts.size()) &&
             m_model->m_accounts[index + 1].account->id() == _id &&
             m_model->m_accounts[index].currency != _currency) {
        ++index;
      }
    }

    setCurrentIndex(index);
  } else {
    setCurrentIndex(0);
  }
}

void AccountSelector::onCurrentChanged(int) {
  emit currentAccountChanged(currentAccount());
}

Account* AccountSelector::currentAccount() const {
  int row = m_model->indexFor(currentText());

  return row != -1 ? m_model->accountAt(row) : nullptr;
}

QString AccountSelector::currentCurrency() const {
  int row = m_model->indexFor(currentText());

  return row != -1 ? m_model->currencyAt(row) : nullptr;
}

void AccountSelector::resetSelectorParams(
    AccountSelectorParams _selectorParams) {
  m_model->m_selectorParams = std::move(_selectorParams);
  m_model->loadData();
  view()->reset();
}

void AccountSelector::focusOutEvent(QFocusEvent* _e) {
  QComboBox::focusOutEvent(_e);
}

AccountSelectorModel::AccountSelectorModel(
    AccountSelectorParams _selectorParams, QObject* _parent)
    : QAbstractListModel(_parent),
      m_selectorParams(std::move(_selectorParams)) {
  loadData();
}

QVariant AccountSelectorModel::data(const QModelIndex& _index,
                                    int _role) const {
  if (!_index.isValid()) {
    return QVariant();
  }

  if (_role == Qt::DisplayRole || _role == Qt::EditRole) {
    return m_accounts[_index.row()].displayString;
  } else if (_role == Qt::UserRole) {
    const Account* a = m_accounts[_index.row()].account;
    return a ? a->id() : Constants::NO_ID;
  } else {
    return QVariant();
  }
}

void AccountSelectorModel::loadData() {
  m_accounts.clear();

  // Add an empty account
  m_index[""] = 0;
  m_accounts.emplace_back("", nullptr, "");

  // Load all accounts (depth first search).
  auto filterFn = [](const Account* a,
                     const AccountSelectorParams& params) -> bool {
    if (params.idExcludeAccount != Constants::NO_ID &&
        a->id() != params.idExcludeAccount) {
      return false;
    }
    if (!a->isOpen() &&
        !(params.selectorFlags & AccountSelectorFlags::Flag_IncludeClosed)) {
      return false;
    }
    if (a->isPlaceholder() &&
        !(params.selectorFlags &
          AccountSelectorFlags::Flag_IncludePlaceholders)) {
      return false;
    }
    if (params.idSecurityRestrict != Constants::NO_ID &&
        params.idSecurityRestrict != a->idSecurity()) {
      return false;
    }
    if (!params.currencyRestrict.isEmpty() &&
        !a->supportsCurrency(params.currencyRestrict)) {
      return false;
    }
    if (!Account::matchesFlags(a, params.typeFlags)) {
      return false;
    }
    if (params.customFilter && !params.customFilter(a)) {
      return false;
    }
    return true;

    //    return (params.idExcludeAccount == Constants::NO_ID ||
    //            a->id() == params.idExcludeAccount) &&
    //           (a->isOpen() ||
    //            params.selectorFlags &
    //            AccountSelectorFlags::Flag_IncludeClosed) &&
    //           (!a->isPlaceholder() ||
    //            params.selectorFlags &
    //                AccountSelectorFlags::Flag_IncludePlaceholders) &&
    //           (params.idSecurityRestrict == Constants::NO_ID ||
    //            params.idSecurityRestrict == a->idSecurity()) &&
    //           (params.currencyRestrict.isEmpty() ||
    //            a->supportsCurrency(params.currencyRestrict)) &&
    //           Account::matchesFlags(a, params.selectorFlags) &&
    //           (!params.customFilter || params.customFilter(a));
  };
  rec_load(Account::getTopLevel(), "", filterFn);
}

int AccountSelectorModel::indexFor(const QString& _display) const {
  return m_index.contains(_display) ? m_index[_display] : -1;
}

void AccountSelectorModel::rec_load(
    Account* _current, const QString& _prefix,
    bool (*_filterFn)(const Account*, const AccountSelectorParams& params)) {
  auto displayWithPrefix = [&_prefix](const QString& disp) {
    return _prefix.isEmpty() ? disp : _prefix + Account::PATH_SEPARATOR + disp;
  };

  // Show the account if the conditions are OK
  if (_filterFn(_current, m_selectorParams)) {
    // If account is multi currency and multi currency enabled...
    const std::vector<QString> accountCurrencies = _current->allCurrencies();
    if (accountCurrencies.size() > 1 &&
        m_selectorParams.selectorFlags &
            AccountSelectorFlags::Flag_MultipleCurrencies) {
      for (const QString& cur : accountCurrencies) {
        if (m_selectorParams.currencyRestrict.isEmpty() ||
            m_selectorParams.currencyRestrict == cur) {
          QString display =
              displayWithPrefix(QString("%1 (%2)").arg(_current->name(), cur));

          m_index[display] = m_accounts.size();

          if (!m_accountIndex.contains(_current->id())) {
            m_accountIndex[_current->id()] = m_accounts.size();
          }

          m_accounts.emplace_back(display, _current, cur);
        }
      }
    } else {
      QString display = displayWithPrefix(_current->name());

      m_accountIndex[_current->id()] = m_accounts.size();
      m_index[display] = m_accounts.size();
      m_accounts.emplace_back(display, _current, _current->mainCurrency());
    }
  }

  // Recurse for all children
  for (Account* c : _current->getChildren()) {
    if (_current->type() != AccountType::TOPLEVEL) {
      if (_prefix.isEmpty()) {
        rec_load(c, _current->name(), _filterFn);
      } else {
        rec_load(c, _prefix + Account::PATH_SEPARATOR + _current->name(),
                 _filterFn);
      }
    } else {
      rec_load(c, "", _filterFn);
    }
  }
}

}  // namespace KLib

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

#include "simpleaccountcontroller.h"

#include <QColor>
#include <QDebug>
#include <QFont>
#include <QIcon>
#include <QPixmap>

#include "../model/account.h"
#include "../model/ledger.h"
#include "../model/pricemanager.h"

namespace KLib {

bool accountSort(Account* a, Account* b) {
  if (a->type() == b->type()) {
    return QString::compare(a->name(), b->name(), Qt::CaseInsensitive) < 0;
  } else {
    return Account::accountTypeWeight(a->type()) <
           Account::accountTypeWeight(b->type());
  }
}

bool accountNodeSort(SimpleAccountController::AccountNode* a,
                     SimpleAccountController::AccountNode* b) {
  return accountSort(a->account, b->account);
}

bool accountNodeEqual(SimpleAccountController::AccountNode* a,
                      SimpleAccountController::AccountNode* b) {
  return a->account == b->account;
}

SimpleAccountController::SimpleAccountController(QObject* _parent)
    : QAbstractItemModel(_parent), m_showClosedAccounts(false) {
  loadData();

  connect(LedgerManager::instance(), &LedgerManager::balanceTodayChanged, this,
          &SimpleAccountController::onBalanceTodayChanged);

  connect(Account::getTopLevel(), &Account::accountAdded, this,
          &SimpleAccountController::onAccountAdded);
  connect(Account::getTopLevel(), &Account::accountModified, this,
          &SimpleAccountController::onAccountModified);
  connect(Account::getTopLevel(), &Account::accountRemoved, this,
          &SimpleAccountController::onAccountRemoved);
}

QVariant SimpleAccountController::data(const QModelIndex& _index,
                                       int _role) const {
  if (_index.isValid() && _index.internalPointer()) {
    return static_cast<AccountNode*>(_index.internalPointer())
        ->data(_index.column(), _role);
  } else {
    return QVariant();
  }
}

SimpleAccountController::AccountNode::AccountNode(Account* _account,
                                                  AccountNode* _parent)
    : parent(_parent),
      account(_account),
      category(AccountClassification::INVALID) {}

QVariant SimpleAccountController::AccountNode::data(int _column,
                                                    int _role) const {
  switch (_role) {
    case Qt::DecorationRole:
      if (_column == (int)SimpleAccountControllerColumn::NAME && account) {
        return QIcon(account->defaultPicture(true));
      }
      break;

    case Qt::DisplayRole:
      if (_column == (int)SimpleAccountControllerColumn::NAME) {
        if (account) {
          return account->name();
        } else {
          switch (category) {
            case AccountClassification::CASH_FLOW:
              return tr("Cash Flow");

            case AccountClassification::INVESTING:
              return tr("Investing");

            case AccountClassification::PROPERTY_DEBT:
              return tr("Property");

            case AccountClassification::OTHER:
              return tr("Other");

            default:
              return QVariant();
          }
        }
      } else if (_column == (int)SimpleAccountControllerColumn::BALANCE) {
        return account ? /*account->idSecurity() == Constants::NO_ID ?*/ account
                             ->formatAmount(account->type() ==
                                                    AccountType::BROKERAGE
                                                ? account->brokerageValueToday()
                                                : account->balanceToday())
                       //: QString::number(account->balanceToday().toDouble())
                       : Account::getTopLevel()->formatAmount(categoryBalance);
      }
      break;

    case Qt::TextAlignmentRole:
      if (_column == (int)SimpleAccountControllerColumn::BALANCE) {
        return (int)Qt::AlignRight | Qt::AlignVCenter;
      }
      break;

    case Qt::TextColorRole:
      if (_column == (int)SimpleAccountControllerColumn::BALANCE &&
          ((account && account->balanceToday() < 0) ||
           (category != AccountClassification::INVALID &&
            categoryBalance < 0))) {
        return QColor(Qt::red);
      }
      break;

    case Qt::FontRole: {
      QFont f;
      // f.setPointSizeF(f.pointSizeF()-0.5);
      if (_column == (int)SimpleAccountControllerColumn::BALANCE &&
          category != AccountClassification::INVALID) {
        f.setBold(true);
      } else if (account && !account->isOpen()) {
        f.setStrikeOut(true);
      }
      //            else
      //            {
      //                QFont f;
      //                f.setPointSize(f.pointSize()-1);
      //                return f;
      //            }
      return f;
    }

  }  // switch role

  return QVariant();
}

void SimpleAccountController::AccountNode::updateTotal() {
  if (category != AccountClassification::INVALID) {
    categoryBalance = 0;
    const QString topCurrency = Account::getTopLevel()->mainCurrency();
    const QDate today = QDate::currentDate();

    for (AccountNode* n : children) {
      Amount addBy;
      if (n->account->type() == AccountType::BROKERAGE) {
        addBy = n->account->brokerageValueToday();
      } else {
        Amount bal = n->account->balanceToday();

        if (bal != 0) {
          if (n->account &&
              !n->account->mainCurrency().isEmpty())  // Currency account
          {
            addBy = bal * PriceManager::instance()->rate(
                              n->account->mainCurrency(), topCurrency, today);
          } else if (n->account)  // Security-based account
          {
            addBy = bal * PriceManager::instance()->rate(
                              n->account->idSecurity(), topCurrency, today);
          }
        }
      }

      if (Account::generalType(n->account->type()) == AccountType::LIABILITY) {
        categoryBalance -= addBy;
      } else {
        categoryBalance += addBy;
      }
    }
  }
}

QVariant SimpleAccountController::headerData(int _section,
                                             Qt::Orientation _orientation,
                                             int _role) const {
  if (_orientation == Qt::Horizontal && _role == Qt::DisplayRole) {
    if (_section == (int)SimpleAccountControllerColumn::NAME) {
      return tr("Name");
    } else if (_section == (int)SimpleAccountControllerColumn::BALANCE) {
      return tr("Balance");
    }
  }

  return QVariant();
}

QModelIndex SimpleAccountController::index(int _row, int _column,
                                           const QModelIndex& _parent) const {
  if (!hasIndex(_row, _column, _parent)) return QModelIndex();

  AccountNode* parentItem;

  if (!_parent.isValid()) {
    parentItem = m_root;
  } else {
    parentItem = static_cast<AccountNode*>(_parent.internalPointer());
  }

  if (_row < parentItem->children.count()) {
    return createIndex(_row, _column, parentItem->children[_row]);
  } else {
    return QModelIndex();
  }
}

QModelIndex SimpleAccountController::parent(const QModelIndex& _index) const {
  if (!_index.isValid()) return QModelIndex();

  AccountNode* childItem = static_cast<AccountNode*>(_index.internalPointer());
  AccountNode* parentItem = childItem->parent;

  if (!parentItem || parentItem == m_root) return QModelIndex();

  // Find the row
  int row = parentItem->parent->children.indexOf(parentItem);

  if (row != -1) {
    return createIndex(row, 0, parentItem);
  } else {
    return QModelIndex();
  }
}

Qt::ItemFlags SimpleAccountController::flags(const QModelIndex& _index) const {
  // If a category, it cannot be selected.
  if (_index.isValid()) {
    AccountNode* node = static_cast<AccountNode*>(_index.internalPointer());

    if (node && node->category != AccountClassification::INVALID) {
      return Qt::ItemIsEnabled;
    }
  }

  return QAbstractItemModel::flags(_index);
}

int SimpleAccountController::rowCount(const QModelIndex& _parent) const {
  AccountNode* parentItem;
  if (_parent.column() > 0) return 0;

  if (!_parent.isValid()) {
    parentItem = m_root;
  } else {
    parentItem = static_cast<AccountNode*>(_parent.internalPointer());
  }

  return parentItem->children.count();
}

int SimpleAccountController::columnCount(const QModelIndex& _parent) const {
  Q_UNUSED(_parent)
  return (int)SimpleAccountControllerColumn::NumColumns;
}

bool SimpleAccountController::indexIsAfterTypeChange(
    const QModelIndex& _index) const {
  if (_index.isValid() && _index.row() > 0) {
    AccountNode* node = static_cast<AccountNode*>(_index.internalPointer());

    if (node && node->account) {
      return node->parent->children[_index.row() - 1]->account->type() !=
             node->account->type();
    }
  }

  return false;
}

QModelIndex SimpleAccountController::indexForAccount(Account* _account) const {
  if (_account) {
    // Try to find the account

    for (AccountNode* cat : m_root->children) {
      for (auto i = cat->children.begin(); i != cat->children.end(); ++i) {
        if ((*i)->account && (*i)->account == _account) {
          return createIndex(i - cat->children.begin(), 0, *i);
        }
      }
    }
  }

  return QModelIndex();
}

QModelIndex SimpleAccountController::indexForCategory(
    AccountClassification _cat) const {
  if (_cat != AccountClassification::INVALID) {
    for (auto i = m_root->children.begin(); i != m_root->children.end(); ++i) {
      if ((*i)->category == _cat) {
        return createIndex(i - m_root->children.begin(), 0, *i);
      }
    }
  }

  return QModelIndex();
}

Account* SimpleAccountController::accountAt(const QModelIndex& _index) const {
  if (_index.isValid() && static_cast<AccountNode*>(_index.internalPointer())) {
    return static_cast<AccountNode*>(_index.internalPointer())->account;
  } else {
    return nullptr;
  }
}

void SimpleAccountController::setShowClosedAccounts(bool _show) {
  if (m_showClosedAccounts != _show) {
    beginResetModel();
    delete m_root;
    m_showClosedAccounts = _show;
    loadData();
    endResetModel();
  }
}

void SimpleAccountController::onAccountAdded(KLib::Account* a) {
  AccountClassification c = Account::classificationForType(a->type());

  if (c == AccountClassification::INVALID || a->isPlaceholder() ||
      (!a->isOpen() && !m_showClosedAccounts) ||
      a->type() == AccountType::INVESTMENT) {
    return;
  }

  for (AccountNode* cat : m_root->children) {
    if (cat->category == c) {
      // Find where to add the account in the sorted list
      AccountNode* n = new AccountNode(a, cat);
      auto pos = std::upper_bound(cat->children.begin(), cat->children.end(), n,
                                  accountNodeSort);

      if (pos == cat->children.end() || (*pos)->account != a) {
        beginInsertRows(createIndex(m_root->children.indexOf(cat), 0, cat),
                        pos - cat->children.begin(),
                        pos - cat->children.begin());
        cat->children.insert(pos, n);
        endInsertRows();

        cat->updateTotal();
      } else {
        delete n;
      }

      break;
    }
  }
}

void SimpleAccountController::onAccountRemoved(KLib::Account* a) {
  // We need to go through EVERY classification to find the account...

  for (AccountNode* cat : m_root->children) {
    // Find where to remove the account in the sorted list
    AccountNode n(a, cat);
    auto pos = cat->children.begin();

    for (; pos != cat->children.end(); ++pos) {
      if ((*pos)->account == a) break;
    }

    if (pos != cat->children.end()) {
      int row = pos - cat->children.begin();
      beginRemoveRows(createIndex(m_root->children.indexOf(cat), 0, cat), row,
                      row);
      delete cat->children.takeAt(row);
      endRemoveRows();

      cat->updateTotal();

      break;
    }
  }
}

void SimpleAccountController::onAccountModified(KLib::Account* a) {
  AccountClassification c = Account::classificationForType(a->type());

  if (c == AccountClassification::INVALID ||
      (!a->isOpen() && !m_showClosedAccounts) || a->isPlaceholder() ||
      a->type() == AccountType::INVESTMENT) {
    // Maybe it was somewhere else before (or it was closed)... try to remove
    // it!
    onAccountRemoved(a);
    return;
  }

  for (AccountNode* cat : m_root->children) {
    if (cat->category == c) {
      // Check if the account is indeed in this category. If not, remove it
      // and add it in the correct category.
      AccountNode n(a, cat);

      auto pos = cat->children.begin();

      for (; pos != cat->children.end(); ++pos) {
        if ((*pos)->account == a) break;
      }

      if (pos == cat->children.end()) {
        // We need to remove it from wherever it is (if it's somewhere), and
        // add it at the correct location.
        onAccountRemoved(a);
        onAccountAdded(a);
      } else if (pos != cat->children.end() && !a->isOpen() &&
                 !m_showClosedAccounts) {
        onAccountRemoved(a);
      } else  // Check if it is in the correct location!
      {
        auto range = std::equal_range(cat->children.begin(),
                                      cat->children.end(), &n, accountNodeSort);
        int curRow = pos - cat->children.begin();

        if (range.first <= pos && range.second > pos) {
          // Everything good, simply reload the account!
          emit dataChanged(createIndex(curRow, 0, *pos),
                           createIndex(curRow, 0, *pos));
        } else  // We need to take the account and put it in the correct
                // range!
        {
          QModelIndex parentIndex =
              createIndex(m_root->children.indexOf(cat), 0, cat);
          int destRow = range.first - cat->children.begin();
          beginMoveRows(parentIndex, curRow, curRow, parentIndex, destRow);
          // If adding after the cur row, take in account that curRow will
          // have been removed before the insert
          //  (remove curRow, then insert).
          cat->children.move(curRow, destRow < curRow ? destRow : destRow - 1);
          endMoveRows();
        }
      }
    }
  }
}

void SimpleAccountController::onBalanceTodayChanged(
    int _idAccount, const Balances& _difference) {
  // Search for the right account
  for (AccountNode* n : m_root->children) {
    int row = 0;
    for (AccountNode* c : n->children) {
      if (c->account && c->account->id() == _idAccount) {
        for (auto i = _difference.begin(); i != _difference.end(); ++i) {
          if (!i.key().isEmpty()) {
            if (i.value() != 0) {
              n->categoryBalance +=
                  i.value() * PriceManager::instance()->rate(
                                  i.key(),
                                  Account::getTopLevel()->mainCurrency(),
                                  QDate::currentDate());
              QModelIndex acctIndex = createIndex(/* row */ row,
                                                  /* col */ 0,
                                                  /* ptr */ c);
              QModelIndex catIndex = indexForCategory(n->category);

              emit dataChanged(acctIndex, acctIndex);
              emit dataChanged(catIndex, catIndex);
            }
          }
        }

        return;  // An account can only be in a single category
      }

      ++row;
    }
  }
}

void SimpleAccountController::loadData() {
  // Create the root
  m_root = new AccountNode(AccountClassification::INVALID, nullptr);

  // Go over all accounts and put them in the correct bins
  QList<Account*> accounts[(int)AccountClassification::NumCategories];

  for (Account* a : Account::getTopLevel()->accounts()) {
    AccountClassification c = Account::classificationForType(a->type());

    if (c != AccountClassification::INVALID && !a->isPlaceholder() &&
        (a->isOpen() || m_showClosedAccounts) &&
        a->type() != AccountType::INVESTMENT) {
      accounts[(int)c] << a;
    }
  }

  // Sort and create the final structure
  for (int i = 0; i < (int)AccountClassification::NumCategories; ++i) {
    std::sort(accounts[i].begin(), accounts[i].end(), accountSort);

    AccountNode* cur = new AccountNode((AccountClassification)i, m_root);
    m_root->children << cur;

    for (Account* a : accounts[i]) {
      cur->children << new AccountNode(a, cur);
    }
  }

  // Load total amounts
  for (AccountNode* n : m_root->children) {
    n->updateTotal();
  }
}

}  // namespace KLib

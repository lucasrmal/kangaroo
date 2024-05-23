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

#include "accountcontroller.h"

#include <QDebug>
#include <QFont>
#include <functional>

#include "../model/account.h"
#include "../model/currency.h"
#include "../ui/core.h"

namespace KLib {

AccountController::AccountController(bool _openAccountsOnly, QObject* _parent)
    : AccountController(Account::getTopLevel(), _openAccountsOnly,
                        AccountTypeFlags::Flag_All, _parent) {}

AccountController::AccountController(Account* _topLevel, bool _openAccountsOnly,
                                     int _flags, QObject* _parent)
    : QAbstractItemModel(_parent),
      m_topLevel(_topLevel),
      m_openOnly(_openAccountsOnly),
      m_flags(_flags),
      m_endDate(QDate::currentDate()) {
  loadAccounts();

  connect(m_topLevel, &Account::accountAdded, this,
          &AccountController::onAccountAdded);
  connect(m_topLevel, &Account::accountModified, this,
          &AccountController::onAccountModified);
  connect(m_topLevel, &Account::accountRemoved, this,
          &AccountController::onAccountRemoved);
}

Account* AccountController::accountForIndex(const QModelIndex& _index) const {
  return _index.isValid()
             ? static_cast<AccountNode*>(_index.internalPointer())->account
             : nullptr;
}

void AccountController::setShowOpenOnly(bool _show) {
  if (m_openOnly != _show) {
    beginResetModel();
    m_openOnly = _show;
    delete m_root;
    loadAccounts();
    endResetModel();
  }
}

void AccountController::loadAccounts() {
  m_root = new AccountNode(m_topLevel, nullptr, this);
}

bool AccountController::canAddAccount(const Account* _a) {
  return (_a->isOpen() || !m_openOnly) && Account::matchesFlags(_a, m_flags);
}

void AccountController::AccountNode::loadRec(AccountController* _controller) {
  for (Account* a : account->getChildren()) {
    if (_controller->canAddAccount(a)) {
      children << new AccountNode(a, this, _controller);
    }
  }
}

AccountController::AccountNode::~AccountNode() {
  for (AccountNode* n : children) {
    delete n;
  }
}

AccountController::AccountNode* AccountController::AccountNode::find(
    Account* _a) {
  if (account->id() == _a->id()) return this;

  for (AccountNode* node : children) {
    AccountNode* theOne = node->find(_a);

    if (theOne) return theOne;
  }

  return nullptr;
}

void AccountController::setBalancesBetween(const QDate& _start,
                                           const QDate& _end) {
  m_startDate = _start;
  m_endDate = _end;
  emit dataChanged(index(0, AccountColumn::BALANCE_CURRENT),
                   index(0, AccountColumn::VALUE_CURRENT));
}

QVariant AccountController::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();

  Account* account =
      static_cast<AccountNode*>(index.internalPointer())->account;

  if (role == Qt::DisplayRole) {
    switch (index.column()) {
      case AccountColumn::NAME:
        return account->name();

      case AccountColumn::BALANCE_CURRENT:
        return account->isPlaceholder()
                   ? QVariant()
                   : account->formatAmount(
                         account->balanceBetween(m_startDate, m_endDate));

      case AccountColumn::VALUE_CURRENT:
        return Account::getTopLevel()->formatAmount(
            account->treeValueBetween(m_startDate, m_endDate));

      case AccountColumn::NOTE:
        return account->note();

        //            case AccountColumn::BALANCE_FUTURE:
        //                return account->formatAmount(account->balance());

        //            case AccountColumn::VALUE_FUTURE:
        //                return
        //                Account::getTopLevel()->formatAmount(account->treeValue());

      default:
        return "";
    }

  } else if (role == Qt::EditRole) {
    switch (index.column()) {
      case AccountColumn::BALANCE_CURRENT:
        return account->balanceToday().toDouble();

      case AccountColumn::VALUE_CURRENT:
        return account->treeValueToday().toDouble();

        //            case AccountColumn::BALANCE_FUTURE:
        //                return account->balance().toDouble();

        //            case AccountColumn::VALUE_FUTURE:
        //                return account->treeValue().toDouble();

      default:
        return 0.0;
    }

  } else if (role == Qt::DecorationRole &&
             index.column() == AccountColumn::NAME) {
    return QIcon(account->defaultPicture(true));
  } else if (role == Qt::FontRole) {
    if (!account->isOpen()) {
      QFont f;
      f.setStrikeOut(true);
      return f;
    } else if (account->isPlaceholder() ||
               (account->type() == AccountType::BROKERAGE &&
                index.column() != AccountColumn::BALANCE_CURRENT)) {
      QFont f;
      f.setBold(true);
      return f;
    } else {
      return QVariant();
    }
  } else if (role == Qt::TextAlignmentRole &&
             (index.column() == AccountColumn::BALANCE_CURRENT ||
              index.column() == AccountColumn::VALUE_CURRENT)) {
    return (int)Qt::AlignRight | Qt::AlignVCenter;
  } else if (role == Qt::TextColorRole &&
             (index.column() == AccountColumn::BALANCE_CURRENT ||
              index.column() == AccountColumn::VALUE_CURRENT) &&
             data(index, Qt::EditRole).toDouble() < 0.0) {
    return QColor(Qt::red);
  } else if (role == Qt::UserRole) {
    AccountNode* curNode = static_cast<AccountNode*>(index.internalPointer());

    if (curNode->parent && curNode->parent->children.last() == curNode) {
      return 1;
    }

    return 0;
  }

  return QVariant();
}

Qt::ItemFlags AccountController::flags(const QModelIndex& index) const {
  if (!index.isValid()) return 0;

  return QAbstractItemModel::flags(index);
}

QVariant AccountController::headerData(int section, Qt::Orientation orientation,
                                       int role) const {
  if (orientation != Qt::Horizontal) return QVariant();

  if (role == Qt::DisplayRole) {
    switch (section) {
      case AccountColumn::NAME:
        return tr("Name");

      case AccountColumn::BALANCE_CURRENT:
        return tr("Current Balance");

      case AccountColumn::VALUE_CURRENT:
        return tr("Current Value (%1)")
            .arg(Account::getTopLevel()->mainCurrency());

      case AccountColumn::NOTE:
        return tr("Note");

        //            case AccountColumn::BALANCE_FUTURE:
        //                return tr("Future Balance");

        //            case AccountColumn::VALUE_FUTURE:
        //                return tr("Future Value
        //                (%1)").arg(Account::getTopLevel()->currency());

      default:
        return "";
    }
  } else if (role == Qt::TextAlignmentRole) {
    switch (section) {
      case AccountColumn::BALANCE_CURRENT:
      case AccountColumn::VALUE_CURRENT:
        return (int)Qt::AlignRight | Qt::AlignVCenter;
    }
  }

  return QVariant();
}

QModelIndex AccountController::index(int row, int column,
                                     const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) return QModelIndex();

  AccountNode* parentItem;

  if (!parent.isValid()) {
    parentItem = m_root;
  } else {
    parentItem = static_cast<AccountNode*>(parent.internalPointer());
  }

  if (row < parentItem->children.count()) {
    return createIndex(row, column, parentItem->children[row]);
  } else {
    return QModelIndex();
  }
}

QModelIndex AccountController::parent(const QModelIndex& index) const {
  if (!index.isValid()) return QModelIndex();

  AccountNode* childItem = static_cast<AccountNode*>(index.internalPointer());
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

int AccountController::rowCount(const QModelIndex& parent) const {
  AccountNode* parentItem;
  if (parent.column() > 0) return 0;

  if (!parent.isValid()) {
    parentItem = m_root;
  } else {
    parentItem = static_cast<AccountNode*>(parent.internalPointer());
  }

  return parentItem->children.count();
}

int AccountController::columnCount(const QModelIndex&) const {
  return AccountColumn::NumColumns;
}

void AccountController::onAccountAdded(Account* a) {
  if (a && canAddAccount(a)) {
    AccountNode* theParent = m_root->find(a->parent());
    if (!theParent) return;

    int row = theParent->children.count();

    if (theParent->parent) {
      int parentRow = theParent->parent->children.indexOf(theParent);

      beginInsertRows(createIndex(parentRow, 0, theParent), row, row);
    } else  // Inserted as top level
    {
      beginInsertRows(QModelIndex(), row, row);
    }

    theParent->children << new AccountNode(a, theParent, this);
    endInsertRows();
  }
}

void AccountController::onAccountRemoved(Account* a) {
  if (a) {
    AccountNode* node = m_root->find(a);

    if (node) {
      AccountNode* theParent = node->parent;
      int row = theParent->children.indexOf(node);
      int parentRow = theParent->parent
                          ? theParent->parent->children.indexOf(theParent)
                          : 0;
      beginRemoveRows(createIndex(parentRow, 0, theParent), row, row);
      theParent->children.removeAt(row);
      endRemoveRows();
    }
  }
}

void AccountController::onAccountModified(Account* a) {
  if (a) {
    AccountNode* node = m_root->find(a);

    if (!node && canAddAccount(a)) {
      onAccountAdded(a);
    } else if (!canAddAccount(a) && node) {
      onAccountRemoved(a);
    } else if (node) {
      AccountNode* theParent = node->parent;
      if (theParent && theParent->children.indexOf(node) != -1) {
        int row = theParent->children.indexOf(node);
        emit dataChanged(
            createIndex(row, 0, theParent),
            createIndex(row, AccountColumn::VALUE_CURRENT, theParent));
      }
    }
  }
}

AccountSortFilterProxyModel::AccountSortFilterProxyModel(QObject* _parent)
    : QSortFilterProxyModel(_parent), m_includeChildrenInFilter(true) {
  setFilterKeyColumn(AccountColumn::NAME);
  setFilterCaseSensitivity(Qt::CaseInsensitive);
}

bool AccountSortFilterProxyModel::lessThan(const QModelIndex& _left,
                                           const QModelIndex& _right) const {
  if (_left.column() == AccountColumn::NAME) {
    AccountController::AccountNode* l =
        static_cast<AccountController::AccountNode*>(_left.internalPointer());
    AccountController::AccountNode* r =
        static_cast<AccountController::AccountNode*>(_right.internalPointer());

    if (!l) {
      return true;
    } else if (!r) {
      return false;
    } else if (l->account->type() == r->account->type()) {
      return l->account->name() < r->account->name();
    } else {
      return Account::accountTypeWeight(l->account->type()) <
             Account::accountTypeWeight(r->account->type());
    }
  } else {
    return QSortFilterProxyModel::lessThan(_left, _right);
  }
}

bool AccountSortFilterProxyModel::filterAcceptsRow(
    int source_row, const QModelIndex& source_parent) const {
  std::function<bool(const QModelIndex&)> ancestorMatches;
  ancestorMatches = [this, &ancestorMatches](const QModelIndex& parent) {
    if (!parent.isValid()) {
      return false;
    } else if (sourceModel()
                   ->data(parent, filterRole())
                   .toString()
                   .contains(filterRegExp())) {
      return true;
    } else {
      return ancestorMatches(parent.parent());
    }
  };

  // custom behaviour :
  if (!filterRegExp().isEmpty()) {
    // get source-model index for current row
    QModelIndex source_index = sourceModel()->index(
        source_row, this->filterKeyColumn(), source_parent);

    if (source_index.isValid()) {
      if (sourceModel()
              ->data(source_index, filterRole())
              .toString()
              .contains(filterRegExp())) {
        return true;
      } else if (m_includeChildrenInFilter &&
                 ancestorMatches(source_index.parent())) {
        return true;
      } else {
        // if any of its children matches the filter, then current index matches
        // the filter as well
        for (int i = 0; i < sourceModel()->rowCount(source_index); ++i) {
          if (filterAcceptsRow(i, source_index)) {
            return true;
          }
        }
        return false;
      }
    } else {
      return false;
    }
  } else {
    // parent call for initial behaviour
    return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
  }
}

void AccountSortFilterProxyModel::setIncludeChildrenInFilter(bool _include) {
  m_includeChildrenInFilter = _include;
  invalidateFilter();
}

}  // namespace KLib

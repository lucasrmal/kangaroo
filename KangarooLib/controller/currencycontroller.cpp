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

#include "currencycontroller.h"
#include "../model/currency.h"

#include <QSortFilterProxyModel>

namespace KLib {

CurrencyController::CurrencyController(QObject* parent)
    : QAbstractTableModel(parent), m_manager(CurrencyManager::instance()) {
  connect(m_manager, SIGNAL(currencyAdded(KLib::Currency*)), this,
          SLOT(onCurrencyAdded(KLib::Currency*)));
  connect(m_manager, SIGNAL(currencyModified(KLib::Currency*)), this,
          SLOT(onCurrencyModified(KLib::Currency*)));
  connect(m_manager, SIGNAL(currencyRemoved(KLib::Currency*)), this,
          SLOT(onCurrencyRemoved(KLib::Currency*)));
}

QSortFilterProxyModel* CurrencyController::sortProxy(QObject* _parent) {
  QSortFilterProxyModel* proxyCurrency = new QSortFilterProxyModel(_parent);
  proxyCurrency->setSourceModel(new CurrencyController(_parent));
  proxyCurrency->setSortCaseSensitivity(Qt::CaseInsensitive);
  proxyCurrency->sort(0);
  return proxyCurrency;
}

int CurrencyController::rowCount(const QModelIndex&) const {
  return m_manager->count();
}

int CurrencyController::columnCount(const QModelIndex&) const {
  return CurrencyColumn::NumColumns;
}

QVariant CurrencyController::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) return QVariant();

  if (index.row() >= m_manager->count()) return QVariant();

  if (role == Qt::DisplayRole || role == Qt::EditRole) {
    switch (index.column()) {
      case CurrencyColumn::CODE:
        return m_manager->at(index.row())->code();
      case CurrencyColumn::NAME:
        return m_manager->at(index.row())->name();
      case CurrencyColumn::SYMBOL:
        return m_manager->at(index.row())->customSymbol();
      default:
        return QVariant();
    }
  } else if (role == Qt::UserRole) {
    return m_manager->at(index.row())->code();
  } else {
    return QVariant();
  }
}

QVariant CurrencyController::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
  if (role != Qt::DisplayRole) return QVariant();

  if (orientation == Qt::Horizontal) {
    switch (section) {
      case CurrencyColumn::CODE:
        return tr("Code");
      case CurrencyColumn::NAME:
        return tr("Name");
      case CurrencyColumn::SYMBOL:
        return tr("Symbol");
      default:
        return QVariant();
    }
  } else {
    return QString("%1").arg(section + 1);
  }
}

Qt::ItemFlags CurrencyController::flags(const QModelIndex& _index) const {
  if (!_index.isValid()) return Qt::ItemIsEnabled;

  //    if (_index.column() == CurrencyColumn::CODE)
  //    {
  return QAbstractItemModel::flags(_index);
  //    }
  //    else
  //    {
  //        return QAbstractItemModel::flags(_index) | Qt::ItemIsEditable;
  //    }
}

bool CurrencyController::setData(const QModelIndex& index,
                                 const QVariant& value, int role) {
  Q_UNUSED(index)
  Q_UNUSED(value)
  Q_UNUSED(role)
  //    if (index.isValid() && role == Qt::EditRole)
  //    {
  //        switch (index.column())
  //        {
  //        case CurrencyColumn::NAME:
  //            m_manager->at(index.row())->setName(value.toString());
  //            break;
  //        case CurrencyColumn::SYMBOL:
  //            m_manager->at(index.row())->setCustomSymbol(value.toString());
  //            break;
  //        default:
  //            return false;
  //        }

  //        emit dataChanged(index, index);
  //        return true;
  //    }
  return false;
}

void CurrencyController::onCurrencyAdded(Currency* _i) {
  int rowNo = m_manager->currencies().indexOf(_i);
  beginInsertRows(QModelIndex(), rowNo, rowNo);
  endInsertRows();
}

void CurrencyController::onCurrencyRemoved(Currency* _i) {
  int rowNo = m_manager->currencies().indexOf(_i);
  beginRemoveRows(QModelIndex(), rowNo, rowNo);
  endRemoveRows();
}

void CurrencyController::onCurrencyModified(Currency* _i) {
  int rowNo = m_manager->currencies().indexOf(_i);
  emit dataChanged(createIndex(rowNo, 0),
                   createIndex(rowNo, columnCount() - 1));
}
}

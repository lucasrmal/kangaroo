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

#include "securitycontroller.h"
#include "../model/security.h"

#include <QSortFilterProxyModel>

namespace KLib {

SecurityController::SecurityController(QObject *parent)
    : QAbstractTableModel(parent),
      m_manager(SecurityManager::instance())
{
    connect(m_manager, &SecurityManager::securityAdded,    this, &SecurityController::onSecurityAdded);
    connect(m_manager, &SecurityManager::securityModified, this, &SecurityController::onSecurityModified);
    connect(m_manager, &SecurityManager::securityRemoved,  this, &SecurityController::onSecurityRemoved);
}

QSortFilterProxyModel* SecurityController::sortProxy(QObject* _parent)
{
    QSortFilterProxyModel* proxySecurity = new QSortFilterProxyModel(_parent);
    proxySecurity->setSourceModel(new SecurityController(_parent));
    proxySecurity->setSortCaseSensitivity(Qt::CaseInsensitive);
    proxySecurity->sort(SecurityColumn::NAME);
    return proxySecurity;
}

int SecurityController::rowCount(const QModelIndex&) const
{
    return m_manager->securityCount();
}

int SecurityController::columnCount(const QModelIndex&) const
{
    return SecurityColumn::NumColumns;
}

QVariant SecurityController::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_manager->securityCount())
        return QVariant();

    switch (role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
        switch (index.column())
        {
        case SecurityColumn::TYPE:
            return Security::typeToString(m_manager->securityAt(index.row())->type());

        case SecurityColumn::NAME:
            return m_manager->securityAt(index.row())->name();

        case SecurityColumn::MARKET:
            return m_manager->securityAt(index.row())->market();

        case SecurityColumn::SECTOR:
            return m_manager->securityAt(index.row())->sector();

        case SecurityColumn::SYMBOL:
            return m_manager->securityAt(index.row())->symbol();

        case SecurityColumn::CURRENCY:
            return m_manager->securityAt(index.row())->currency();

        default:
            return QVariant();
        }

    case Qt::UserRole:
        return m_manager->securityAt(index.row())->id();

    default:
        return QVariant();
    }
}

QVariant SecurityController::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case SecurityColumn::TYPE:
            return tr("Type");

        case SecurityColumn::NAME:
            return tr("Name");

        case SecurityColumn::MARKET:
            return tr("Market");

        case SecurityColumn::SECTOR:
            return tr("Sector");

        case SecurityColumn::SYMBOL:
            return tr("Symbol");

        case SecurityColumn::CURRENCY:
            return tr("Currency");

        default:
            return QVariant();
        }
    }
    else
    {
        return QString("%1").arg(section+1);
    }
}

Qt::ItemFlags SecurityController::flags(const QModelIndex& _index) const
{
    if (!_index.isValid())
        return Qt::ItemIsEnabled;

    return QAbstractItemModel::flags(_index);
}

void SecurityController::onSecurityAdded(Security* _i)
{
    int rowNo = m_manager->securities().indexOf(_i);

    if (rowNo != -1)
    {
        beginInsertRows(QModelIndex(), rowNo, rowNo);
        endInsertRows();
    }
}

void SecurityController::onSecurityRemoved(Security* _i)
{
    int rowNo = m_manager->securities().indexOf(_i);

    if (rowNo != -1)
    {
        beginRemoveRows(QModelIndex(), rowNo, rowNo);
        endRemoveRows();
    }
}

void SecurityController::onSecurityModified(Security* _i)
{
    int rowNo = m_manager->securities().indexOf(_i);

    if (rowNo != -1)
    {
        emit dataChanged(createIndex(rowNo,0), createIndex(rowNo, columnCount()-1));
    }
}

}

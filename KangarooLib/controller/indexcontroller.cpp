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

#include "indexcontroller.h"
#include "../model/security.h"

#include <QSortFilterProxyModel>

namespace KLib
{
    IndexController::IndexController(QObject *parent)
        : QAbstractTableModel(parent),
          m_manager(SecurityManager::instance())
    {
        connect(m_manager, &SecurityManager::indexAdded,    this, &IndexController::onIndexAdded);
        connect(m_manager, &SecurityManager::indexModified, this, &IndexController::onIndexModified);
        connect(m_manager, &SecurityManager::indexRemoved,  this, &IndexController::onIndexRemoved);
    }

    QSortFilterProxyModel* IndexController::sortProxy(QObject* _parent)
    {
        QSortFilterProxyModel* proxySecurity = new QSortFilterProxyModel(_parent);
        proxySecurity->setSourceModel(new IndexController(_parent));
        proxySecurity->setSortCaseSensitivity(Qt::CaseInsensitive);
        proxySecurity->sort(IndexColumn::NAME);
        return proxySecurity;
    }

    int IndexController::rowCount(const QModelIndex&) const
    {
        return m_manager->indexCount();
    }

    int IndexController::columnCount(const QModelIndex&) const
    {
        return IndexColumn::NumColumns;
    }

    QVariant IndexController::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid() || index.row() >= m_manager->indexCount())
            return QVariant();

        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
            switch (index.column())
            {
            case IndexColumn::NAME:
                return m_manager->indexAt(index.row())->name();

            case IndexColumn::SECTOR:
                return m_manager->indexAt(index.row())->sector();

            case IndexColumn::SYMBOL:
                return m_manager->indexAt(index.row())->symbol();

            case IndexColumn::CURRENCY:
                return m_manager->indexAt(index.row())->currency();

            default:
                return QVariant();
            }
            break;

        case Qt::UserRole:
            return m_manager->indexAt(index.row())->id();

        default:
            return QVariant();
        }
    }

    QVariant IndexController::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case IndexColumn::NAME:
                return tr("Name");

            case IndexColumn::SECTOR:
                return tr("Sector");

            case IndexColumn::SYMBOL:
                return tr("Symbol");

            case IndexColumn::CURRENCY:
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

    Qt::ItemFlags IndexController::flags(const QModelIndex& _index) const
    {
        if (!_index.isValid())
            return Qt::ItemIsEnabled;

        return QAbstractItemModel::flags(_index);
    }

    void IndexController::onIndexAdded(Security* _i)
    {
        int rowNo = m_manager->indexes().indexOf(_i);

        if (rowNo != -1)
        {
            beginInsertRows(QModelIndex(), rowNo, rowNo);
            endInsertRows();
        }
    }

    void IndexController::onIndexRemoved(Security* _i)
    {
        int rowNo = m_manager->indexes().indexOf(_i);

        if (rowNo != -1)
        {
            beginRemoveRows(QModelIndex(), rowNo, rowNo);
            endRemoveRows();
        }
    }

    void IndexController::onIndexModified(Security* _i)
    {
        int rowNo = m_manager->indexes().indexOf(_i);

        if (rowNo != -1)
        {
            emit dataChanged(createIndex(rowNo,0), createIndex(rowNo, columnCount()-1));
        }
    }

}


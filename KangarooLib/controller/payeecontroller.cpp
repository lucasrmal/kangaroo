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

#include "payeecontroller.h"
#include "../model/payee.h"
#include "../model/picturemanager.h"

#include <QIcon>
#include <QSortFilterProxyModel>
#include <QSettings>

namespace KLib {

    PayeeController::PayeeController(QObject *parent)
        : QAbstractTableModel(parent),
          m_manager(PayeeManager::instance())
    {
        QSettings s;
        m_showPics = s.value("UI/ShowPicturePayee").toBool();

        connect(m_manager, SIGNAL(payeeAdded(KLib::Payee*)), this, SLOT(onPayeeAdded(KLib::Payee*)));
        connect(m_manager, SIGNAL(payeeModified(KLib::Payee*)), this, SLOT(onPayeeModified(KLib::Payee*)));
        connect(m_manager, SIGNAL(payeeRemoved(KLib::Payee*)), this, SLOT(onPayeeRemoved(KLib::Payee*)));
    }

    QSortFilterProxyModel* PayeeController::sortProxy(QObject* _parent)
    {
        QSortFilterProxyModel* proxyPayee = new QSortFilterProxyModel(_parent);
        proxyPayee->setSourceModel(new PayeeController(_parent));
        proxyPayee->setSortCaseSensitivity(Qt::CaseInsensitive);
        proxyPayee->sort(0);
        return proxyPayee;
    }

    int PayeeController::rowCount(const QModelIndex&) const
    {
        return m_manager->count();
    }

    int PayeeController::columnCount(const QModelIndex&) const
    {
        return PayeeColumn::NumColumns;
    }

    QVariant PayeeController::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_manager->count())
            return QVariant();

        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            switch (index.column())
            {
            case PayeeColumn::NAME:
                return m_manager->at(index.row())->name();

            case PayeeColumn::ADDRESS:
                if (role == Qt::EditRole)
                {
                    return m_manager->at(index.row())->address();
                }
                else
                {
                    //We want the address to be displayed in a single line
                    return m_manager->at(index.row())->address().replace('\n', ' ');
                }

            case PayeeColumn::COUNTRY:
                return m_manager->at(index.row())->country();

            case PayeeColumn::PHONE:
                return m_manager->at(index.row())->phone();

            case PayeeColumn::FAX:
                return m_manager->at(index.row())->fax();

            case PayeeColumn::CONTACT_NAME:
                return m_manager->at(index.row())->contactName();

            case PayeeColumn::WEBSITE:
                return m_manager->at(index.row())->website();

            }

        }
        else if (m_showPics
                 && role == Qt::DecorationRole
                 && index.column() == PayeeColumn::NAME)
        {
            int id = m_manager->at(index.row())->idPicture();

            if (id != Constants::NO_ID)
            {
                return QIcon(PictureManager::instance()->get(id)->picture);
            }
        }
        else if (role == Qt::UserRole)
        {
            return m_manager->at(index.row())->id();
        }

        return QVariant();
    }

    QVariant PayeeController::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case PayeeColumn::NAME:
                return tr("Name");

            case PayeeColumn::ADDRESS:
                return tr("Address");

            case PayeeColumn::COUNTRY:
                return tr("Country");

            case PayeeColumn::PHONE:
                return tr("Phone");

            case PayeeColumn::FAX:
                return tr("Fax");

            case PayeeColumn::CONTACT_NAME:
                return tr("Contact Name");

            case PayeeColumn::WEBSITE:
                return tr("Website");

            default:
                return QVariant();

            }
        }
        else
        {
            return QString("%1").arg(section+1);
        }
    }

    Qt::ItemFlags PayeeController::flags(const QModelIndex &index) const
    {
        return QAbstractItemModel::flags(index);
    }

    bool PayeeController::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        Q_UNUSED(index)
        Q_UNUSED(value)
        Q_UNUSED(role)
//        if (index.isValid() && role == Qt::EditRole)
//        {
//            m_manager->at(index.row())->setName(value.toString());

//            emit dataChanged(index, index);
//            return true;
//        }
        return false;
    }

    void PayeeController::onPayeeAdded(Payee* _i)
    {
        int rowNo = m_manager->payees().indexOf(_i);
        beginInsertRows(QModelIndex(), rowNo, rowNo);
        endInsertRows();
    }

    void PayeeController::onPayeeRemoved(Payee* _i)
    {
        int rowNo = m_manager->payees().indexOf(_i);
        beginRemoveRows(QModelIndex(), rowNo, rowNo);
        endRemoveRows();
    }

    void PayeeController::onPayeeModified(Payee* _i)
    {
        int rowNo = m_manager->payees().indexOf(_i);
        emit dataChanged(createIndex(rowNo,0), createIndex(rowNo, PayeeColumn::NumColumns));
    }

}

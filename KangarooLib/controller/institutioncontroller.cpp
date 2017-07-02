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

#include "institutioncontroller.h"
#include "../model/institution.h"
#include "../model/picturemanager.h"

#include <QIcon>
#include <QSortFilterProxyModel>
#include <QSettings>

namespace KLib {

    InstitutionController::InstitutionController(AllowableInput _input, QObject *parent)
        : QAbstractTableModel(parent),
          m_manager(InstitutionManager::instance()),
          m_input(_input)
    {
        QSettings s;
        m_showPics = s.value("UI/ShowPictureInstitution").toBool();

        connect(m_manager, SIGNAL(institutionAdded(KLib::Institution*)), this, SLOT(onInstitutionAdded(KLib::Institution*)));
        connect(m_manager, SIGNAL(institutionModified(KLib::Institution*)), this, SLOT(onInstitutionModified(KLib::Institution*)));
        connect(m_manager, SIGNAL(institutionRemoved(KLib::Institution*)), this, SLOT(onInstitutionRemoved(KLib::Institution*)));
    }

    QSortFilterProxyModel* InstitutionController::sortProxy(QObject* _parent)
    {
        QSortFilterProxyModel* proxyInstitution = new QSortFilterProxyModel(_parent);
        proxyInstitution->setSourceModel(new InstitutionController(InstitutionController::AllowEmpty, _parent));
        proxyInstitution->setSortCaseSensitivity(Qt::CaseInsensitive);
        proxyInstitution->sort(0);
        return proxyInstitution;
    }

    int InstitutionController::rowCount(const QModelIndex&) const
    {
        return m_input == AllowEmpty ? m_manager->count() + 1 : m_manager->count();
    }

    int InstitutionController::columnCount(const QModelIndex&) const
    {
        return InstitutionColumn::NumColumns;
    }

    QVariant InstitutionController::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= rowCount())
            return QVariant();

        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            int row = index.row();

            if (m_input == AllowEmpty)
            {
                if (index.row() == 0)
                {
                    return QVariant();
                }
                else
                {
                    --row;
                }
            }

            switch (index.column())
            {
            case InstitutionColumn::NAME:
                return m_manager->at(row)->name();

            case InstitutionColumn::COUNTRY:
                return m_manager->at(row)->country();

            case InstitutionColumn::PHONE:
                return m_manager->at(row)->phone();

            case InstitutionColumn::FAX:
                return m_manager->at(row)->fax();

            case InstitutionColumn::CONTACT_NAME:
                return m_manager->at(row)->contactName();

            case InstitutionColumn::WEBSITE:
                return m_manager->at(row)->website();

            case InstitutionColumn::INSTITUTION_NUMBER:
                return m_manager->at(row)->institutionNumber();

            case InstitutionColumn::ROUTING_NUMBER:
                return m_manager->at(row)->routingNumber();

            }
        }
        else if (m_showPics
                 && role == Qt::DecorationRole
                 && index.column() == InstitutionColumn::NAME)
        {
            int row = index.row();

            if (m_input == AllowEmpty)
            {
                if (index.row() == 0)
                {
                    return QVariant();
                }
                else
                {
                    --row;
                }
            }

            int id = m_manager->at(row)->idPicture();

            if (id != Constants::NO_ID)
            {
                return QIcon(PictureManager::instance()->get(id)->picture);
            }
        }
        else if (role == Qt::UserRole)
        {
            if (m_input == AllowEmpty)
            {
                if (index.row() == 0)
                    return Constants::NO_ID;
                else
                    return m_manager->at(index.row()-1)->id();
            }
            else
            {
                return m_manager->at(index.row())->id();
            }
        }

        return QVariant();
    }

    QVariant InstitutionController::headerData(int _section, Qt::Orientation _orientation, int _role) const
    {
        if (_role != Qt::DisplayRole)
            return QVariant();

        if (_orientation == Qt::Horizontal)
        {
            switch (_section)
            {
            case InstitutionColumn::NAME:
                return tr("Name");

            case InstitutionColumn::COUNTRY:
                return tr("Country");

            case InstitutionColumn::PHONE:
                return tr("Phone");

            case InstitutionColumn::FAX:
                return tr("Fax");

            case InstitutionColumn::CONTACT_NAME:
                return tr("Contact Name");

            case InstitutionColumn::WEBSITE:
                return tr("Website");

            case InstitutionColumn::INSTITUTION_NUMBER:
                return tr("Institution Number");

            case InstitutionColumn::ROUTING_NUMBER:
                return tr("Routing Number");

            default:
                return QVariant();
            }
        }
        else
        {
            return QString("%1").arg(_section+1);
        }
    }

    Qt::ItemFlags InstitutionController::flags(const QModelIndex &index) const
    {
        return QAbstractItemModel::flags(index);
//        if (!index.isValid())
//            return Qt::ItemIsEnabled;

//        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool InstitutionController::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        Q_UNUSED(index)
        Q_UNUSED(value)
        Q_UNUSED(role)
//        if (index.isValid() && role == Qt::EditRole)
//        {
//            if (m_input == AllowEmpty && index.row() == 0)
//            {
//                return false;
//            }

//            int row = m_input == AllowEmpty ? index.row()-1 : index.row();

//            m_manager->at(row)->setName(value.toString());

//            emit dataChanged(index, index);
//            return true;
//        }
        return false;
    }

    void InstitutionController::onInstitutionAdded(Institution* _i)
    {
        int rowNo = m_manager->institutions().indexOf(_i);

        if (m_input == AllowEmpty)
        {
            rowNo++;
        }

        beginInsertRows(QModelIndex(), rowNo, rowNo);
        endInsertRows();
    }

    void InstitutionController::onInstitutionRemoved(Institution* _i)
    {
        int rowNo = m_manager->institutions().indexOf(_i);

        if (m_input == AllowEmpty)
        {
            rowNo++;
        }

        beginRemoveRows(QModelIndex(), rowNo, rowNo);
        endRemoveRows();
    }

    void InstitutionController::onInstitutionModified(Institution* _i)
    {
        int rowNo = m_manager->institutions().indexOf(_i);

        if (m_input == AllowEmpty)
        {
            rowNo++;
        }

        emit dataChanged(createIndex(rowNo,0), createIndex(rowNo, InstitutionColumn::NumColumns));
    }

}

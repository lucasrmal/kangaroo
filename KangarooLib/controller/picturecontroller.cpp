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

#include "picturecontroller.h"
#include "../model/picturemanager.h"

#include <QSortFilterProxyModel>

namespace KLib {

    PictureController::PictureController(bool _showText, QObject *parent)
        : QAbstractListModel(parent),
          m_pictures(PictureManager::instance()->pictures()),
          m_showText(_showText)
    {
        connect(PictureManager::instance(), &PictureManager::pictureAdded,    this, &PictureController::onPictureAdded);
        connect(PictureManager::instance(), &PictureManager::pictureRemoved,  this, &PictureController::onPictureRemoved);
        connect(PictureManager::instance(), &PictureManager::pictureModified, this, &PictureController::onPictureModified);
    }

    QSortFilterProxyModel* PictureController::sortProxy(bool _showText, QObject* _parent)
    {
        QSortFilterProxyModel* proxyPicture = new QSortFilterProxyModel(_parent);
        proxyPicture->setSourceModel(new PictureController(_showText,_parent));
        proxyPicture->setSortCaseSensitivity(Qt::CaseInsensitive);
        proxyPicture->sort(0);
        return proxyPicture;
    }

    int PictureController::rowCount(const QModelIndex&) const
    {
        return m_pictures.count();
    }

    QVariant PictureController::data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_pictures.count())
            return QVariant();

        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return m_showText ? m_pictures[index.row()]->name : QString();

        case Qt::DecorationRole:
            return m_pictures[index.row()]->thumbnail;

        case Qt::UserRole:
            return m_pictures[index.row()]->id;

        default:
            return QVariant();
        }
    }

    QVariant PictureController::headerData(int section, Qt::Orientation orientation, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        if (orientation == Qt::Horizontal)
            return QString("Picture");
        else
            return QString("%1").arg(section+1);
    }

    Qt::ItemFlags PictureController::flags(const QModelIndex &index) const
    {
        if (!index.isValid() || !m_showText)
            return QAbstractItemModel::flags(index);

        return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
    }

    bool PictureController::setData(const QModelIndex &index, const QVariant &value, int role)
    {
        if (index.isValid() && role == Qt::EditRole)
        {
            PictureManager::instance()->setName(m_pictures[index.row()]->id, value.toString());
            return true;
        }
        return false;
    }

    void PictureController::onPictureAdded(KLib::Picture* _p)
    {
        int rowNo = m_pictures.count();
        beginInsertRows(QModelIndex(), rowNo, rowNo);
        m_pictures.append(_p);
        endInsertRows();
    }

    void PictureController::onPictureRemoved(KLib::Picture* _p)
    {
        int rowNo = m_pictures.indexOf(_p);

        if (rowNo != -1)
        {
            beginRemoveRows(QModelIndex(), rowNo, rowNo);
            m_pictures.removeAt(rowNo);
            endRemoveRows();
        }
    }

    void PictureController::onPictureModified(KLib::Picture* _p)
    {
        int rowNo = m_pictures.indexOf(_p);

        if (rowNo != -1)
            emit dataChanged(createIndex(rowNo,0), createIndex(rowNo, 0));
    }

}

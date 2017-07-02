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

#ifndef PAYEECONTROLLER_H
#define PAYEECONTROLLER_H

#include <QAbstractTableModel>

class QSortFilterProxyModel;

namespace KLib {

    class PayeeManager;
    class Payee;

    namespace PayeeColumn
    {
        enum Columns
        {
            NAME = 0,
            ADDRESS,
            COUNTRY,
            PHONE,
            FAX,
            CONTACT_NAME,
            WEBSITE,
            NumColumns
        };
    }

    class PayeeController : public QAbstractTableModel
    {
        Q_OBJECT

        public:
            explicit PayeeController(QObject *parent = 0);

            int             rowCount(const QModelIndex &parent = QModelIndex()) const override;
            int             columnCount(const QModelIndex &parent = QModelIndex()) const override;
            QVariant        data(const QModelIndex &index, int role) const override;
            QVariant        headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

            Qt::ItemFlags   flags(const QModelIndex &index) const override;
            bool            setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

            static QSortFilterProxyModel* sortProxy(QObject *_parent);

        private slots:
            void onPayeeAdded(KLib::Payee* _p);
            void onPayeeRemoved(KLib::Payee* _p);
            void onPayeeModified(KLib::Payee* _p);

        private:
            PayeeManager* m_manager;

            bool m_showPics;


    };

}

#endif // PAYEECONTROLLER_H

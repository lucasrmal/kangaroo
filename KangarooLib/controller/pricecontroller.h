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

#ifndef PRICECONTROLLER_H
#define PRICECONTROLLER_H

#include <QAbstractItemModel>
#include <QDate>

namespace KLib
{

    namespace PriceColumn
    {
        enum Columns
        {
            PAIR = 0,
            DATE,
            NumColumns
        };
    }

    namespace PriceType
    {
        enum Type
        {
            Currency,
            Security,
            NumTypes
        };
    }

    class ExchangePair;
    class PriceItem;

    class PriceController : public QAbstractItemModel
    {
        Q_OBJECT

        public:
            explicit        PriceController(QObject *parent = 0);

            virtual         ~PriceController();

            QVariant        data(const QModelIndex& _index, int _role) const override;
            Qt::ItemFlags   flags(const QModelIndex& _index) const override;
            QVariant        headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const override;

            QModelIndex     index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const override;
            QModelIndex     parent(const QModelIndex& _index) const override;

            int             rowCount(const QModelIndex& _parent = QModelIndex()) const override;
            int             columnCount(const QModelIndex& _parent = QModelIndex()) const override;

            bool            setData(const QModelIndex& _index, const QVariant& _value, int _role = Qt::EditRole) override;

        private slots:
            void onExchangePairAdded(KLib::ExchangePair* _p);
            void onExchangePairRemoved(KLib::ExchangePair* _p);

            void onRateSet(KLib::ExchangePair* _p, const QDate& _date);
            void onRateRemoved(KLib::ExchangePair* _p, const QDate& _date);

        private:
            void            loadData();

            PriceItem*      m_root;

    };

    class PriceItem
    {

    public:

        enum Type
        {
            Root,
            PriceTypes,
            Exchange,
            Rate
        };

        PriceItem(Type _itemType);
        ~PriceItem();

        void addToParent(PriceItem* parent);



        Type itemType;

        //////////////////
        ExchangePair* pair;
        ///////////////////
        PriceType::Type priceType;
        ///////////////////
        QDate date;
        double value;

        QList<PriceItem*> children;
        PriceItem* parent;
        int index;

    };

}

#endif // PRICECONTROLLER_H

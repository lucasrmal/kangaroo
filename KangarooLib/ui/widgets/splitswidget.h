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

#ifndef SPLITSWIDGET_H
#define SPLITSWIDGET_H

#include <QTableView>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include "../../model/transaction.h"

namespace KLib
{
    namespace SplitsColumn
    {
        enum Columns
        {
            MEMO = 0,
            TRANSFER,
            DEBIT,
            CREDIT,
            NumColumns
        };
    }

    class SplitsController : public QAbstractTableModel
    {
        Q_OBJECT

        public:
            SplitsController(QList<Transaction::Split>& _splits, bool _readOnly, QObject* _parent = 0);

            ~SplitsController() override {}

            int             rowCount(const QModelIndex& _parent = QModelIndex()) const override;
            int             columnCount(const QModelIndex& _parent = QModelIndex()) const override;

            QVariant        data(const QModelIndex& _index, int _role) const override;

            QVariant        headerData(int _section,
                                       Qt::Orientation _orientation,
                                       int _role = Qt::DisplayRole) const override;

            Qt::ItemFlags   flags(const QModelIndex& _index) const override;

            bool            setData(const QModelIndex& _index,
                                    const QVariant& _value,
                                    int _role = Qt::EditRole) override;

            bool            insertRows(int _row, int _count, const QModelIndex& _parent = QModelIndex()) override;

            bool            removeRows(int _row, int _count, const QModelIndex& _parent = QModelIndex()) override;

        public slots:
            void reload();

        private:
            bool rowIsEmpty(int _row);
            void ensureOneEmptyRow();

            QList<Transaction::Split>& m_splits;
            bool m_isReadOnly;
    };

    class SplitsWidgetDelegate : public QItemDelegate
    {
        Q_OBJECT

        public:
            SplitsWidgetDelegate(QObject *parent = 0);
            virtual ~SplitsWidgetDelegate() {}

            QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

            void setEditorData(QWidget *editor, const QModelIndex &index) const;
            void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

            void updateEditorGeometry(QWidget *editor,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const;
    };

    class SplitsWidget : public QTableView
    {
        Q_OBJECT

        public:
            explicit SplitsWidget(QList<Transaction::Split>& _splits, bool _readOnly, QWidget *parent = 0);
            virtual ~SplitsWidget() {}

            QSize	sizeHint () const { return QSize(700, 300); }

            bool   validate();

            QList<Transaction::Split> validSplits() const;

        public slots:
             void addRow();
             void removeCurrentRow();

             void reload();

        private slots:
             void onDataChanged(const QModelIndex& _from, const QModelIndex& _to);

        signals:
             void amountChanged();

        private:
            SplitsController* m_model;
            QList<Transaction::Split>& m_splits;

            SplitsWidgetDelegate m_delegate;
    };

}

#endif // SPLITSWIDGET_H

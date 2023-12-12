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

#ifndef BROKERAGELEDGERCONTROLLER_H
#define BROKERAGELEDGERCONTROLLER_H

#include "ledgercontroller.h"
#include "genericledgercontroller.h"

namespace KLib
{

    namespace BrokerageLedgerColumn
    {
        enum Columns
        {
            FLAG = 0,
            DATE,
            NO,
            ACTION,
            MEMO,
            CLEARED,
            SECURITY_TRANSFER,
            QUANTITY,
            PRICE,
            COMMISSION,
            CASH_CHANGE,
            CASH_BALANCE,
            NumColumns
        };
    }

    class BrokerageLedgerController : public LedgerController
    {
        Q_OBJECT

        protected:
            BrokerageLedgerController(Ledger* _ledger, LedgerBuffer* _buffer, QObject* _parent = 0);

        public:
            BrokerageLedgerController(Ledger* _ledger, QObject* _parent = 0);

            virtual ~BrokerageLedgerController() {}

            Qt::ItemFlags flags(const QModelIndex& _index) const override;

            void addExtraActions(LedgerWidget* _widget, std::function<void(int, LedgerAction*)> _addAction) override;

            QVariant        headerData(int _section,
                                       Qt::Orientation _orientation,
                                       int _role = Qt::DisplayRole) const override;

            int columnCount(const QModelIndex& _parent = QModelIndex()) const override;

            LedgerWidgetDelegate* buildDelegate(LedgerWidget* _widget) const;

            int col_status() const override      { return -1; }
            int col_flag() const override        { return BrokerageLedgerColumn::FLAG; }
            int col_no() const override          { return BrokerageLedgerColumn::NO; }
            int col_date() const override        { return BrokerageLedgerColumn::DATE; }
            int col_memo() const override        { return BrokerageLedgerColumn::MEMO; }
            int col_payee() const override       { return -1; }
            int col_cleared() const override     { return BrokerageLedgerColumn::CLEARED; }
            int col_transfer() const override    { return -1; }
            int col_debit() const override       { return -1; }
            int col_credit() const override      { return -1; }
            int col_balance() const override     { return BrokerageLedgerColumn::CASH_BALANCE; }
            int col_action() const override      { return -1; /* No action column. */ }

            bool alignRight(int _column) const override { return _column >= BrokerageLedgerColumn::QUANTITY; }

        protected:
            bool        canEditTransaction(const Transaction* _transaction, QString* _message = nullptr) const override;
            QVariant cacheData(int _column, int _cacheRow, int _row, bool _editRole) const override;

    };

    class BrokerageLedgerWidgetDelegate : public LedgerWidgetDelegate
    {
        Q_OBJECT

        public:
            BrokerageLedgerWidgetDelegate(const LedgerController* _controller, KLib::LedgerWidget* _widget) :
                LedgerWidgetDelegate(_controller, _widget) {}

//            QWidget* createEditor(QWidget* _parent, const QStyleOptionViewItem& _option, const QModelIndex& _index) const override;

//            void setEditorData(QWidget* _editor, const QModelIndex& _index) const override;
//            void setModelData(QWidget* _editor, QAbstractItemModel* _model, const QModelIndex &_index) const override;

            void setColumnWidth(LedgerWidget* _view) const override;
    };

}

#endif // BROKERAGELEDGERCONTROLLER_H

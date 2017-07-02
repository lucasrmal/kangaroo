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

#ifndef GENERICLEDGERCONTROLLER_H
#define GENERICLEDGERCONTROLLER_H

#include "ledgercontroller.h"

namespace KLib {

    namespace GenericLedgerColumn
    {
        enum Columns
        {
            STATUS = 0,
            FLAG,
            DATE,
            NO,
            MEMO,
            PAYEE,
            CLEARED,
            TRANSFER,
            DEBIT,
            CREDIT,
            BALANCE,
            NumColumns
        };
    }

    class GenericLedgerController : public LedgerController
    {
        Q_OBJECT

        protected:
            GenericLedgerController(Ledger* _ledger, LedgerBuffer* _buffer, QObject* _parent = nullptr);

        public:
            GenericLedgerController(Ledger* _ledger, QObject* _parent = nullptr);

            virtual ~GenericLedgerController() {}

            Qt::ItemFlags flags(const QModelIndex& _index) const override;

            void addExtraActions(LedgerWidget* _widget, std::function<void(int, LedgerAction*)> _addAction) override;

            int columnCount(const QModelIndex& _parent = QModelIndex()) const override;

            int col_status() const override      { return GenericLedgerColumn::STATUS; }
            int col_flag() const override        { return GenericLedgerColumn::FLAG; }
            int col_no() const override          { return GenericLedgerColumn::NO; }
            int col_date() const override        { return GenericLedgerColumn::DATE; }
            int col_memo() const override        { return GenericLedgerColumn::MEMO; }
            int col_payee() const override       { return GenericLedgerColumn::PAYEE; }
            int col_cleared() const override     { return GenericLedgerColumn::CLEARED; }
            int col_transfer() const override    { return GenericLedgerColumn::TRANSFER; }
            int col_debit() const override       { return GenericLedgerColumn::DEBIT; }
            int col_credit() const override      { return GenericLedgerColumn::CREDIT; }
            int col_balance() const override     { return GenericLedgerColumn::BALANCE; }

            QModelIndex     firstEditableIndex(const QModelIndex& _index) const override;
            QModelIndex     lastEditableIndex(const QModelIndex& _index) const override;

            bool alignRight(int _column) const override { return _column >= GenericLedgerColumn::DEBIT; }

        protected:
            bool canEditTransaction(const Transaction* _transaction, QString* _message = nullptr) const override;

    };

}

#endif // GENERICLEDGERCONTROLLER_H

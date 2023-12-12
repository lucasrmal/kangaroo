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

#ifndef INVESTMENTLEDGERCONTROLLER_H
#define INVESTMENTLEDGERCONTROLLER_H

#include "../../model/investmenttransaction.h"
#include "ledgercontroller.h"

namespace KLib
{
    class Currency;
    class Security;
    class GainLossData;

    namespace InvestmentLedgerColumn
    {
        enum Columns
        {
            STATUS = 0,
            FLAG,
            DATE,
            NO,
            MEMO,
            CLEARED,
            ACTION,
            TRANSFER,
            QUANTITY,
            PRICE,
            //COMMISSION,
            BALANCE,
            NumColumns
        };
    }

    class InvestmentLedgerBuffer : public LedgerBuffer
    {
        Q_OBJECT

        public:

            //Investment data
            InvestmentAction action;

            // Price per share or cost basis adjustment
            Amount      pricePerShare;
            Amount      quantity;
            Amount      fee;
            Amount      cashInLieu;
            Amount      gainLoss;
            Amount      swapTo;
            Lots        lots;

            int         idFeeAccount;
            int         idGainLossAccount;
            int         idTaxAccount;
            int         idCashInLieuAccount;
            int         idDivDistToAccount;

            //For distributions
            DistribComposition distribComposition;

            //For a split. First: new, second: old.
            SplitFraction splitFraction;

            //For cost basis adjustment, undistributed gain
            Amount basisAdjustment;
            Amount taxPaid;

            //Methods
            void clear() override;
            QVariant data(int _column, int _row, bool _editRole, const LedgerController* _controller) const override;
            bool setData(int _column, int _row, const QVariant& _value, LedgerController* _controller) override;
            void load(const Transaction* _transaction, const LedgerController* _controller) override;
            QStringList validate(int& _firstErrorColumn, const LedgerController* _controller) override;

            bool saveToTransaction(Transaction* _transaction, LedgerController* _controller) override;

            Transaction* createTransaction() const override { return new InvestmentTransaction(); }

//            bool save(LedgerController* _controller) override;

            void loadGainLossData(GainLossData& _data) const;


            int rowCount() const override;

            //These should not do anything.
            void changeToSplitTransaction(const LedgerController*) override {}
            void removeRowAt(int) override {}

            InvestmentLedgerBuffer(int _idSecurity);

        protected:
            void loadSplits(const QList<Transaction::Split>&, const LedgerController*) override {}

        private:
            void rowCountChanged(int _previous);
            void displayNetAmount();

            const Security* m_security;
    };

    class InvestmentLedgerController : public LedgerController
    {
        Q_OBJECT

        public:
            explicit InvestmentLedgerController(Ledger* _ledger, QObject* _parent = nullptr);

            Qt::ItemFlags flags(const QModelIndex& _index) const override;

            void addExtraActions(LedgerWidget* _widget, std::function<void(int, LedgerAction*)> _addAction) override;

            int columnCount(const QModelIndex& _parent = QModelIndex()) const override;

            QVariant        headerData(int _section,
                                       Qt::Orientation _orientation,
                                       int _role = Qt::DisplayRole) const override;

            Security*       security() const { return m_security; }
            Currency*       currency() const { return m_currency; }

            LedgerWidgetDelegate* buildDelegate(LedgerWidget* _widget) const;

            int col_status() const override      { return InvestmentLedgerColumn::STATUS; }
            int col_flag() const override        { return InvestmentLedgerColumn::FLAG; }
            int col_no() const override          { return InvestmentLedgerColumn::NO; }
            int col_date() const override        { return InvestmentLedgerColumn::DATE; }
            int col_memo() const override        { return InvestmentLedgerColumn::MEMO; }
            int col_payee() const override       { return -1; /* No payee column */ }
            int col_cleared() const override     { return InvestmentLedgerColumn::CLEARED; }
            int col_transfer() const override    { return InvestmentLedgerColumn::TRANSFER; }
            int col_debit() const override       { return -1; /* No debit column */ }
            int col_credit() const override      { return -1; /* No credit column */ }
            int col_balance() const override     { return InvestmentLedgerColumn::BALANCE; }
            int col_action() const override     { return InvestmentLedgerColumn::ACTION; }

            QModelIndex     firstEditableIndex(const QModelIndex& _index) const override;
            QModelIndex     lastEditableIndex(const QModelIndex& _index) const override;

            bool alignRight(int _column) const { return _column >= InvestmentLedgerColumn::QUANTITY; }

            static const QString INVALID_TRANSFER_TEXT;

            static int rowCountForAction(InvestmentAction _action);
            static QString labelForRow(int _row, InvestmentAction _action);

        public slots:
            void editSplits(const QModelIndex&) override {}

            void editDistributionComposition(const QList<int>& _rows);
            void openGainLossWizard(const QModelIndex& _index);

        protected slots:
            void onSecurityModified();

        protected:
            bool        canEditTransaction(const Transaction* _transaction, QString* _message = nullptr) const override;
            QVariant    cacheData(int _column, int _cacheRow, int _row, bool _editRole) const override;
            int         subRowCount(const Transaction* _tr) const override;
            Balances    cacheBalance(int _cacheRow) const override;

            InvestmentLedgerBuffer* m_invBuffer; ///< Simply a casted version of m_buffer

            Security* m_security;
            Currency* m_currency;

    };

    class InvestmentLedgerWidgetDelegate : public LedgerWidgetDelegate
    {
        Q_OBJECT

        public:
            InvestmentLedgerWidgetDelegate(const LedgerController* _controller, KLib::LedgerWidget* _widget) :
                LedgerWidgetDelegate(_controller, _widget) {}

            QWidget* createEditor(QWidget* _parent, const QStyleOptionViewItem& _option, const QModelIndex& _index) const override;

            void setEditorData(QWidget* _editor, const QModelIndex& _index) const override;
            void setModelData(QWidget* _editor, QAbstractItemModel* _model, const QModelIndex &_index) const override;

            void setColumnWidth(LedgerWidget* _view) const override;
    };

}

#endif // INVESTMENTLEDGERCONTROLLER_H

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

#ifndef SIMPLEACCOUNTCONTROLLER_H
#define SIMPLEACCOUNTCONTROLLER_H

#include <QAbstractItemModel>
#include "../amount.h"
#include "../model/ledger.h"

namespace KLib
{
    class Account;
    enum class AccountClassification;

    enum class SimpleAccountControllerColumn
    {
        NAME,
        BALANCE,

        NumColumns
    };

    /**
     * @brief The SimpleAccountController class
     *
     * This controller is similar to AccountController, but only shows a reduced
     * amount of information. Expense,Income and Trading account types are not
     * shown, and neither are placeholders (categories). The accounts are divided in
     * the following categories:
     * <ul>
     *  <li>CASH FLOW</li>
     *  <li>INVESTING</li>
     *  <li>PROPERTY</li>
     *  <li>OTHER</li>
     * </ul>
     *
     */
    class SimpleAccountController : public QAbstractItemModel
    {
        Q_OBJECT

        public:

            struct AccountNode
            {
                AccountNode(AccountClassification _cat, AccountNode* _parent) :
                    parent(_parent),
                    account(nullptr),
                    category(_cat)  {}

                AccountNode(Account* _account, AccountNode* _parent);

                ~AccountNode() { for (AccountNode* c : children) { delete c; } }

                QVariant data(int _column, int _role) const;
                void updateTotal();

                AccountNode* parent;
                QList<AccountNode*> children;

                Account* account;
                AccountClassification category;

                Amount categoryBalance;
            };

            explicit SimpleAccountController(QObject* _parent = nullptr);

            virtual ~SimpleAccountController() { delete m_root; }

            QVariant        data(const QModelIndex& _index, int _role) const override;
            QVariant        headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const override;

            QModelIndex     index(int _row, int _column, const QModelIndex& _parent = QModelIndex()) const override;
            QModelIndex     parent(const QModelIndex& _index) const override;

            Qt::ItemFlags   flags(const QModelIndex& _index) const override;

            int             rowCount(const QModelIndex& _parent = QModelIndex()) const override;
            int             columnCount(const QModelIndex& _parent = QModelIndex()) const override;

            bool            indexIsAfterTypeChange(const QModelIndex& _index) const;

            QModelIndex     indexForAccount(Account* _account) const;
            QModelIndex     indexForCategory(AccountClassification _cat) const;

            Account*        accountAt(const QModelIndex& _index) const;

            void            setShowClosedAccounts(bool _show);

        private slots:
            void onAccountAdded(KLib::Account* a);
            void onAccountRemoved(KLib::Account* a);
            void onAccountModified(KLib::Account* a);
            void onBalanceTodayChanged(int _idAccount, const Balances& _difference);

        private:
            void loadData();

            AccountNode* m_root;
            bool m_showClosedAccounts;

    };

}

#endif // SIMPLEACCOUNTCONTROLLER_H

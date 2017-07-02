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

#ifndef ACCOUNTSELECTOR_H
#define ACCOUNTSELECTOR_H

#include <QComboBox>
#include <QAbstractListModel>
#include <tuple>
#include "../../klib.h"
#include "../../model/account.h"

namespace KLib {

    class AccountSelectorModel;
    class Security;
    class Currency;

    enum AccountSelectorFlags
    {
        Flag_None               = 0x00,
        Flag_Placeholders       = 0x01,
        Flag_Closed             = 0x02,
        Flag_MultipleCurrencies = 0x04,

        Flag_PlaceholdersClosed = Flag_Placeholders | Flag_Closed,
        Flag_All = Flag_Placeholders | Flag_Closed | Flag_MultipleCurrencies
    };

    class AccountSelector : public QComboBox
    {
        Q_OBJECT

        public:

            explicit AccountSelector(int _displayFlags,
                                     int _typeFlags = AccountTypeFlags::Flag_All,
                                     int _idExceptAccount = Constants::NO_ID,
                                     QWidget *parent = nullptr);

            explicit AccountSelector(int _displayFlags,
                                     int _typeFlags,
                                     const QString& _cur,
                                     int _idExceptAccount = Constants::NO_ID,
                                     QWidget *parent = nullptr);

            explicit AccountSelector(int _displayFlags,
                                     const Security* _sec,
                                     int _idExceptAccount = Constants::NO_ID,
                                     QWidget *parent = nullptr);

            explicit AccountSelector(int _displayFlags,
                                     const Currency* _cur,
                                     int _idExceptAccount = Constants::NO_ID,
                                     QWidget *parent = nullptr);

            explicit AccountSelector(std::function<bool(const Account*)> _customFilter,
                                     QWidget *parent = nullptr);

            explicit AccountSelector(int _displayFlags, QWidget *parent = nullptr);

            virtual ~AccountSelector() {}

            void setCurrentAccount(int _id, const QString& _currency = QString());

            Account* currentAccount() const;
            QString currentCurrency() const;

            void setShowOnlyCurrency(const Currency* _cur, int _flags = AccountTypeFlags::Flag_All);
            void setShowOnlySecurity(const Security* _sec, int _flags = AccountTypeFlags::Flag_All);

        protected:
            void focusOutEvent(QFocusEvent* _e);

        signals:
            void currentAccountChanged(KLib::Account* _account);

        private slots:
            void onCurrentChanged(int _i);
            void onEditingFinished();

            void setCompletedIndex(const QString& _name);

        private:
            void init();
            AccountSelectorModel* m_model;

            int m_completedIndex;
    };

    class AccountSelectorModel : public QAbstractListModel
    {
        Q_OBJECT

        /** Display, Account, Currency (the latter empty if not multiple currencies) */
        typedef std::tuple<QString, Account*, QString> AccountPair;

        public:
            AccountSelectorModel(int _displayFlags,
                                 int _typeFlags = AccountTypeFlags::Flag_All,
                                 int _idSecurity = Constants::NO_ID,
                                 const QString& _currency = QString(),
                                 int _idExceptAccount = Constants::NO_ID,
                                 std::function<bool(const Account*)> _customFilter = std::function<bool(const Account*)>(),
                                 QObject* _parent = nullptr);

            QVariant    data(const QModelIndex& _index, int _role) const;

            int         rowCount(const QModelIndex&) const  { return m_accounts.count(); }

            Account*    accountAt(int _row) const           { return std::get<1>(m_accounts[_row]); }
            QString     currencyAt(int _row) const          { return std::get<2>(m_accounts[_row]); }
            int         indexFor(const QString _display) const;

        private:
            void loadData();
            void rec_load(Account* _parent, const QString &_prefix);

            int     m_displayFlags;
            int     m_typeFlags;
            int     m_idSecurity;
            QString m_currency;
            int     m_idExceptAccount;
            std::function<bool(const Account*)> m_filter;

            QList<AccountPair> m_accounts;
            QHash<QString, int> m_index;
            QHash<int, int> m_accountIndex; ///< First: account ID, second: first index of this account in m_accounts

            friend class AccountSelector;

    };



}

#endif // ACCOUNTSELECTOR_H

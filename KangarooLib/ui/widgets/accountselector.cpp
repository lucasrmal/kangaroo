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

#include "accountselector.h"
#include "../../model/account.h"
#include "../../model/security.h"
#include "../../model/currency.h"

#include <QCompleter>
#include <QLineEdit>
#include <QMessageBox>
#include <QAbstractItemView>
#include <QtDebug>

namespace KLib {

    AccountSelector::AccountSelector(int _displayFlags, int _typeFlags, int _idExceptAccount, QWidget *parent) :
        QComboBox(parent),
        m_model(new AccountSelectorModel(_displayFlags,
                                         _typeFlags,
                                         Constants::NO_ID,
                                         QString(),
                                         _idExceptAccount,
                                         std::function<bool(const Account*)>(),
                                         this))
    {
        init();
    }

    AccountSelector::AccountSelector(int _displayFlags, int _typeFlags, const QString& _cur, int _idExceptAccount, QWidget *parent) :
        QComboBox(parent),
        m_model(new AccountSelectorModel(_displayFlags,
                                         _typeFlags,
                                         Constants::NO_ID,
                                         _cur,
                                         _idExceptAccount,
                                         std::function<bool(const Account*)>(),
                                         this))
    {
        init();
    }

    AccountSelector::AccountSelector(int _displayFlags, const Security* _sec, int _idExceptAccount, QWidget *parent) :
        QComboBox(parent),
        m_model(new AccountSelectorModel(_displayFlags,
                                         AccountTypeFlags::Flag_Investment | AccountTypeFlags::Flag_Trading,
                                          _sec ? _sec->id() : Constants::NO_ID,
                                         QString(),
                                         _idExceptAccount,
                                         std::function<bool(const Account*)>(),
                                         this))
    {
        init();
    }

    AccountSelector::AccountSelector(int _displayFlags, const Currency* _cur, int _idExceptAccount, QWidget *parent) :
        QComboBox(parent),
        m_model(new AccountSelectorModel(_displayFlags,
                                         AccountTypeFlags::Flag_Investment | AccountTypeFlags::Flag_Trading,
                                         Constants::NO_ID,
                                          _cur ? _cur->code() : QString(),
                                         _idExceptAccount,
                                         std::function<bool(const Account*)>(),
                                         this))
    {
        init();
    }

    AccountSelector::AccountSelector(int _displayFlags, QWidget *parent) :
        QComboBox(parent),
        m_model(new AccountSelectorModel(_displayFlags,
                                         AccountTypeFlags::Flag_All,
                                         Constants::NO_ID,
                                         QString(),
                                         Constants::NO_ID,
                                         std::function<bool(const Account*)>(),
                                         this))
    {
        init();
    }

    AccountSelector::AccountSelector(std::function<bool(const Account*)> _customFilter, QWidget *parent) :
        QComboBox(parent),
        m_model(new AccountSelectorModel(Flag_All,
                                         AccountTypeFlags::Flag_All,
                                         Constants::NO_ID,
                                         QString(),
                                         Constants::NO_ID,
                                         _customFilter,
                                         this))
    {
        init();
    }

    void AccountSelector::init()
    {
        m_completedIndex = Constants::NO_ID;

        setModel(m_model);
        setEditable(true);

        QCompleter* completer = new QCompleter(m_model, this);
        completer->setCaseSensitivity(Qt::CaseInsensitive);
        completer->setFilterMode(Qt::MatchContains);
        setCompleter(completer);

        setCurrentText("");

        //Widths of views
        int width = minimumSizeHint().width();
        view()->sizePolicy().setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        view()->setMinimumWidth(width);

        completer->popup()->sizePolicy().setHorizontalPolicy(QSizePolicy::MinimumExpanding);
        completer->popup()->setMinimumWidth(width);

        //Connections
        connect(completer,          static_cast<void (QCompleter::*)(const QString&)>(&QCompleter::highlighted),
                this,               &AccountSelector::setCompletedIndex);

        connect(this,               static_cast<void (AccountSelector::*)(int)>(&AccountSelector::currentIndexChanged),
                this,               &AccountSelector::onCurrentChanged);

        connect(this->lineEdit(),   &QLineEdit::editingFinished,
                this,               &AccountSelector::onEditingFinished);
    }

    void AccountSelector::setCompletedIndex(const QString& _name)
    {
        int row = m_model->indexFor(_name);

        if (row != -1)
        {
            setCurrentIndex(row);
        }

    }

    void AccountSelector::onEditingFinished()
    {
        emit currentAccountChanged(currentAccount());
    }

    void AccountSelector::setCurrentAccount(int _id, const QString& _currency)
    {
        int index = m_model->m_accountIndex.value(_id, Constants::NO_ID);

        if (index != Constants::NO_ID)
        {
            if (!_currency.isEmpty())
            {
                while (index+1 < m_model->m_accounts.count()
                       && std::get<1>(m_model->m_accounts[index+1])->id() == _id
                       && std::get<2>(m_model->m_accounts[index]) != _currency)
                {
                    ++index;
                }
            }

            setCurrentIndex(index);
        }
        else
        {
            setCurrentIndex(0);
        }
    }

    void AccountSelector::onCurrentChanged(int)
    {
        emit currentAccountChanged(currentAccount());
    }

    Account* AccountSelector::currentAccount() const
    {
        int row = m_model->indexFor(currentText());

        return row != -1 ? m_model->accountAt(row)
                         : nullptr;
    }

    QString AccountSelector::currentCurrency() const
    {
        int row = m_model->indexFor(currentText());

        return row != -1 ? m_model->currencyAt(row)
                         : nullptr;
    }

    void AccountSelector::setShowOnlyCurrency(const Currency* _cur, int _flags)
    {
        if (_cur)
        {
            m_model->m_currency = _cur->code();
            m_model->m_idSecurity = Constants::NO_ID;
        }
        else
        {
            m_model->m_currency.clear();
        }

        m_model->m_typeFlags = _flags;
        m_model->loadData();
        view()->reset();
    }

    void AccountSelector::setShowOnlySecurity(const Security* _sec, int _flags)
    {
        if (_sec)
        {
            m_model->m_currency.clear();
            m_model->m_idSecurity = _sec->id();
        }
        else
        {
            m_model->m_idSecurity = Constants::NO_ID;
        }

        m_model->m_typeFlags = _flags;
        m_model->loadData();
        view()->reset();
    }

    void AccountSelector::focusOutEvent(QFocusEvent* _e)
    {
      qDebug() << "Focus out AccountSelector";
        QComboBox::focusOutEvent(_e);
    }

    AccountSelectorModel::AccountSelectorModel(int _displayFlags,
                                               int _typeFlags,
                                               int _idSecurity,
                                               const QString& _currency,
                                               int _idExceptAccount,
                                               std::function<bool(const Account*)> _customFilter,
                                               QObject* _parent) :
          QAbstractListModel(_parent),
          m_displayFlags(_displayFlags),
          m_typeFlags(_typeFlags),
          m_idSecurity(_idSecurity),
          m_currency(_currency),
          m_idExceptAccount(_idExceptAccount),
          m_filter(_customFilter)
    {
        //Create the filter lambda
        if (!m_filter)
        {
            m_filter = [this] (const Account* a) {
                return !a->id() != m_idExceptAccount
                    && (a->isOpen() || m_displayFlags & Flag_Closed)
                    && (!a->isPlaceholder() || m_displayFlags & Flag_Placeholders)
                    && (m_idSecurity == Constants::NO_ID || m_idSecurity == a->idSecurity())
                    && (m_currency.isEmpty() || a->allCurrencies().contains(m_currency))
                    && Account::matchesFlags(a, m_typeFlags);
            };
        }

        loadData();
    }

    QVariant AccountSelectorModel::data(const QModelIndex& _index, int _role) const
    {
        if (!_index.isValid())
        {
            return QVariant();
        }

        if (_role == Qt::DisplayRole || _role == Qt::EditRole)
        {
            return std::get<0>(m_accounts[_index.row()]);
        }
        else if (_role == Qt::UserRole)
        {
            Account* a = std::get<1>(m_accounts[_index.row()]);
            return a ? a->id() : Constants::NO_ID;
        }
        else
        {
            return QVariant();
        }
    }

    void AccountSelectorModel::loadData()
    {
        m_accounts.clear();

        //Add a empty account
        m_index[QString()] = 0;
        m_accounts << std::make_tuple(QString(), nullptr, QString());

        //Load accounts
        rec_load(Account::getTopLevel(), "");
    }

    int AccountSelectorModel::indexFor(const QString _display) const
    {
        return m_index.contains(_display) ? m_index[_display]
                                          : -1;
    }



    void AccountSelectorModel::rec_load(Account* _current, const QString& _prefix)
    {        
        auto displayWithPrefix = [&_prefix] (const QString& disp)
        {
            return _prefix.isEmpty() ? disp
                                     : _prefix + Account::PATH_SEPARATOR + disp;
        };

        //Show the account if the conditions are OK
        if (m_filter(_current))
        {
            //If account is multi currency and multi currency enabled...
            if (_current->allCurrencies().count() > 1 && m_displayFlags & Flag_MultipleCurrencies)
            {
                for (const QString& cur : _current->allCurrencies())
                {
                    if (m_currency.isEmpty() || m_currency == cur)
                    {
                        QString display = displayWithPrefix(QString("%1 (%2)").arg(_current->name()).arg(cur));

                        m_index[display] = m_accounts.count();

                        if (!m_accountIndex.contains(_current->id()))
                        {
                            m_accountIndex[_current->id()] = m_accounts.count();
                        }

                        m_accounts << std::make_tuple(display, _current, cur);
                    }
                }
            }
            else
            {
                QString display = displayWithPrefix(_current->name());

                m_accountIndex[_current->id()] = m_accounts.count();
                m_index[display] = m_accounts.count();
                m_accounts << std::make_tuple(display, _current, _current->mainCurrency());
            }
        }

        //Recurse for all children
        for (Account* c : _current->getChildren())
        {
            if (_current->type() != AccountType::TOPLEVEL)
            {
                if (_prefix.isEmpty())
                {
                    rec_load(c, _current->name());
                }
                else
                {
                    rec_load(c, _prefix + Account::PATH_SEPARATOR + _current->name());
                }
            }
            else
            {
                rec_load(c, "");
            }

        }
    }

}


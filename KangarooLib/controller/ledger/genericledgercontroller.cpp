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

#include "genericledgercontroller.h"
#include "../../model/investmenttransaction.h"
#include "../../model/ledger.h"
#include "../../model/account.h"
#include "../../ui/widgets/ledgerwidget.h"
#include "../../ui/dialogs/formmultiinvestmententry.h"
#include "../../ui/core.h"
#include <QAction>

namespace KLib {




    GenericLedgerController::GenericLedgerController(Ledger* _ledger, LedgerBuffer* _buffer, QObject* _parent)
        : LedgerController(_ledger,
                           _buffer,
                           _parent)
    {
    }

    GenericLedgerController::GenericLedgerController(Ledger* _ledger, QObject *_parent)
            : LedgerController(_ledger,
                               new LedgerBuffer(),
                               _parent)
    {
    }

    int GenericLedgerController::columnCount(const QModelIndex& _parent) const
    {
        Q_UNUSED(_parent)
        return GenericLedgerColumn::NumColumns;
    }

    bool GenericLedgerController::canEditTransaction(const Transaction* _transaction, QString* _message) const
    {
        if (qobject_cast<const InvestmentTransaction*>(_transaction))
        {
            if (_message) *_message = tr("Investment transactions must be modified from their Investment Ledger");
            return false;
        }
        else
        {
            return true;
        }
    }

    Qt::ItemFlags GenericLedgerController::flags(const QModelIndex& _index) const
    {
        const int mainRow = mapToCacheRow(_index);

        if (!_index.isValid() || !m_ledger->account()->isOpen())
            return QAbstractItemModel::flags(_index);

        if (!canEdit(mainRow))
        {
            return QAbstractItemModel::flags(_index);
        }

        if (_index.column() == col_balance()            // Balance column
            || (_index.parent().isValid()               // Splits can only edit transfer, debit or credit.
                && _index.column() != col_transfer()
                && _index.column() != col_debit()
                && _index.column() != col_credit())
            || (_index.column() == col_status()         // Status, Flag and Cleared cannot be edited
                || _index.column() == col_flag()
                || _index.column() == col_cleared())
            || (_index.column() == col_transfer()       // If split trans, current transfer account cannot
                && !_index.parent().isValid()           // be edited for the first split
                && _index.model()->rowCount(_index) > 0))
        {
            return QAbstractItemModel::flags(_index);
        }
        else
        {
            return QAbstractItemModel::flags(_index) | Qt::ItemIsEditable;
        }
    }

    QModelIndex GenericLedgerController::firstEditableIndex(const QModelIndex& _index) const
    {
        if (_index.isValid())
        {
            return _index.parent().isValid() ? index(_index.row(), GenericLedgerColumn::TRANSFER, _index.parent())
                                             : index(_index.row(), GenericLedgerColumn::DATE);
        }
        else
        {
            return _index;
        }
    }

    QModelIndex GenericLedgerController::lastEditableIndex(const QModelIndex& _index) const
    {
        if (_index.isValid())
        {
            return index(_index.row(), GenericLedgerColumn::CREDIT, _index.parent());
        }
        else
        {
            return _index;
        }
    }

    void GenericLedgerController::addExtraActions(LedgerWidget* _widget, std::function<void(int, LedgerAction*)> _addAction)
    {
        LedgerAction* actSplits = new LedgerAction(new QAction(Core::icon("split"), tr("Make &Split"), this), true,
        [this, _widget](const QList<int>&)
        {
            return account()->isOpen()
                    && _widget->selectedRows().count() == 1;
        },
        [this, _widget]()
        {
            if (_widget->selectedRows().count() != 1)
            {
                return;
            }

            editSplits(_widget->currentIndex());
        });

        LedgerAction* actRemoveSplit = new LedgerAction(new QAction(Core::icon("remove-split"), tr("&Remove Split"), this), false,
        [this, _widget](const QList<int>&)
        {
            return account()->isOpen()
                    && hasModifiedRow()
                    && _widget->currentIndex().parent().isValid();
        },
        [this, _widget]()
        {
            if (hasModifiedRow() && canRemoveSplitAt(_widget->currentIndex()))
            {
                _widget->commitAndCloseEditor();
                removeSplitAt(_widget->currentIndex());
            }
        });

        _addAction(50, actSplits);
        _addAction(60, actRemoveSplit);

        if (account()->type() == AccountType::BROKERAGE)
        {
          LedgerAction* actMultiEntry = new LedgerAction(new QAction(Core::icon("brokerage-account"),
                                                                     tr("Multi Entry"), this), true,
          [this](const QList<int>&) { return account()->isOpen(); },
          [this, _widget]()
          {
            FormMultiInvestmentEntry form(account()->id(), _widget);
            form.exec();
          });

          _addAction(70, actMultiEntry);
        }
    }

}

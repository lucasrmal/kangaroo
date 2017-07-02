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

#include "brokerageledgercontroller.h"
#include "../../ui/core.h"
#include "../../model/ledger.h"
#include "../../model/account.h"
#include "../../ui/widgets/ledgerwidget.h"
#include "../../ui/dialogs/formmultiinvestmententry.h"

#include <QAction>
#include <QHeaderView>

namespace KLib
{

    BrokerageLedgerController::BrokerageLedgerController(Ledger* _ledger, LedgerBuffer* _buffer, QObject* _parent)
        : LedgerController(_ledger,
                           _buffer,
                           _parent)
    {
    }

    BrokerageLedgerController::BrokerageLedgerController(Ledger* _ledger, QObject *_parent)
            : LedgerController(_ledger,
                               new LedgerBuffer(),
                               _parent)
    {
    }

    int BrokerageLedgerController::columnCount(const QModelIndex& _parent) const
    {
        Q_UNUSED(_parent)
        return BrokerageLedgerColumn::NumColumns;
    }

    QVariant BrokerageLedgerController::headerData(int _section, Qt::Orientation _orientation, int _role) const
    {
        if (_orientation == Qt::Horizontal
            && _role == Qt::DisplayRole)
        {
            switch (_section)
            {
            case BrokerageLedgerColumn::ACTION:
                return tr("Action");

            case BrokerageLedgerColumn::SECURITY_TRANSFER:
                return tr("Security/Transfer");

            case BrokerageLedgerColumn::QUANTITY:
                return tr("Quantity");

            case BrokerageLedgerColumn::PRICE:
                return tr("Price");

            case BrokerageLedgerColumn::COMMISSION:
                return tr("Comm");

            case BrokerageLedgerColumn::CASH_CHANGE:
                return tr("Net");

            case BrokerageLedgerColumn::CASH_BALANCE:
                return tr("Cash Balance");

            default:
                break;
            }
        }

        return LedgerController::headerData(_section, _orientation, _role);
    }

    LedgerWidgetDelegate* BrokerageLedgerController::buildDelegate(LedgerWidget* _widget) const
    {
        return new BrokerageLedgerWidgetDelegate(this, _widget);
    }

    bool BrokerageLedgerController::canEditTransaction(const Transaction*, QString*) const
    {
        return true;
    }

    QVariant BrokerageLedgerController::cacheData(int _column, int _cacheRow, int _row, bool _editRole) const
    {
        switch (_column)
        {
        case BrokerageLedgerColumn::ACTION:
        case BrokerageLedgerColumn::SECURITY_TRANSFER:
        case BrokerageLedgerColumn::QUANTITY:
        case BrokerageLedgerColumn::PRICE:
        case BrokerageLedgerColumn::COMMISSION:
        case BrokerageLedgerColumn::CASH_CHANGE:
            return QVariant();

        default:
            return LedgerController::cacheData(_column, _cacheRow, _row, _editRole);
        }
    }

    void BrokerageLedgerController::addExtraActions(LedgerWidget* _widget,
                                                    std::function<void(int, LedgerAction*)> _addAction)
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

    Qt::ItemFlags BrokerageLedgerController::flags(const QModelIndex& _index) const
    {
        if (!_index.isValid() || !m_ledger->account()->isOpen())
            return QAbstractItemModel::flags(_index);

//        if (_index.row() != newTransactionRow()
//            && m_cache[_index.row()].transaction()->type() != TransactionType::Standard)
//        {
//            return QAbstractItemModel::flags(_index);
//        }

//        if (_index.column() == col_balance())
//        {
//            return QAbstractItemModel::flags(_index);
//        }
//        else if (_index.column() == col_transfer() ||
//                 _index.column() == col_debit() ||
//                 _index.column() == col_credit())
//        {
//            if (m_buffer->row == _index.row())
//            {
//                if (!m_buffer->splits.isEmpty() && !m_buffer->multiCurrency)
//                {
//                    return QAbstractItemModel::flags(_index);
//                }
//            }
//            else if (_index.row() != newTransactionRow())
//            {
//                if (m_cache[_index.row()].transaction()->splitCount() != 2 &&
//                    !m_cache[_index.row()].transaction()->isCurrencyExchange())
//                {
//                    return QAbstractItemModel::flags(_index);
//                }
//            }
//        }
//        else if (_index.column() == col_status()
//                 || _index.column() == col_flag()
//                 || _index.column() == col_cleared())
//        {
//            return QAbstractItemModel::flags(_index);
//        }

        return QAbstractItemModel::flags(_index) | Qt::ItemIsEditable;
    }

    //--------------------------------------------- DELEGATE ---------------------------------------------

    void BrokerageLedgerWidgetDelegate::setColumnWidth(LedgerWidget* _view) const
    {

        _view->setColumnWidth(m_controller->col_flag(), 26);
        _view->setColumnWidth(m_controller->col_no(), 60);
        _view->setColumnWidth(m_controller->col_date(), 100);
        _view->setColumnWidth(BrokerageLedgerColumn::ACTION, 100);
        _view->setColumnWidth(m_controller->col_memo(), 150);
        _view->setColumnWidth(m_controller->col_cleared(), 26);
        _view->setColumnWidth(BrokerageLedgerColumn::SECURITY_TRANSFER, 200);
        _view->setColumnWidth(BrokerageLedgerColumn::QUANTITY, 90);
        _view->setColumnWidth(BrokerageLedgerColumn::PRICE, 90);
        _view->setColumnWidth(BrokerageLedgerColumn::COMMISSION, 90);
        _view->setColumnWidth(BrokerageLedgerColumn::CASH_CHANGE, 90);
        _view->setColumnWidth(m_controller->col_balance(), 90);

        _view->header()->setSectionResizeMode(m_controller->col_memo(), QHeaderView::Stretch);
    }

}

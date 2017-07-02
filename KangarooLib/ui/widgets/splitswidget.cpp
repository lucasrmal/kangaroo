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

#include "splitswidget.h"
#include "../../model/account.h"
#include "amountedit.h"
#include "accountselector.h"
#include "../../model/modelexception.h"

#include <QHeaderView>
#include <QMessageBox>

namespace KLib {

SplitsController::SplitsController(QList<Transaction::Split>& _splits, bool _readOnly, QObject* _parent)
    : QAbstractTableModel(_parent),
      m_splits(_splits),
      m_isReadOnly(_readOnly)
{
    ensureOneEmptyRow();
}

void SplitsController::reload()
{
    beginResetModel();
    ensureOneEmptyRow();
    endResetModel();
}

int SplitsController::rowCount(const QModelIndex& _parent) const
{
    Q_UNUSED(_parent)
    return m_splits.count();
}

int SplitsController::columnCount(const QModelIndex& _parent) const
{
    Q_UNUSED(_parent)
    return SplitsColumn::NumColumns;
}

QVariant SplitsController::data(const QModelIndex& _index, int _role) const
{
    if (_role != Qt::EditRole && _role != Qt::DisplayRole)
        return QVariant();

    if (_index.isValid() && _index.row() < rowCount())
    {
        Transaction::Split s = m_splits[_index.row()];

        switch (_index.column())
        {
        case SplitsColumn::MEMO:
            return s.memo;

        case SplitsColumn::TRANSFER:
            if (_role == Qt::DisplayRole)
            {
                if (s.idAccount == Account::getTopLevel()->id())
                    return "";
                else
                    return Account::getTopLevel()->getPath(s.idAccount);
            }
            else
            {
                return s.idAccount;
            }
        case SplitsColumn::DEBIT:
            return s.amount > 0 ? s.amount.toString() : "";

        case SplitsColumn::CREDIT:
            return s.amount < 0 ? (-s.amount).toString() : "";

        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant SplitsController::headerData(int _section,
                                      Qt::Orientation _orientation,
                                      int _role) const
{
    if (_role != Qt::DisplayRole)
        return QVariant();

    if (_orientation == Qt::Horizontal)
    {
        switch (_section)
        {
        case SplitsColumn::MEMO:
            return tr("Description");

        case SplitsColumn::TRANSFER:
            return tr("Transfer");

        case SplitsColumn::DEBIT:
            return tr("Debit");

        case SplitsColumn::CREDIT:
            return tr("Credit");

        default:
            return "";
        }
    }
    else
    {
        return QString("%1").arg(_section+1);
    }
}

Qt::ItemFlags SplitsController::flags(const QModelIndex& _index) const
{
    if (_index.isValid() && !m_isReadOnly)
    {
        return QAbstractTableModel::flags(_index) | Qt::ItemIsEditable;
    }
    else
    {
        return QAbstractTableModel::flags(_index);
    }
}

bool SplitsController::setData(const QModelIndex& _index,
                               const QVariant& _value,
                               int _role)
{
    if (_index.isValid() && _index.row() < rowCount() && _role == Qt::EditRole)
    {
        switch (_index.column())
        {
        case SplitsColumn::MEMO:
            m_splits[_index.row()].memo = _value.toString();
            break;

        case SplitsColumn::TRANSFER:
            m_splits[_index.row()].idAccount = _value.toInt();


            try
            {
                m_splits[_index.row()].currency = Account::getTopLevel()->account(_value.toInt())->mainCurrency();
            }
            catch (...) {}

            break;

        case SplitsColumn::DEBIT:
        {
            Amount a = Amount::fromUserLocale(_value.toString());

            if (a == 0 && m_splits[_index.row()].amount < 0) // Check if there was data in the Credit column
            {
                return false;
            }

            m_splits[_index.row()].amount = a;
            break;
        }
        case SplitsColumn::CREDIT:
        {
            Amount a = Amount::fromUserLocale(_value.toString());

            if (a == 0 && m_splits[_index.row()].amount > 0) // Check if there was data in the Debit column
            {
                return false;
            }

            m_splits[_index.row()].amount = -a;
            break;
        }

        default:
            return false;
        }
    }

    emit dataChanged(_index, _index);

    //If editing last row and is not empty, we add one
    if (_index.row() == rowCount()-1 && !rowIsEmpty(_index.row()))
    {
        insertRows(rowCount(), 1);
    }
    else if (rowIsEmpty(rowCount()-1) && rowIsEmpty(rowCount()-2))
    {
        //If the two last rows are empty, remove one.
        removeRows(rowCount()-1, 1);
    }
    return true;
}

bool SplitsController::rowIsEmpty(int _row)
{
    if (_row < 0 || _row >= rowCount())
        return false;

    return m_splits[_row].amount == 0
           && m_splits[_row].memo.isEmpty()
           && m_splits[_row].idAccount == Constants::NO_ID;
}

void SplitsController::ensureOneEmptyRow()
{
    if (m_splits.count() == 0 || !rowIsEmpty(rowCount()-1))
    {
        insertRows(rowCount(), 1);
    }
}

bool SplitsController::insertRows(int _row, int _count, const QModelIndex& _parent)
{
    beginInsertRows(_parent, _row, _row+_count-1);
    for (int i = 0; i < _count; ++i)
    {
        m_splits.insert(_row, Transaction::Split());
    }
    endInsertRows();

    return true;
}

bool SplitsController::removeRows(int _row, int _count, const QModelIndex& _parent)
{
    beginRemoveRows(_parent, _row, _row+_count-1);
    for (int i = 0; i < _count; ++i)
    {
        m_splits.removeAt(_row);
    }
    endRemoveRows();

    return true;
}

// ####################################################################################

SplitsWidgetDelegate::SplitsWidgetDelegate(QObject* _parent) :
    QItemDelegate(_parent)
{
}

QWidget* SplitsWidgetDelegate::createEditor(QWidget* _parent,
                                            const QStyleOptionViewItem& _option,
                                            const QModelIndex& _index) const
{
    switch (_index.column())
    {
    case SplitsColumn::DEBIT:
    case SplitsColumn::CREDIT:
    {
        AmountEdit* amtEdit = new AmountEdit(2, _parent);
        amtEdit->setMinimum(0);
        amtEdit->setFocus();
        return amtEdit;
    }
    case SplitsColumn::TRANSFER:
    {
        return new AccountSelector(Flag_MultipleCurrencies,
                                   AccountTypeFlags::Flag_All & ~AccountTypeFlags::Flag_Investment,
                                   Constants::NO_ID,
                                   _parent);
    }
    default:
        return QItemDelegate::createEditor(_parent, _option, _index);
    }
}

void SplitsWidgetDelegate::setEditorData(QWidget* _editor, const QModelIndex& _index) const
{
    switch (_index.column())
    {
    case SplitsColumn::DEBIT:
    case SplitsColumn::CREDIT:
    {
        Amount value = Amount::fromUserLocale(_index.model()->data(_index, Qt::EditRole).toString());
        AmountEdit* amtEdit =  static_cast<AmountEdit*>(_editor);
        amtEdit->setAmount(value);
        break;
    }
    case SplitsColumn::TRANSFER:
    {
        int idAccount = _index.model()->data(_index, Qt::EditRole).toInt();
        AccountSelector* accSelect =  static_cast<AccountSelector*>(_editor);
        accSelect->setCurrentAccount(idAccount);
        break;
    }
    default:
        return QItemDelegate::setEditorData(_editor, _index);
    }
}
void SplitsWidgetDelegate::setModelData(QWidget* _editor,
                                        QAbstractItemModel* _model,
                                        const QModelIndex& _index) const
{
    switch (_index.column())
    {
    case SplitsColumn::DEBIT:
    case SplitsColumn::CREDIT:
    {
        AmountEdit* amtEdit =  static_cast<AmountEdit*>(_editor);
        _model->setData(_index, amtEdit->amount().toString(), Qt::EditRole);
        break;
    }
    case SplitsColumn::TRANSFER:
    {
        AccountSelector* accSelect =  static_cast<AccountSelector*>(_editor);
        _model->setData(_index, accSelect->currentData(), Qt::EditRole);
        break;
    }
    default:
        return QItemDelegate::setModelData(_editor,_model, _index);
    }
}

void SplitsWidgetDelegate::updateEditorGeometry(QWidget* _editor,
                                                const QStyleOptionViewItem& _option,
                                                const QModelIndex& _index) const
{
    Q_UNUSED(_index)
    _editor->setGeometry(_option.rect);
}

/* #################################################################################### */

SplitsWidget::SplitsWidget(QList<Transaction::Split>& _splits, bool _readOnly, QWidget *parent) :
    QTableView(parent),
    m_model(new SplitsController(_splits, _readOnly, this)),
    m_splits(_splits)
{
    setModel(m_model);
    setItemDelegate(&m_delegate);

    setColumnWidth(SplitsColumn::MEMO, 200);
    setColumnWidth(SplitsColumn::TRANSFER, 220);
    setColumnWidth(SplitsColumn::DEBIT, 100);
    setColumnWidth(SplitsColumn::CREDIT, 100);
    horizontalHeader()->setSectionResizeMode(SplitsColumn::MEMO, QHeaderView::Stretch);

    setSelectionBehavior(QAbstractItemView::SelectRows);

//    if (m_splits.count() < 5)
//    {
//        m_model->insertRows(m_model->rowCount(), 5-m_splits.count());
//    }

    setAlternatingRowColors(true);

    connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(onDataChanged(QModelIndex,QModelIndex)));
}

void SplitsWidget::onDataChanged(const QModelIndex& _from, const QModelIndex& _to)
{
    if (_from.column() <= (int) SplitsColumn::CREDIT && _to.column() >= (int) SplitsColumn::DEBIT)
    {
        emit amountChanged();
    }
}

QList<Transaction::Split> SplitsWidget::validSplits() const
{
    QList<Transaction::Split> notEmpty;

    for (Transaction::Split s : m_splits)
    {
        if (s.amount != 0)
        {
            notEmpty << s;
        }
    }

    return notEmpty;
}

bool SplitsWidget::validate()
{
    QList<Transaction::Split> notEmpty;
    QString msgBoxTitle = tr("Save Splits");

    for (Transaction::Split s : m_splits)
    {
        if (s.idAccount != Constants::NO_ID || s.amount != 0 || !s.memo.isEmpty())
        {
            if (s.idAccount == Constants::NO_ID)
            {
                QMessageBox::information(this, msgBoxTitle, tr("Please select an account for this split."));
                edit(m_model->index(currentIndex().row(), SplitsColumn::TRANSFER));
                return false;
            }
            else if (s.amount == 0)
            {
                QMessageBox::information(this, msgBoxTitle, tr("Please enter an amount for this split"));
                edit(m_model->index(currentIndex().row(), SplitsColumn::DEBIT));
                return false;
            }

            notEmpty << s;
        }
    }

    // Check if it balances
    try
    {
        if (notEmpty.count() < 2)
        {
            QMessageBox::information(this,msgBoxTitle, tr("There must be at least two splits."));
            return false;
        }

        if (!Transaction::splitsBalance(notEmpty))
        {
            QMessageBox::information(this,msgBoxTitle, tr("The splits do not balance. Debits must equal credits."));
            return false;
        }
    }
    catch (ModelException)
    {
        QMessageBox::information(this, msgBoxTitle,
                                 tr("One or more transfer account is invalid."));
        return false;
    }

    m_splits = notEmpty;
    return true;
}

void SplitsWidget::addRow()
{
    m_model->insertRow(m_model->rowCount());
}

void SplitsWidget::removeCurrentRow()
{
    m_model->removeRow(currentIndex().row());
}

void SplitsWidget::reload()
{
    m_model->reload();
}

}

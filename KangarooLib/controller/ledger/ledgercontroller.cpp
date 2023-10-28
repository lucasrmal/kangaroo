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

#include "ledgercontroller.h"
#include "../../model/account.h"
#include "../../model/currency.h"
#include "../../model/ledger.h"
#include "../../model/modelexception.h"
#include "../../model/payee.h"
#include "../../model/security.h"
#include "../../ui/core.h"
#include "../../ui/dialogs/formcurrencyexchange.h"
#include "../../ui/dialogs/formeditattachments.h"
#include "../../ui/dialogs/formeditschedule.h"
#include "../../ui/dialogs/optionsdialog.h"
#include "../../ui/dialogs/spliteditor.h"
#include "../../ui/mainwindow.h"
#include "../../ui/settingsmanager.h"
#include "../../ui/widgets/accountselector.h"
#include "../../ui/widgets/amountedit.h"
#include "../../ui/widgets/ledgerwidget.h"
#include "../payeecontroller.h"

#include <QComboBox>
#include <QCompleter>
#include <QDateEdit>
#include <QDebug>
#include <QHeaderView>
#include <QLinkedList>
#include <QMessageBox>
#include <QPainter>
#include <QStatusBar>
#include <QTableView>
#include <QTextEdit>

namespace KLib {
const int LedgerBuffer::NO_ROW = -1;
const QString LedgerController::SPLIT_TEXT = QObject::tr("<< Split >>");
QDate LedgerWidgetDelegate::m_lastUsedDate = QDate::currentDate();

LedgerController::LedgerController(Ledger* _ledger, LedgerBuffer* _buffer,
                                   QObject* _parent)
    : QAbstractItemModel(_parent),
      m_ledger(_ledger),
      m_parentWidget(Core::instance()->mainWindow()),
      m_buffer(_buffer),
      m_cache(this) {
  m_dateFormat =
      SettingsManager::instance()->value("General/DateFormat").toString();
  m_accountHeightDisplayed = SettingsManager::instance()
                                 ->value("Ledger/AccountHeightDisplayed")
                                 .toInt();
  m_askBeforeAddingNewPayee = SettingsManager::instance()
                                  ->value("Ledger/AskBeforeAddingNewPayee")
                                  .toInt();
  m_expandAllSplits = true;
  m_defaultFont.setPointSize(
      SettingsManager::instance()->value("Ledger/FontSize").toInt());

  m_precision = account()->idSecurity() == Constants::NO_ID
                    ? CurrencyManager::instance()
                          ->get(account()->mainCurrency())
                          ->precision()
                    : SecurityManager::instance()
                          ->get(account()->idSecurity())
                          ->precision();

  connect(m_buffer, &LedgerBuffer::rowInserted, this,
          &LedgerController::onBufferRowInserted);
  connect(m_buffer, &LedgerBuffer::rowRemoved, this,
          &LedgerController::onBufferRowRemoved);
  connect(m_buffer, &LedgerBuffer::showMessage, this,
          &LedgerController::showMessage);
}

void LedgerController::setup() {
  if (m_cache.isEmpty()) {
    m_cache.reloadData();
  }
}

LedgerController::~LedgerController() { delete m_buffer; }

LedgerWidgetDelegate* LedgerController::buildDelegate(
    LedgerWidget* _widget) const {
  return new LedgerWidgetDelegate(this, _widget);
}

Account* LedgerController::account() const { return m_ledger->account(); }

Ledger* LedgerController::ledger() const { return m_ledger; }

bool LedgerController::doClickEdition(const QModelIndex& _index) {
  if (!_index.isValid() ||
      _index.parent().isValid()  // We do not do click edit on sub-rows
      || _index.row() >= newTransactionRow() ||
      m_cache[_index.row()].schedule || !canEdit(_index.row())) {
    return false;
  }

  try {
    Transaction* t = m_cache[_index.row()].editableTransaction();

    if (_index.column() == col_flag()) {
      t->setFlagged(!t->isFlagged());

      emit dataChanged(_index, _index);
      return true;
    } else if (_index.column() == col_cleared()) {
      // Roll through the reconciled statuses... None ... Cleared ... Reconciled
      // ...
      if (t->clearedStatus() != ClearedStatus::Reconciled) {
        t->setClearedStatus(t->clearedStatus() + 1);
      } else {
        t->setClearedStatus(ClearedStatus::None);
      }

      emit dataChanged(_index, _index);
      return true;
    } else {
      return false;
    }
  } catch (ModelException e) {
    QMessageBox::warning(
        m_parentWidget, tr("Error"),
        tr("An error has occured while processing the request:\n\n%1")
            .arg(e.description()));
    return false;
  }
}

bool LedgerController::canClickEdit(const QModelIndex& _index) const {
  if (!_index.isValid() ||
      _index.parent().isValid()  // We do not do click edit on sub-rows
      || _index.row() >= newTransactionRow() ||
      m_cache[_index.row()].schedule || !canEdit(_index.row())) {
    return false;
  } else if (_index.column() == col_flag() ||
             _index.column() == col_cleared()) {
    return true;
  } else {
    return false;
  }
}

bool LedgerController::canEditColumn(int _column) const {
  if (_column == col_status() || _column == col_flag() ||
      _column == col_cleared() || _column == col_balance() || _column < 0 ||
      _column >= columnCount()) {
    return false;
  } else {
    return true;
  }
}

bool LedgerController::duplicateTransaction(const QModelIndex& _index,
                                            const QDate& _date) {
  int mainRow = mapToCacheRow(_index);

  if (_index.isValid() && !hasModifiedRow() && mainRow < rowCount() &&
      mainRow != newTransactionRow()) {
    // Simply load the row in the buffer, then set its row id to the new row
    m_buffer->load(m_cache[mainRow].transaction(), this);

    if (_date.isValid()) {
      m_buffer->date = _date;
    }

    m_buffer->row = newTransactionRow();
    emit editNewStarted();
    emit editStarted(m_buffer->row);
    emit dataChanged(index(newTransactionRow(), 0),
                     index(newTransactionRow(), col_balance() - 1));

    if (m_buffer->rowCount() > 1) {
      beginInsertRows(index(m_buffer->row, 0), 0, m_buffer->rowCount() - 2);
      endInsertRows();
    }

    return true;
  }

  return false;
}

int LedgerController::mapToCacheRow(const QModelIndex& _index) const {
  return _index.isValid() ? (_index.parent().isValid() ? _index.parent().row()
                                                       : _index.row())
                          : -1;
}

bool LedgerController::enterSchedule(const QModelIndex& _index) {
  int row = mapToCacheRow(_index);

  if (rowIsSchedule(row)) {
    try {
      m_cache[row].schedule->enterOccurrenceOf(m_cache[row].dueDate);
      return true;
    } catch (ModelException e) {
      QMessageBox::warning(
          m_parentWidget, tr("Save Changes"),
          tr("An error has occured while entering the schedule:\n\n%1")
              .arg(e.description()));
      return false;
    }
  } else {
    return false;
  }
}

void LedgerController::editNote(const QModelIndex& _index) {
  int row = mapToCacheRow(_index);

  if (row == -1 || (hasModifiedRow() && row != modifiedRow())) return;

  QTextEdit* txtEdit = new QTextEdit();

  if (row == modifiedRow()) {
    txtEdit->setPlainText(m_buffer->note);
  } else {
    txtEdit->setPlainText(cacheItemAtRow(row).transaction()->note());
  }

  CAMSEGDialog* dlg =
      new CAMSEGDialog(txtEdit, CAMSEGDialog::DialogWithPicture,
                       CAMSEGDialog::OkCancelButtons, m_parentWidget);

  dlg->setBothTitles(tr("Transaction Note"));
  dlg->setPicture(Core::pixmap("note"));
  dlg->setMinimumSize(400, 250);

  if (dlg->exec() == QDialog::Accepted) {
    try {
      if (row == modifiedRow()) {
        m_buffer->note = txtEdit->toPlainText();
      } else if (rowIsSchedule(row)) {
        cacheItemAtRow(row).schedule->transaction()->setNote(
            txtEdit->toPlainText());
      } else {
        cacheItemAtRow(row).editableTransaction()->setNote(
            txtEdit->toPlainText());
      }
    } catch (ModelException e) {
      QMessageBox::warning(
          m_parentWidget, tr("Transaction Note"),
          tr("Unable to set the note!\n\n%1").arg(e.description()));
    }
  }

  delete dlg;
}

void LedgerController::editAttachments(const QModelIndex& _index) {
  int row = mapToCacheRow(_index);

  if (row == -1 || (hasModifiedRow() && row != modifiedRow())) return;

  QList<int> attachments;

  if (row == modifiedRow()) {
    attachments = m_buffer->attachments.toList();
  } else {
    attachments = cacheItemAtRow(row).transaction()->attachments().toList();
  }

  FormEditAttachments* dlg =
      new FormEditAttachments(attachments, m_parentWidget);
  dlg->exec();

  try {
    if (row == modifiedRow()) {
      m_buffer->attachments = attachments.toSet();
    } else if (rowIsSchedule(row)) {
      cacheItemAtRow(row).schedule->transaction()->setAttachments(
          attachments.toSet());
    } else {
      cacheItemAtRow(row).editableTransaction()->setAttachments(
          attachments.toSet());
    }
  } catch (ModelException e) {
    QMessageBox::warning(
        m_parentWidget, tr("Transaction Attachments"),
        tr("Unable to set the attachments!\n\n%1").arg(e.description()));
  }

  delete dlg;
}

bool LedgerController::schedule(const QModelIndex& _index, const QDate& _date) {
  int row = mapToCacheRow(_index);

  if (hasModifiedRow()) {
    if (row != m_buffer->row || !m_buffer->isSchedule) {
      return false;
    } else {
      // Open schedule dialog
      FormEditSchedule form(&m_buffer->schedule, m_parentWidget);

      if (form.exec() == QDialog::Accepted &&
          (row == newTransactionRow() ||
           m_buffer->schedule.recurrence().frequency == Frequency::Once)) {
        m_buffer->date = m_buffer->schedule.recurrence().beginDate;
        emit dataChanged(index(row, col_date()), index(row, col_date()));
      }
    }
  } else if (row != newTransactionRow() && row >= 0 && row < rowCount()) {
    if (rowIsSchedule(row)) {
      // Edit the current schedule
      FormEditSchedule form(m_cache[row].schedule, m_parentWidget);
      form.exec();
    } else if (duplicateTransaction(
                   _index, _date))  // Duplicate the transaction as schedule
    {
      m_buffer->makeSchedule();
    }
  }

  return false;
}

bool LedgerController::rowIsSchedule(int _row) const {
  if (_row == m_buffer->row) {
    return m_buffer->isSchedule;
  } else if (_row != newTransactionRow() && _row >= 0 && _row < rowCount()) {
    return m_cache[_row].schedule;
  } else {
    return false;
  }
}

Amount LedgerController::totalForAccount(int _row) const {
  Amount tot;

  if (_row < 0) {
    return 0;
  } else if (_row == m_buffer->row) {
    tot = m_buffer->totalForAccount(this);
  } else if (_row != newTransactionRow()) {
    tot = Transaction::totalForAccount(account()->id(),
                                       m_cache[_row].transaction()->splits());
  } else {
    return 0;
  }

  return Account::negativeDebits(account()->type()) ? tot * -1 : tot;
}

int LedgerController::newTransactionRow() const { return m_cache.count(); }

bool LedgerController::rowIsLocked(int _row) const {
  if (_row != newTransactionRow() && _row >= 0 && _row < rowCount()) {
    return !canEdit(_row);
  } else {
    return false;
  }
}

bool LedgerController::submitChanges(int& _firstErrorColumn) {
  if (m_buffer->row == LedgerBuffer::NO_ROW) return true;

  QStringList errors = m_buffer->validate(_firstErrorColumn, this);
  const int row = m_buffer->row;
  // bool wasFirst =

  if (!errors.isEmpty()) {
    QString strErrors;
    for (QString s : errors) {
      strErrors += "\t" + s + "\n";
    }
    int ans = QMessageBox::question(
        m_parentWidget, tr("Save Changes"),
        tr("The following errors prevent the transaction to be "
           "saved:\n %1\nPress Yes to continue editing or "
           "Discard to discard the changes.")
            .arg(strErrors),
        QMessageBox::Yes | QMessageBox::Discard, QMessageBox::Yes);

    if (ans == QMessageBox::Yes) {
      return false;
    } else {
      discardChanges();
      return true;
    }
  }

  // In case saving fails, we still need to return a valid column
  _firstErrorColumn = col_date();

  try {
    // Update the number of rows to insert/remove the right number of rows
    // later.
    if (m_buffer->row != newTransactionRow()) {
      m_cache[m_buffer->row].cachedSubRowCount = m_buffer->rowCount() - 1;
    }

    if (!m_buffer->save(this)) {
      return false;
    }
  } catch (ModelException e) {
    QMessageBox::warning(
        m_parentWidget, tr("Save Changes"),
        tr("An error has occured while saving the transaction:\n\n%1")
            .arg(e.description()));
    return false;
  }

  emit changesSaved();
  emit editFinished(row);
  return true;
}

void LedgerController::discardChanges() {
  int row = m_buffer->row;
  int rowCount = m_buffer->rowCount();
  m_buffer->clear();
  QModelIndex parent = createIndex(row, 0);

  if (row != newTransactionRow()) {
    m_cache[row].cachedSubRowCount = cacheSubRowCount(row);

    if (cacheSubRowCount(row) < rowCount) {
      beginRemoveRows(parent, cacheSubRowCount(row), rowCount - 1);
      endRemoveRows();
    } else if (cacheSubRowCount(row) > rowCount) {
      beginInsertRows(parent, rowCount, cacheSubRowCount(row) - 1);
      endInsertRows();
    }

    emit dataChanged(
        parent, index(cacheSubRowCount(row) - 1, columnCount() - 1, parent));
  } else {
    if (rowCount > 1) {
      beginRemoveRows(parent, 0, rowCount - 1);
      endRemoveRows();
    }

    emit dataChanged(parent, createIndex(row, columnCount() - 1));
  }

  emit changesDiscarded();
  emit editFinished(row);
}

void LedgerController::editSplits(const QModelIndex& _index) {
  int cacheRow = mapToCacheRow(_index);

  // Start the edit if not already started and make into split
  if (cacheRow >= 0 && startEdit(cacheRow)) {
    m_buffer->changeToSplitTransaction(this);
  }
}

void LedgerController::onBufferRowInserted(int _row) {
  if (m_buffer->row != LedgerBuffer::NO_ROW) {
    beginInsertRows(index(m_buffer->row, 0), _row, _row);
    endInsertRows();
  }
}

void LedgerController::onBufferRowRemoved(int _row) {
  if (m_buffer->row != LedgerBuffer::NO_ROW) {
    beginRemoveRows(index(m_buffer->row, 0), _row, _row);
    endRemoveRows();
  }
}

void LedgerController::onSettingsChanged(const QString& _key) {
  if (_key == "Ledger/ScheduleDisplayPolicy") {
    m_cache.reloadData();
  }
}

Balances LedgerController::cacheBalance(int _cacheRow) const {
  return m_cache[_cacheRow].transaction()->totalFor(m_ledger->idAccount());
}

bool LedgerController::canEdit(int _row, QString* _message) const {
  return _row != -1 &&
         (_row == newTransactionRow() ||
          canEditTransaction(m_cache[_row].transaction(), _message));
}

QList<Transaction::Split> LedgerController::displayedSplits(
    const Transaction* _tr) const {
  QList<Transaction::Split> splits = _tr->splits();

  // Place the relevant split first.
  int i = 0;

  while (i < splits.count()) {
    if (splits[i].idAccount == m_ledger->idAccount()) {
      splits.swap(i, 0);
    } else if (_tr->isCurrencyExchange() &&
               Account::accountIsCurrencyTrading(splits[i].idAccount)) {
      splits.removeAt(i);
      continue;
    }

    ++i;
  }

  return splits;
}

QVariant LedgerController::cacheData(int _column, int _cacheRow, int _row,
                                     bool _editRole) const {
  const Transaction* tr = m_cache[_cacheRow].transaction();
  const QList<Transaction::Split> splits = displayedSplits(tr);
  const bool isSplit = splits.count() != 2;

  auto accountDisplay = [this](int idAccount, const QString& currency) {
    QString path =
        Account::getTopLevel()->getPath(idAccount, accountHeightDisplayed());

    Account* a = Account::getTopLevel()->account(idAccount);
    if (a && a->allCurrencies().count() > 1) {
      return QString("%1 (%2)").arg(path).arg(currency);
    } else {
      return path;
    }
  };

  if (_row == 0) {
    if (_column == col_status()) {
      if (!m_cache[_cacheRow].schedule) {
        return (int)(canEdit(_cacheRow) ? CacheItemType::PlainTransaction
                                        : CacheItemType::LockedTransaction);
      } else  // Schedule
      {
        bool isBill = m_cache[_cacheRow].schedule->remindBefore() >= 0;
        if (!canEdit(_cacheRow)) {
          return (int)(isBill ? CacheItemType::LockedBill
                              : CacheItemType::LockedSchedule);
        } else if (m_cache[_cacheRow].dueDate >= QDate::currentDate()) {
          return (int)(isBill ? CacheItemType::BillReminder
                              : CacheItemType::FutureSchedule);
        } else {
          return (int)(isBill ? CacheItemType::OverdueBill
                              : CacheItemType::OverdueSchedule);
        }
      }
    } else if (_column == col_no()) {
      return tr->no();
    } else if (_column == col_flag()) {
      return tr->isFlagged();
    } else if (_column == col_date()) {
      QDate d;

      if (!m_cache[_cacheRow].schedule) {
        d = tr->date();
      } else {
        d = m_cache[_cacheRow].dueDate;
      }

      return d.toString(m_dateFormat);
    } else if (_column == col_memo()) {
      return tr->memo();
    } else if (_column == col_payee()) {
      return tr->idPayee() == Constants::NO_ID
                 ? QVariant()
                 : PayeeManager::instance()->get(tr->idPayee())->name();
    } else if (_column == col_cleared()) {
      return tr->clearedStatus();
    }
  }

  if (_row == 0 && _column == col_balance() &&
      (!m_cache[_cacheRow].schedule ||
       m_cache[_cacheRow].dueDate > QDate::currentDate())) {
    return formatBalances(_cacheRow);
  } else if (_row == 0 && !isSplit) {
    if (_column == col_transfer()) {
      if (!_editRole) {
        return accountDisplay(splits[1].idAccount, splits[1].currency);
      } else {
        return QVariant::fromValue(
            AccountCurrency(splits[1].idAccount, splits[1].currency));
      }
    } else if (_column == col_debit()) {
      return splits[0].amount > 0
                 ? formatCurrency(splits[0].amount, splits[0].currency)
                 : QVariant();
    } else if (_column == col_credit()) {
      return splits[0].amount < 0
                 ? formatCurrency(-splits[0].amount, splits[0].currency)
                 : QVariant();
    }
  } else if (isSplit && _row < tr->splitCount()) {
    if (_column == col_transfer()) {
      if (!_editRole) {
        return accountDisplay(splits[_row].idAccount, splits[_row].currency);
      } else {
        return QVariant::fromValue(
            AccountCurrency(splits[_row].idAccount, splits[_row].currency));
      }
    } else if (_column == col_debit()) {
      return splits[_row].amount > 0
                 ? formatCurrency(splits[_row].amount, splits[_row].currency)
                 : QVariant();
    } else if (_column == col_credit()) {
      return splits[_row].amount < 0
                 ? formatCurrency(-splits[_row].amount, splits[_row].currency)
                 : QVariant();
    }
  }

  return QVariant();
}

QString LedgerController::formatCurrency(const Amount& _amount,
                                         const QString& _currency) const {
  if (account()->allCurrencies().count() > 1)  // Show currency signs
  {
    try {
      return CurrencyManager::instance()->get(_currency)->formatAmount(_amount);
    } catch (...) {
    }
  }

  return _amount.toPrecision(m_precision).toString();
}

QString LedgerController::formatBalances(int _cacheRow) const {
  //        QString show;
  //        Balances cur = m_balanceCache[_cacheRow];
  //        Balances prev = _cacheRow > 0 ? m_balanceCache[_cacheRow-1] :
  //        Balances();

  //        for (auto i = cur.begin(); i != cur.end(); ++i)
  //        {
  //            if (!prev.contains(i.key()) || prev.value(i.key()) != i.value())
  //            //Show it!
  //            {
  //                show +=
  //                CurrencyManager::instance()->get(i.key())->formatAmount(i.value())
  //                + " ";
  //            }
  //        }

  //        //Erase last space
  //        show.resize(show.size()-1);

  //        return show;

  return account()->formatAmount(
      m_cache.balanceAt(_cacheRow).inCurrency(account()->mainCurrency()));
}

int LedgerController::cacheSubRowCount(int _cacheRow) const {
  return subRowCount(m_cache[_cacheRow].transaction());
}

int LedgerController::subRowCount(const Transaction* _tr) const {
  int count = displayedSplits(_tr).count();
  return m_expandAllSplits && count > 2 ? count - 1 : 0;
}

void LedgerController::loadBuffer(int _row) {
  if (m_buffer->row == LedgerBuffer::NO_ROW) {
    if (_row != newTransactionRow() && !m_cache[_row].schedule) {
      // Set the values of the row in the buffer
      m_buffer->row = _row;
      m_buffer->load(m_cache[_row].transaction(), this);

      emit editExistingStarted();
    } else if (_row != newTransactionRow() && m_cache[_row].schedule) {
      // Set the values of the row in the buffer
      m_buffer->row = _row;
      m_buffer->load(m_cache[_row].schedule, this);
      m_buffer->date = m_cache[_row].dueDate;

      emit editExistingStarted();
    } else  //_row == newTransactionRow()
    {
      m_buffer->clear();
      m_buffer->row = _row;
      emit editNewStarted();
    }

    emit editStarted(m_buffer->row);
  }
}

int LedgerController::rowCount(const QModelIndex& _parent) const {
  int row_count = 0;
  if (!_parent.isValid()) {  // Main rows
    // One more for the ADD row
    row_count = m_cache.count() + 1;
  } else if (!_parent.parent().isValid() &&
             _parent.row() == m_buffer->row) {  // Cache
    row_count = m_buffer->rowCount() - 1;
  } else if (!_parent.parent().isValid() && _parent.row() >= 0 &&
             _parent.row() < m_cache.count()) {  // Buffer transaction
    row_count = cacheSubRowCount(_parent.row());
  }
  return row_count;
}

bool LedgerController::alignCenter(int _column) const {
  return _column == col_cleared() || _column == col_flag();
}

QVariant LedgerController::data(const QModelIndex& _index, int _role) const {
  const int mainRow = mapToCacheRow(_index);

  if (!_index.isValid() || mainRow >= rowCount() ||
      _index.column() >= columnCount())
    return QVariant();

  try {
    switch (_role) {
      case Qt::TextColorRole:
        if (mainRow != newTransactionRow()) {
          if (m_cache[mainRow].schedule) {
            return QColor(Qt::gray);
          } else if (_index.column() == col_balance()) {
            return m_cache.balanceAt(mainRow) < 0 ? QColor(Qt::red)
                                                  : QColor(Qt::black);
          }
        }
        break;
      case Qt::FontRole:
        if (mainRow == m_buffer->row) {
          QFont f = m_defaultFont;
          f.setItalic(true);
          return f;
        } else if (mainRow != newTransactionRow() &&
                   m_cache[mainRow].schedule &&
                   m_cache[mainRow].dueDate < QDate::currentDate()) {
          QFont f = m_defaultFont;
          f.setBold(true);
          return f;
        } else {
          return m_defaultFont;
        }
        break;

      case Qt::ToolTipRole:
        if (_index.column() == col_status()) {
          // Check if message
          QString message;

          if (!canEdit(mainRow, &message)) {
            return message;
          } else if (mainRow != newTransactionRow() &&
                     m_cache[mainRow].schedule) {
            if (m_cache[mainRow].dueDate < QDate::currentDate()) {
              return m_cache[mainRow].schedule->remindBefore() < 0
                         ? tr("Overdue Scheduled Transaction")
                         : tr("Overdue Bill");
            } else if (mainRow != newTransactionRow() &&
                       m_cache[mainRow].schedule) {
              return m_cache[mainRow].schedule->remindBefore() < 0
                         ? tr("Scheduled Transaction")
                         : tr("Bill Reminder");
            }
          } else {
            return QString();
          }
        }
        break;

      case Qt::TextAlignmentRole:
        if (alignRight(_index.column())) {
          return (int)(Qt::AlignRight | Qt::AlignVCenter);
        } else if (alignCenter(_index.column())) {
          return (int)(Qt::AlignHCenter | Qt::AlignVCenter);
        }
        break;

      case Qt::EditRole:
      case Qt::DisplayRole:
        if (mainRow == m_buffer->row && _index.column() == col_balance()) {
          return mainRow == newTransactionRow() || _index.parent().isValid()
                     ? QVariant()
                     : QVariant(formatBalances(mainRow));
        } else if (mainRow == m_buffer->row) {
          return m_buffer->data(
              /* column */ _index.column(),
              /* row    */ _index.parent().isValid() ? _index.row() + 1 : 0,
              /* role   */ _role == Qt::EditRole,
              /* cntlr  */ this);
        } else if (mainRow != newTransactionRow()) {
          return cacheData(
              /* column    */ _index.column(),
              /* cache row */ mainRow,
              /* cache row */ _index.parent().isValid() ? _index.row() + 1 : 0,
              /* role      */ _role == Qt::EditRole);
        }
        break;

      default:
        break;
    }
  } catch (ModelException e) {
    qDebug() << e.description();
  }

  return QVariant();
}

QVariant LedgerController::headerData(int _section,
                                      Qt::Orientation _orientation,
                                      int _role) const {
  if (_role == Qt::DisplayRole) {
    if (_orientation == Qt::Horizontal) {
      if (_section == col_no()) {
        return tr("Check #");
      } else if (_section == col_date()) {
        return tr("Date");
      } else if (_section == col_memo()) {
        return tr("Memo");
      } else if (_section == col_payee()) {
        return tr("Payee");
      } else if (_section == col_cleared()) {
        return tr("Clr");
      } else if (_section == col_transfer()) {
        return tr("Transfer");
      } else if (_section == col_debit()) {
        return tr("Debit");
      } else if (_section == col_credit()) {
        return tr("Credit");
      } else if (_section == col_balance()) {
        return tr("Balance");
      } else {
        return QVariant();
      }
    } else {
      return QString("%1").arg(_section + 1);
    }
  }

  return QVariant();
}

QModelIndex LedgerController::index(int _row, int _column,
                                    const QModelIndex& _parent) const {
  if (!hasIndex(_row, _column, _parent)) {
    return QModelIndex();
  } else {
    if (_parent.isValid()) {
      return createIndex(_row, _column, (quintptr)_parent.row() + 1);
    } else {
      return createIndex(_row, _column, (quintptr)0);
    }
  }
}

QModelIndex LedgerController::parent(const QModelIndex& _child) const {
  if (!_child.isValid() || !_child.internalId()) {
    return QModelIndex();
  } else {
    return createIndex(((int)_child.internalId()) - 1, 0, (quintptr)0);
  }
}

bool LedgerController::editNewSchedule() {
  if (startEdit(newTransactionRow())) {
    m_buffer->makeSchedule();
    return true;
  } else {
    return false;
  }
}

bool LedgerController::editNewBillReminder() {
  if (startEdit(newTransactionRow())) {
    m_buffer->makeSchedule();
    m_buffer->schedule.setRemindBefore(5);
    return true;
  } else {
    return false;
  }
}

bool LedgerController::startEdit(int _row) {
  if (m_buffer->row != LedgerBuffer::NO_ROW && _row == m_buffer->row) {
    return true;  // Already set!
  }

  // Can only modify if the account is opened
  // Check that we do not want to edit a different row than the one currently
  // edited
  if (m_ledger->account()->isOpen() &&
      (m_buffer->row == LedgerBuffer::NO_ROW || m_buffer->row == _row) &&
      canEdit(_row)) {
    // Load the buffer if necessary
    loadBuffer(_row);

    return true;
  } else {
    return false;
  }
}

bool LedgerController::canRemoveSplitAt(const QModelIndex& _index) {
  return _index.isValid() && mapToCacheRow(_index) == modifiedRow() &&
         _index.parent().isValid();
}

void LedgerController::removeSplitAt(const QModelIndex& _index) {
  if (canRemoveSplitAt(_index)) {
    m_buffer->removeRowAt(_index.parent().isValid() ? _index.row() + 1 : 0);
  }
}

bool LedgerController::setData(const QModelIndex& _index,
                               const QVariant& _value, int _role) {
  const int mainRow = mapToCacheRow(_index);

  if (_index.isValid() && _role == Qt::EditRole && startEdit(mainRow)) {
    bool ret = m_buffer->setData(
        /* column */ _index.column(),
        /* row    */ _index.parent().isValid() ? _index.row() + 1 : 0,
        /* value  */ _value,
        /* cntlr  */ this);

    if (ret) {
      emit dataChanged(_index, _index);
    }

    return ret;
  } else {
    return false;
  }
}

//--------------------------------------------- BUFFER
//---------------------------------------------

void LedgerBuffer::showImbalances(const QList<Transaction::Split>& splits) {
  // Show message with imbalances
  emit showMessage(
      QObject::tr("Imbalances:%1").arg(Transaction::splitsImbalances(splits)));
}

void LedgerBuffer::clear() {
  if (splits.count()) {
    showMessage(QString());
  }

  row = NO_ROW;
  no.clear();
  date = QDate::currentDate();
  memo.clear();
  payee.clear();
  idTransfer = idSchedule = Constants::NO_ID;
  debit = credit = exchTransfer = 0;
  multiCurrency = flagged = isSchedule = false;
  clearedStatus = ClearedStatus::None;
  splits.clear();
  schedule.setDescription(QString());
  schedule.setRecurrence(
      Recurrence(LedgerWidgetDelegate::lastUsedDate(), Frequency::Once, 0));
  attachments.clear();
}

void LedgerBuffer::makeSchedule() {
  if (!isSchedule) {
    isSchedule = true;
    schedule.setRecurrence(Recurrence(date, Frequency::Once, 0));
    idSchedule = Constants::NO_ID;
  }
}

bool LedgerBuffer::setData(int _column, int _row, const QVariant& _value,
                           LedgerController* _controller) {
  auto setAccountCurrency = [_value](int& _idTransfer, QString& _currency) {
    AccountCurrency ac = _value.value<AccountCurrency>();

    _idTransfer = ac.first;

    if (!ac.second.isEmpty()) {
      _currency = ac.second;
    } else {
      Account* a = Account::getTopLevel()->account(ac.first);
      _currency = a ? a->mainCurrency() : QString();
    }
  };

  if (_row == 0) {
    if (_column == _controller->col_no()) {
      no = _value.toString();
    } else if (_column == _controller->col_date()) {
      date = _value.toDate();

      if (isSchedule && (schedule.recurrence().frequency == Frequency::Once ||
                         row == _controller->newTransactionRow())) {
        Recurrence r = schedule.recurrence();
        r.beginDate = date;
        schedule.setRecurrence(r);
      }
    } else if (_column == _controller->col_memo()) {
      memo = _value.toString();
    } else if (_column == _controller->col_payee()) {
      payee = _value.toString();
    }
  }

  if (splits.count() && _row < splits.count())  // If we're in split mode
  {
    if (_column == _controller->col_transfer()) {
      if (_row == 0) return false;

      setAccountCurrency(splits[_row].idAccount, splits[_row].currency);
    } else if (_column == _controller->col_debit()) {
      Amount a = Amount::fromUserLocale(_value.toString());

      if (a == 0 &&
          splits[_row].amount <
              0)  // Check if there was data in the Credit column
      {
        return false;
      }

      splits[_row].amount = a;
    } else if (_column == _controller->col_credit()) {
      Amount a = Amount::fromUserLocale(_value.toString());

      if (a == 0 &&
          splits[_row].amount >
              0)  // Check if there was data in the Debit column
      {
        return false;
      }

      splits[_row].amount = -a;
    }

    // Show message with imbalances
    showImbalances(splits);

    // If editing last row and is not empty, we add one
    if (_row == rowCount() - 1 && !rowIsEmpty(_row)) {
      insertRowAt(rowCount());
    } else if (rowIsEmpty(rowCount() - 1) && rowIsEmpty(rowCount() - 2)) {
      // If the two last rows are empty, remove one.
      removeRow(rowCount() - 1);
    }
  } else if (splits.count() == 0 && _row == 0)  // Otherwise (not in split mode)
  {
    if (_column == _controller->col_transfer()) {
      setAccountCurrency(idTransfer, transferCurrency);
    } else if (_column == _controller->col_debit()) {
      debit = Amount::fromUserLocale(_value.toString());

      if (debit > 0) credit = 0;
    } else if (_column == _controller->col_credit()) {
      credit = Amount::fromUserLocale(_value.toString());

      if (credit > 0) debit = 0;
    }
  } else {
    return false;
  }

  return true;
}

bool LedgerBuffer::rowIsEmpty(int _row) {
  if (_row < 0 || _row >= splits.count()) return false;

  return splits[_row].amount == 0
         //&& splits[_row].memo.isEmpty()
         && splits[_row].idAccount == Constants::NO_ID;
}

void LedgerBuffer::ensureOneEmptyRow() {
  if (splits.count() <= 1 || !rowIsEmpty(rowCount() - 1)) {
    insertRowAt(rowCount());
  }
}

void LedgerBuffer::insertRowAt(int _row) {
  splits.insert(_row, Transaction::Split());
  emit rowInserted(_row);
}

void LedgerBuffer::removeRow(int _row) {
  splits.removeAt(_row);
  emit rowRemoved(_row);
}

QVariant LedgerBuffer::data(int _column, int _row, bool _editRole,
                            const LedgerController* _controller) const {
  auto accountDisplay = [_controller](int idAccount, const QString& currency) {
    QString path = Account::getTopLevel()->getPath(
        idAccount, _controller->accountHeightDisplayed());

    Account* a = Account::getTopLevel()->account(idAccount);
    if (a && a->allCurrencies().count() > 1) {
      return QString("%1 (%2)").arg(path).arg(currency);
    } else {
      return path;
    }
  };

  if (_row == 0) {
    if (_column == _controller->col_status()) {
      if (!isSchedule) {
        return (int)CacheItemType::PlainTransaction;
      } else {
        if (date >= QDate::currentDate()) {
          return (int)CacheItemType::FutureSchedule;
        } else {
          return (int)CacheItemType::OverdueSchedule;
        }
      }
    } else if (_column == _controller->col_no()) {
      return no;
    } else if (_column == _controller->col_cleared()) {
      return clearedStatus;
    } else if (_column == _controller->col_flag()) {
      return flagged;
    } else if (_column == _controller->col_date()) {
      return date.toString(_controller->dateFormat());
    } else if (_column == _controller->col_memo()) {
      return memo;
    } else if (_column == _controller->col_payee()) {
      return payee;
    }
  }

  if (splits.count() && _row < splits.count()) {
    if (_column == _controller->col_transfer()) {
      if (!_editRole) {
        return accountDisplay(splits[_row].idAccount, splits[_row].currency);
      } else {
        return QVariant::fromValue(
            AccountCurrency(splits[_row].idAccount, splits[_row].currency));
      }
    } else if (_column == _controller->col_debit() && splits[_row].amount > 0) {
      return _controller->formatCurrency(splits[_row].amount,
                                         splits[_row].currency);
    } else if (_column == _controller->col_credit() &&
               splits[_row].amount < 0) {
      return _controller->formatCurrency(-splits[_row].amount,
                                         splits[_row].currency);
    }
  } else if (!splits.count() && _row == 0) {
    QString cur =
        _controller->account()->allCurrencies().contains(transferCurrency)
            ? transferCurrency
            : _controller->account()->mainCurrency();
    if (_column == _controller->col_transfer()) {
      if (!_editRole) {
        return accountDisplay(idTransfer, transferCurrency);
      } else {
        return QVariant::fromValue(
            AccountCurrency(idTransfer, transferCurrency));
      }
    } else if (_column == _controller->col_debit()) {
      return debit > 0 ? _controller->formatCurrency(debit, cur) : QString();
    } else if (_column == _controller->col_credit()) {
      return credit > 0 ? _controller->formatCurrency(credit, cur) : QString();
    }
  }

  return QVariant();
}

void LedgerBuffer::load(const Schedule* _schedule,
                        const LedgerController* _controller) {
  load(_schedule->transaction(), _controller);

  // Copy the schedule
  isSchedule = true;
  idSchedule = _schedule->id();
  schedule.copyFrom(*_schedule);
}

void LedgerBuffer::load(const Transaction* _transaction,
                        const LedgerController* _controller) {
  no = _transaction->no();
  date = _transaction->date();
  memo = _transaction->memo();
  note = _transaction->note();

  clearedStatus = _transaction->clearedStatus();
  flagged = _transaction->isFlagged();
  attachments = _transaction->attachments();

  try {
    payee =
        _transaction->idPayee() == Constants::NO_ID
            ? QString()
            : PayeeManager::instance()->get(_transaction->idPayee())->name();
  } catch (ModelException) {
    payee.clear();
  }

  loadSplits(_transaction->splits(), _controller);
}

void LedgerBuffer::loadSplits(const QList<Transaction::Split>& _splits,
                              const LedgerController* _controller) {
  auto loadFromSplit = [this](const Transaction::Split& cur,
                              const Transaction::Split& oth) {
    debit = cur.amount > 0 ? cur.amount : 0;
    credit = cur.amount < 0 ? -cur.amount : 0;
    idTransfer = oth.idAccount;
    transferCurrency = oth.currency;

  };

  if (_splits.count() == 2) {
    if (_splits.first().idAccount == _controller->account()->id()) {
      loadFromSplit(_splits[0], _splits[1]);
    } else {
      loadFromSplit(_splits[1], _splits[0]);
    }
  } else if (Transaction::isCurrencyExchange(_splits)) {
    Transaction::Split cur, oth;

    for (const Transaction::Split& s : _splits) {
      if (s.idAccount == _controller->account()->id()) {
        cur = s;
      } else if (Account::getTopLevel()->getChild(s.idAccount)->type() !=
                 AccountType::TRADING) {
        oth = s;
      }
    }

    loadFromSplit(cur, oth);
    multiCurrency = true;
    exchTransfer = oth.amount.abs();
  } else {
    // We want that the first split be the current account split.
    splits = _splits;
    for (int i = 0; i < splits.count(); ++i) {
      if (splits[i].idAccount == _controller->account()->id()) {
        splits.swap(i, 0);
        break;
      }
    }

    Amount tot =
        Transaction::totalForAccount(_controller->account()->id(), splits);

    debit = tot > 0 ? tot : 0;
    credit = tot < 0 ? tot : 0;

    ensureOneEmptyRow();
    showImbalances(splits);
  }
}

QStringList LedgerBuffer::validate(int& _firstErrorColumn,
                                   const LedgerController* _controller) {
  QStringList errors;
  _firstErrorColumn = -1;

  if (!date.isValid()) {
    errors << QObject::tr("The date is invalid.");
    _firstErrorColumn = _controller->col_date();
  }

  // Schedule
  if (isSchedule && !schedule.recurrence().isValid()) {
    errors << QObject::tr(
        "The schedule is invalid. Click on the <emph>Schedule</emph> button to "
        "edit it.");
  }

  if (!splits.isEmpty()) {
    // Check if the splits are valid
    QList<Transaction::Split> notEmpty;
    int i = 1;
    for (const Transaction::Split& s : splits) {
      if (s.idAccount != Constants::NO_ID || s.amount != 0) {
        if (s.idAccount == Constants::NO_ID) {
          errors << tr("Select an account for split %1.").arg(i);
          if (_firstErrorColumn == -1)
            _firstErrorColumn = _controller->col_transfer();
        } else if (s.amount == 0) {
          errors << tr("Enter an amount for split %1.").arg(i);
          if (_firstErrorColumn == -1)
            _firstErrorColumn = _controller->col_debit();
        }

        notEmpty << s;
      }

      ++i;
    }

    // Check if it balances
    try {
      if (notEmpty.count() < 2) {
        errors << tr("There must be at least two splits.");
        if (_firstErrorColumn == -1)
          _firstErrorColumn = _controller->col_transfer();
      }

      if (!Transaction::splitsBalance(notEmpty)) {
        errors << tr("The splits do not balance.");
        if (_firstErrorColumn == -1)
          _firstErrorColumn = _controller->col_debit();
      }
    } catch (ModelException) {
      errors << QObject::tr("At least one transfer account is invalid.");
      if (_firstErrorColumn == -1)
        _firstErrorColumn = _controller->col_transfer();
    }
  } else if (idTransfer ==
             Constants::NO_ID)  // Check if a transfer account is set
  {
    errors << tr("The transfer account is invalid.");
    if (_firstErrorColumn == -1)
      _firstErrorColumn = _controller->col_transfer();
  } else  // Check if the transfer account exists
  {
    Account* a = Account::getTopLevel()->account(idTransfer);

    if (!a) {
      errors << tr("The transfer account is invalid.");
      if (_firstErrorColumn == -1)
        _firstErrorColumn = _controller->col_transfer();
    } else if (a->isPlaceholder()) {
      errors << tr("The transfer account is a placeholder.");
      if (_firstErrorColumn == -1)
        _firstErrorColumn = _controller->col_transfer();
    }
  }

  if (debit - credit == 0 && splits.isEmpty()) {
    errors << tr("The transaction amount is zero.");
    if (_firstErrorColumn == -1) _firstErrorColumn = _controller->col_debit();
  }

  return errors;
}

QList<Transaction::Split> LedgerBuffer::splitsForSaving(
    LedgerController* _controller, bool& _ok) const {
  //--------------------Amount--------------------
  Amount total;

  if (debit != 0 && credit != 0) {
    total = debit - credit;
  } else if (debit != 0) {
    total = debit;
  } else {
    total = -credit;
  }

  //--------------------Splits--------------------
  QList<Transaction::Split> tmp_splits;

  if (splits.isEmpty()) {
    // Check if we are doing a currency conversion
    // If the current account has multiple currencies, we try to match
    // to the other account's currency. If match is found, then no conversion.
    // Otherwise, convert from main currency to other account's currency
    if (idTransfer != Constants::NO_ID &&
        !_controller->account()->allCurrencies().contains(transferCurrency)) {
      QString from = _controller->account()->mainCurrency();
      QString to = transferCurrency;

      FormCurrencyExchange frm(from, to, total, date,
                               _controller->parentWidget());

      if (row !=
          _controller->newTransactionRow())  // Transaction is being modified
      {
        frm.setToAmount(exchTransfer);
      }

      if (frm.exec() == QDialog::Rejected) {
        _ok = false;
        return tmp_splits;
      }

      tmp_splits << Transaction::Split(total, _controller->account()->id(),
                                       from);
      tmp_splits << Transaction::Split(-frm.exchangedAmount(), idTransfer, to);
      Transaction::addTradingSplits(tmp_splits);

      //                tmp_splits << Transaction::Split(-total,
      //                CurrencyManager::instance()->get(from)->tradingAccount()->id());
      //                tmp_splits << Transaction::Split(frm.exchangedAmount(),
      //                CurrencyManager::instance()->get(to)->tradingAccount()->id());

    } else  // No exchange, a plain 2 account transfer
    {
      tmp_splits << Transaction::Split(total, _controller->account()->id(),
                                       _controller->account()->mainCurrency());
      tmp_splits << Transaction::Split(-total, idTransfer, transferCurrency);
    }
  } else {
    // Keep only the non-empty splits
    QList<Transaction::Split> notEmpty;
    for (const Transaction::Split& s : splits) {
      if (s.idAccount != Constants::NO_ID || s.amount != 0) {
        notEmpty << s;
      }
    }

    tmp_splits = notEmpty;
  }

  _ok = true;
  return tmp_splits;
}

QHash<QString, QVariant> LedgerBuffer::propertiesForSaving(
    LedgerController* _controller) const {
  Q_UNUSED(_controller)
  return QHash<QString, QVariant>();
}

bool LedgerBuffer::saveToTransaction(Transaction* _transaction,
                                     LedgerController* _controller) {
  int idPayee = Constants::NO_ID;

  if (!payee.isEmpty()) {
    try {
      idPayee = PayeeManager::instance()->get(payee)->id();
    } catch (ModelException) {
      if (_controller->m_askBeforeAddingNewPayee) {
        int ans = QMessageBox::question(
            _controller->parentWidget(), QObject::tr("Save Changes"),
            QObject::tr("The payee you entered does not currently "
                        "exists. Do you want to add it, have no "
                        "payee for this transaction, or cancel?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        switch (ans) {
          case QMessageBox::Yes:
            idPayee = PayeeManager::instance()->add(payee)->id();
            break;

          case QMessageBox::No:
            idPayee = Constants::NO_ID;
            payee.clear();
            break;

          default:
            return false;
        }
      } else {
        idPayee = PayeeManager::instance()->add(payee)->id();
      }
    }
  }

  bool ok;
  QList<Transaction::Split> tmp_splits = splitsForSaving(_controller, ok);

  if (!ok) {
    return false;
  }

  QHash<QString, QVariant> prop = propertiesForSaving(_controller);

  // Remove the empty rows
  int i = 0;
  while (i < splits.count()) {
    if (rowIsEmpty(i))
      removeRow(i);
    else
      ++i;
  }

  auto setProperties = [&prop](Transaction* t) {
    t->properties()->clear();

    for (auto i = prop.begin(); i != prop.end(); ++i) {
      t->properties()->set(i.key(), i.value());
    }
  };

  _transaction->setDate(date);
  _transaction->setSplits(tmp_splits);
  _transaction->setNo(no);
  _transaction->setNote(note);
  _transaction->setIdPayee(idPayee);
  _transaction->setMemo(memo);
  _transaction->setFlagged(flagged);
  _transaction->setClearedStatus(clearedStatus);
  _transaction->setAttachments(attachments);
  setProperties(_transaction);

  return true;
}

bool LedgerBuffer::save(LedgerController* _controller) {
  // Need to clear row before inserting, since the new transaction will trigger
  // a reload of that row using the
  //"real" transaction + balance
  int tmp_row = row;

  //--------------------Save--------------------

  Transaction* transaction = nullptr;

  try {
    if (tmp_row == _controller->newTransactionRow())  // New
    {
      transaction = createTransaction();

      if (saveToTransaction(transaction, _controller)) {
        row = NO_ROW;

        if (isSchedule) {
          ScheduleManager::instance()->add(schedule.description(),
                                           schedule.autoEnter(),
                                           schedule.recurrence(), transaction);
        } else {
          LedgerManager::instance()->addTransaction(transaction);
        }
      } else {
        delete transaction;
        return false;
      }
    } else  // Modification
    {
      if (isSchedule) {
        // Different possibilities...
        QStringList options;
        options << tr("Post this occurrence with the current changes, do not "
                      "modify the schedule")
                << tr("Post this occurrence with the current changes and apply "
                      "them to the schedule as well")
                << tr("Do not post this occurrence, but save the current "
                      "changes to the schedule");

        int option = OptionsDialog::askForOptions(
            Core::pixmap("edit"), tr("Edit Schedule"),
            tr("What would you like to do?"), options, 0,
            _controller->parentWidget());

        if (option == -1) return false;

        CacheItem c = _controller->cacheItemAtRow(tmp_row);

        if (option == 0 || option == 1)  // Post the transaction
        {
          transaction = createTransaction();

          if (saveToTransaction(transaction, _controller)) {
            row = NO_ROW;

            // Add the occurrence
            c.schedule->enterOccurrenceOf(c.dueDate, transaction);
          } else {
            delete transaction;
            return false;
          }
        }

        if (option == 1 || option == 2)  // Save the changes to the schedule
        {
          if (saveToTransaction(c.schedule->transaction(), _controller)) {
            row = NO_ROW;

            schedule.setActive(true);
            c.schedule->copyFrom(schedule);
          } else {
            return false;
          }
        }
      } else {
        transaction =
            _controller->cacheItemAtRow(tmp_row).editableTransaction();
        row = NO_ROW;
        transaction->holdToModify();
        bool ok = saveToTransaction(transaction, _controller);
        transaction->doneHoldToModify();

        if (!ok) {
          row = tmp_row;
          return false;
        }
      }

      emit _controller->dataChanged(
          _controller->index(tmp_row, 0),
          _controller->index(tmp_row, _controller->columnCount() - 1));
    }

    Core::instance()->mainWindow()->statusBar()->clearMessage();
    clear();
  } catch (ModelException) {
    // Do not need to delete the transaction if exception as will be done by the
    // Ledger.
    row = tmp_row;
    throw;
  }

  return true;
}

Amount LedgerBuffer::totalForAccount(
    const LedgerController* _controller) const {
  if (splits.isEmpty()) {
    return debit > 0 ? debit : -credit;
  } else {
    return Transaction::totalForAccount(_controller->account()->id(), splits);
  }
}

void LedgerBuffer::changeToSplitTransaction(
    const LedgerController* _controller) {
  if (splits.count()) return;

  Transaction::Split s1(debit > 0 ? debit : -credit,
                        _controller->account()->id(),
                        _controller->account()->mainCurrency());

  Transaction::Split s2(
      debit > 0 ? -debit : credit, idTransfer,
      idTransfer != Constants::NO_ID
          ? Account::getTopLevel()->account(idTransfer)->mainCurrency()
          : "");

  splits << s1 << s2;
  emit rowInserted(1);
  ensureOneEmptyRow();
  showImbalances(splits);
}

int LedgerBuffer::rowCount() const {
  return splits.isEmpty() ? 1 : splits.count();
}

void LedgerBuffer::removeRowAt(int _row) {
  if (_row && _row < rowCount()) {
    removeRow(_row);
    ensureOneEmptyRow();
  }
}

//--------------------------------------------- DELEGATE
//---------------------------------------------

LedgerWidgetDelegate::LedgerWidgetDelegate(const LedgerController* _controller,
                                           LedgerWidget* _widget)
    : QStyledItemDelegate(_widget),
      m_controller(_controller),
      m_widget(_widget),
      m_currentEditor(nullptr) {
  QSettings settings;
  m_rowHeight = settings.value("Ledger/RowHeight").toInt();
//  if (m_rowHeight <= 0) {
//    m_rowHeight = 26;
//  }
}

void LedgerWidgetDelegate::paint(QPainter* _painter,
                                 const QStyleOptionViewItem& _option,
                                 const QModelIndex& _index) const {
  bool paintIcon = false;

  QPixmap toPaint;

  QStyleOptionViewItem newOption = _option;
  initStyleOption(&newOption, _index);
  newOption.state = _option.state & (~QStyle::State_HasFocus);

  // Alternate rows based on main row.

  const LedgerController* con =
      qobject_cast<const LedgerController*>(_index.model());

  if (con) {
    const int mainRow = con->mapToCacheRow(_index);

    if (mainRow == con->modifiedRow()) {
      newOption.state = newOption.state | (QStyle::State_Selected);
    } else {
      auto selected = m_widget->selectedRows();
      int row =
          _index.parent().isValid() ? _index.parent().row() : _index.row();

      if (selected.contains(row)) {
        newOption.state = newOption.state | (QStyle::State_Selected);
      }
    }
  }

  if (_index.column() == m_controller->col_status()) {
    paintIcon = true;

    switch ((CacheItemType)_index.data().toInt()) {
      case CacheItemType::LockedTransaction:
        toPaint = Core::pixmap("object-locked-16");
        break;
      case CacheItemType::FutureSchedule:
        toPaint = Core::pixmap("schedule-future-16");
        break;
      case CacheItemType::OverdueSchedule:
        toPaint = Core::pixmap("schedule-overdue-16");
        break;
      case CacheItemType::LockedSchedule:
        toPaint = Core::pixmap("schedule-locked-16");
        break;
      case CacheItemType::LockedBill:
        toPaint = Core::pixmap("bill-locked-16");
        break;
      case CacheItemType::BillReminder:
        toPaint = Core::pixmap("bill-16");
        break;
      case CacheItemType::OverdueBill:
        toPaint = Core::pixmap("bill-overdue-16");
        break;

      default:
        break;
    }

  } else if (_index.column() == m_controller->col_flag()) {
    paintIcon = true;
    toPaint = _index.data().toBool() ? Core::pixmap("flag-red-16") : QPixmap();
  } else if (_index.column() == m_controller->col_cleared()) {
    paintIcon = true;

    switch (_index.data().toInt()) {
      case ClearedStatus::Cleared:
        toPaint = Core::pixmap("transaction-cleared-16");
        break;
      case ClearedStatus::Reconciled:
        toPaint = Core::pixmap("transaction-reconciled-16");
        break;
    }
  }

  if (paintIcon) {
    if (newOption.state & QStyle::State_Selected)
      _painter->fillRect(newOption.rect, newOption.palette.highlight());

    if (!toPaint.isNull()) {
      _painter->save();
      _painter->setRenderHint(QPainter::Antialiasing, true);

      // Scale it to the right size
      //                toPaint = toPaint.scaled(iconSize(),
      //                                         Qt::KeepAspectRatio);

      // Crop it so it's not too big
      int newWidth = toPaint.width();
      int newHeight = toPaint.height();

      if (toPaint.width() > newOption.rect.width() - 4) {
        newWidth = newOption.rect.width() - 4;
      }

      if (toPaint.height() > newOption.rect.height() - 4) {
        newHeight = newOption.rect.height() - 4;
      }

      toPaint = toPaint.copy(0, 0, newWidth, newHeight);

      _painter->drawPixmap(newOption.rect.left() +
                               (newOption.rect.width() - toPaint.width()) / 2,
                           newOption.rect.top() +
                               (newOption.rect.height() - toPaint.height()) / 2,
                           toPaint.width(), toPaint.height(), toPaint);
      _painter->restore();
    }
  } else {
    QStyledItemDelegate::paint(_painter, newOption, _index);
  }

  // Draw the border
  QPen pen;
  QRect borderRect = newOption.rect;
  borderRect.translate(-1, 0);
  pen.setColor(QColor("#E8E8E8"));
  pen.setWidth(1);
  _painter->save();
  _painter->setPen(pen);

  // Always draw both sides
  _painter->drawLine(borderRect.topLeft(), borderRect.bottomLeft());
  //_painter->drawLine(borderRect.topRight(), borderRect.bottomRight());

  // Draw the top line if parent only
  //        if (!_index.parent().isValid())
  //        {
  //            _painter->drawLine(borderRect.topLeft(), borderRect.topRight());
  //        }

  if ((!_index.parent().isValid() && _index.model()->rowCount(_index) == 0) ||
      (_index.parent().isValid() &&
       _index.row() == _index.model()->rowCount(_index.parent()) - 1)) {
    _painter->drawLine(borderRect.bottomLeft(), borderRect.bottomRight());
  }

  if (_index.column() ==
      _index.model()->columnCount() -
          1)  // Last column, draw right vertical line
  {
    _painter->drawLine(borderRect.topRight(), borderRect.bottomRight());
  }

  //        if (!_index.parent().isValid()) //Top line
  //        {
  //            _painter->drawLine(borderRect.topLeft(), borderRect.topRight());
  //        }

  _painter->restore();
}

QSize LedgerWidgetDelegate::sizeHint(const QStyleOptionViewItem& _option,
                                     const QModelIndex& _index) const {
  return QSize(QStyledItemDelegate::sizeHint(_option, _index).width(),
               m_rowHeight);
}

QWidget* LedgerWidgetDelegate::createEditor(QWidget* _parent,
                                            const QStyleOptionViewItem& _option,
                                            const QModelIndex& _index) const {
  if (_index.column() == m_controller->col_date()) {
    QDateEdit* dteEdit = new QDateEdit(_parent);
    Core::setupDateEdit(dteEdit);
    dteEdit->setDate(m_lastUsedDate);
    setCurrentEditor(dteEdit);
  } else if (_index.column() == m_controller->col_payee()) {
    QComboBox* cboPayee = new QComboBox(_parent);
    cboPayee->setModel(new PayeeController(cboPayee));
    cboPayee->setEditable(true);
    QCompleter* comp = new QCompleter(cboPayee->model(), cboPayee);
    comp->setFilterMode(Qt::MatchContains);
    comp->setCaseSensitivity(Qt::CaseInsensitive);
    cboPayee->setCurrentText("");

    setCurrentEditor(cboPayee);
  } else if (_index.column() == m_controller->col_transfer()) {
    const Account* a =
        static_cast<const LedgerController*>(_index.model())->account();
    setCurrentEditor(new AccountSelector(
        Flag_MultipleCurrencies,
        AccountTypeFlags::Flag_All & ~AccountTypeFlags::Flag_Investment,
        a->id(), _parent));
  } else if (_index.column() == m_controller->col_debit() ||
             _index.column() == m_controller->col_credit()) {
    AmountEdit* amtEdit = new AmountEdit(2, _parent);
    amtEdit->setMinimum(0);
    amtEdit->setFocus();
    amtEdit->installEventFilter(_parent);
    setCurrentEditor(amtEdit);
  } else {
    setCurrentEditor(
        QStyledItemDelegate::createEditor(_parent, _option, _index));
  }

  return m_currentEditor;
}

void LedgerWidgetDelegate::setEditorData(QWidget* _editor,
                                         const QModelIndex& _index) const {
  if (_index.column() == m_controller->col_date()) {
    QDate date = _index.model()->data(_index, Qt::EditRole).toDate();
    QDateEdit* dteEdit = static_cast<QDateEdit*>(_editor);
    dteEdit->setDate(date);
  } else if (_index.column() == m_controller->col_payee()) {
    QComboBox* cboPayee = static_cast<QComboBox*>(_editor);
    QString payee = _index.model()->data(_index, Qt::EditRole).toString();
    if (!payee.isEmpty()) {
      int idx = cboPayee->findData(payee, Qt::DisplayRole);

      if (idx != -1) cboPayee->setCurrentIndex(idx);
    }
  } else if (_index.column() == m_controller->col_transfer()) {
    AccountCurrency ac =
        _index.model()->data(_index, Qt::EditRole).value<AccountCurrency>();
    AccountSelector* accSelect = static_cast<AccountSelector*>(_editor);
    accSelect->setCurrentAccount(ac.first, ac.second);
  } else if (_index.column() == m_controller->col_debit() ||
             _index.column() == m_controller->col_credit()) {
    Amount value = Amount::fromUserLocale(
        _index.model()->data(_index, Qt::EditRole).toString());
    AmountEdit* amtEdit = static_cast<AmountEdit*>(_editor);
    amtEdit->setAmount(value);
  } else {
    QStyledItemDelegate::setEditorData(_editor, _index);
  }

  emit editStarted(m_controller->mapToCacheRow(_index));
}
void LedgerWidgetDelegate::setModelData(QWidget* _editor,
                                        QAbstractItemModel* _model,
                                        const QModelIndex& _index) const {
  if (_index.column() == m_controller->col_date()) {
    QDateEdit* dteEdit = static_cast<QDateEdit*>(_editor);
    _model->setData(_index, dteEdit->date(), Qt::EditRole);
    m_lastUsedDate = dteEdit->date();
  } else if (_index.column() == m_controller->col_payee()) {
    QComboBox* cboPayee = static_cast<QComboBox*>(_editor);
    _model->setData(_index, cboPayee->currentText(), Qt::EditRole);
  } else if (_index.column() == m_controller->col_transfer()) {
    AccountSelector* accSelect = static_cast<AccountSelector*>(_editor);

    Account* a = accSelect->currentAccount();
    AccountCurrency c(a ? a->id() : Constants::NO_ID,
                      accSelect->currentCurrency());

    _model->setData(_index, QVariant::fromValue<AccountCurrency>(c),
                    Qt::EditRole);
  } else if (_index.column() == m_controller->col_debit() ||
             _index.column() == m_controller->col_credit()) {
    AmountEdit* amtEdit = static_cast<AmountEdit*>(_editor);
    _model->setData(_index, amtEdit->amount().toString(), Qt::EditRole);
  } else {
    QStyledItemDelegate::setModelData(_editor, _model, _index);
  }
}

void LedgerWidgetDelegate::updateEditorGeometry(
    QWidget* _editor, const QStyleOptionViewItem& _option,
    const QModelIndex& _index) const {
  Q_UNUSED(_index)
  _editor->setGeometry(_option.rect);
}

void LedgerWidgetDelegate::onEditorDestroyed(QObject* _o) {
  if (_o == m_currentEditor) m_currentEditor = nullptr;
}

void LedgerWidgetDelegate::setCurrentEditor(QWidget* _editor) const {
  m_currentEditor = _editor;
  connect(m_currentEditor, &QObject::destroyed, this,
          &LedgerWidgetDelegate::onEditorDestroyed);
}

//    void LedgerWidgetDelegate::commitAndCloseEditor()
//    {
//        if (m_currentEditor) emit commitData(m_currentEditor);
//        m_currentEditor->deleteLater();
//        m_currentEditor = nullptr;
//        //if (m_currentEditor) emit closeEditor(m_currentEditor);
//    }

void LedgerWidgetDelegate::setColumnWidth(LedgerWidget* _view) const {
  _view->setColumnWidth(m_controller->col_status(), 26);
  _view->setColumnWidth(m_controller->col_flag(), 26);
  _view->setColumnWidth(m_controller->col_no(), 60);
  _view->setColumnWidth(m_controller->col_date(), 100);
  _view->setColumnWidth(m_controller->col_memo(), 150);
  _view->setColumnWidth(m_controller->col_payee(), 150);
  _view->setColumnWidth(m_controller->col_cleared(), 26);
  _view->setColumnWidth(m_controller->col_transfer(), 200);
  _view->setColumnWidth(m_controller->col_debit(), 90);
  _view->setColumnWidth(m_controller->col_credit(), 90);
  _view->setColumnWidth(m_controller->col_balance(), 90);

  if (m_controller->col_memo() != -1) {
    _view->header()->setSectionResizeMode(m_controller->col_memo(),
                                          QHeaderView::Stretch);
  }
}
}

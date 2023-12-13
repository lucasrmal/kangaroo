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

#include "ledgerwidget.h"

#include <QAction>
#include <QHeaderView>
#include <QKeyEvent>
#include <QList>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QPen>
#include <QSettings>
#include <QTimer>
#include <QtDebug>
#include <stdexcept>

#include "../../controller/ledger/ledgercontroller.h"
#include "../../controller/ledger/ledgercontrollermanager.h"
#include "../../model/account.h"
#include "../../model/ledger.h"
#include "../../model/modelexception.h"
#include "../../model/transactionmanager.h"
#include "../core.h"
#include "ui/dialogs/formselectaccount.h"
#include "ui/dialogs/optionsdialog.h"

namespace KLib {

//    class ControlOverlay : public QWidget
//    {
//        public:
//            ControlOverlay(QWidget *parent);

//        protected:
//            void paintEvent(QPaintEvent *event);
//    };

//    ControlOverlay::ControlOverlay(QWidget *parent)
//        : QWidget(parent)
//    {
//        setPalette(Qt::transparent);
//        setAttribute(Qt::WA_TransparentForMouseEvents);
//    }

//    void ControlOverlay::paintEvent(QPaintEvent *event)
//    {
//        Q_UNUSED(event)

//        QPainter painter(this);
//        painter.fillRect(rect(), QColor(10,10,10, 50));
//    }

LedgerAction::LedgerAction(QAction* _action, bool _isPrimary,
                           sel_func _selChanged, trig_func _trig)
    : action(_action),
      isPrimary(_isPrimary),
      onSelectionChanged(_selChanged),
      triggered(_trig) {
  QObject::connect(_action, &QAction::triggered, triggered);
}

LedgerWidgetHorizontalHeader::LedgerWidgetHorizontalHeader(
    LedgerController* _controller, QWidget* _parent)
    : QHeaderView(Qt::Horizontal, _parent), m_controller(_controller) {}

void LedgerWidgetHorizontalHeader::paintSection(QPainter* _painter,
                                                const QRect& _rect,
                                                int _logicalIndex) const {
  // Paint the stuff+background first
  _painter->save();
  QHeaderView::paintSection(_painter, _rect, _logicalIndex);
  _painter->restore();

  // If we need to paint a picture...
  if (_logicalIndex == m_controller->col_flag()) {
    //            QRect drawingRect;
    //            drawingRect.setHeight(16);
    //            drawingRect.setWidth(16);
    //            drawingRect.moveCenter(_rect.center());

    //            _painter->drawPixmap(drawingRect,
    //            Core::pixmap("flag-red-16"));

    QPixmap toPaint = Core::pixmap("flag-red-16");

    _painter->save();
    _painter->setRenderHint(QPainter::Antialiasing, true);

    // Scale it to the right size
    //                toPaint = toPaint.scaled(iconSize(),
    //                                         Qt::KeepAspectRatio);

    // Crop it so it's not too big
    int newWidth = toPaint.width();
    int newHeight = toPaint.height();

    if (toPaint.width() > _rect.width() - 4) {
      newWidth = _rect.width() - 4;
    }

    if (toPaint.height() > _rect.height() - 4) {
      newHeight = _rect.height() - 4;
    }

    toPaint = toPaint.copy(0, 0, newWidth, newHeight);

    _painter->drawPixmap(_rect.left() + (_rect.width() - toPaint.width()) / 2,
                         _rect.top() + (_rect.height() - toPaint.height()) / 2,
                         toPaint.width(), toPaint.height(), toPaint);
    _painter->restore();
  }
}

LedgerWidget::LedgerWidget(Account* _account, QWidget* parent)
    : QTreeView(parent),
      m_account(_account),
      m_ledger(_account->ledger()),
      m_controller(
          LedgerControllerManager::instance()->ledgerFor(m_account, this))
// m_overlay(nullptr)
{
  if (!m_controller) {
    throw std::invalid_argument("No ledger for this account!");
  }

  m_controller->setup();

  m_delegate = m_controller->buildDelegate(this);

  setModel(m_controller);
  setItemDelegate(m_delegate);
  setHeader(new LedgerWidgetHorizontalHeader(m_controller, this));

  m_delegate->setColumnWidth(this);
  setSelectionBehavior(QAbstractItemView::SelectRows);
  setSelectionMode(QAbstractItemView::ExtendedSelection);

  setStyleSheet(
      "QTreeView {alternate-background-color: #F1F8FF;background-color: "
      "#F6FCFF; } ");

  setIndentation(0);
  expandAll();
  setExpandsOnDoubleClick(false);

  connect(m_delegate, &LedgerWidgetDelegate::editStarted, this,
          &LedgerWidget::onEditStarted);

  connect(m_controller, &LedgerController::changesDiscarded, this,
          &LedgerWidget::checkActions);
  connect(m_controller, &LedgerController::changesSaved, this,
          &LedgerWidget::checkActions);
  connect(m_controller, &LedgerController::showMessage, this,
          &LedgerWidget::showMessage);

  connect(m_account, &Account::modified, this, &LedgerWidget::checkActions);

  // Actions
  createBasicActions();

  m_controller->addExtraActions(this, [this](int weight, LedgerAction* action) {
    if (action) m_extraActions.insert(weight, action);
  });

  checkEnableActions({});
  createTransactionMenu();

  scrollToBottom();
  setMouseTracking(true);
}

LedgerWidget::~LedgerWidget() { saveColumnWidths(); }

void LedgerWidget::rowsInserted(const QModelIndex& _parent, int _start,
                                int _end) {
  QTreeView::rowsInserted(_parent, _start, _end);

  if (!_parent.isValid())  // New top level
  {
    for (int i = _start; i <= _end; ++i) {
      expand(m_controller->index(i, 0, _parent));
    }
  } else  // New sub row
  {
    expand(_parent);
  }
}

void LedgerWidget::drawRow(QPainter* _painter,
                           const QStyleOptionViewItem& _option,
                           const QModelIndex& _index) const {
  QStyleOptionViewItem newOption = _option;

  if (_index.isValid()) {
    if (m_controller->mapToCacheRow(_index) % 2)  // Odd
    {
      newOption.features =
          newOption.features | (QStyleOptionViewItem::Alternate);
    } else  // Even
    {
      newOption.features =
          newOption.features & (~QStyleOptionViewItem::Alternate);
    }
  }

  QTreeView::drawRow(_painter, newOption, _index);
}

bool LedgerWidget::askSaveDiscard() {
  if (m_controller->hasModifiedRow()) {
    int ans =
        QMessageBox::question(this, tr("Modified Transaction"),
                              tr("A transaction is currently being edited. "
                                 "Save it or discard the modifications?"),
                              QMessageBox::Save | QMessageBox::Discard);

    if (ans == QMessageBox::Save) {
      return tryToSubmit();
    } else if (ans == QMessageBox::Discard) {
      m_controller->discardChanges();
      return true;
    }
  }

  return true;
}

void LedgerWidget::focusOutEvent(QFocusEvent* _event) {
  //        if (m_controller->hasModifiedRow())
  //        {
  //            int ans = QMessageBox::question(this,
  //                                            tr("Modified Transaction"),
  //                                            tr("A transaction is currently
  //                                            being edited. "
  //                                               "Save it or discard the
  //                                               modifications?"),
  //                                            QMessageBox::Save |
  //                                            QMessageBox::Discard);

  //            if (ans == QMessageBox::Save)
  //            {
  //                tryToSubmit();
  //            }
  //            else if (ans == QMessageBox::Discard)
  //            {
  //                m_controller->discardChanges();
  //            }
  //        }

  qDebug() << "Focus out LedgerWidget";
  QTreeView::focusOutEvent(_event);
}

bool LedgerWidget::eventFilter(QObject* _object, QEvent* _event) {
  if (_event && _event->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(_event);
    switch (keyEvent->key()) {
      case Qt::Key_Return:
      case Qt::Key_Tab:
      case Qt::Key_Backtab:
        keyPressEvent(keyEvent);
        return true;
      default:
        break;
    }
  }
  return QObject::eventFilter(_object, _event);
}

bool LedgerWidget::handleTabEvent() {
  // Find the next editable column
  const QModelIndex cur_idx = currentIndex();
  int nextCol = cur_idx.column() + 1;
  const bool wasEdit = m_controller->hasModifiedRow();

  // Find the next column to go to.
  while (!(m_controller->flags(
               m_controller->index(/* row    */ cur_idx.row(),
                                   /* column */ nextCol,
                                   /* parent */ cur_idx.parent())) &
           Qt::ItemIsEditable) &&
         nextCol < m_controller->columnCount()) {
    ++nextCol;
  }

  if (!hasFocus()) {
    setFocus();
  }

  // Depending on the next column...
  if (nextCol < m_controller->columnCount()) {
    QModelIndex next_idx = m_controller->index(/* row    */ cur_idx.row(),
                                               /* column */ nextCol,
                                               /* parent */ cur_idx.parent());
    setCurrentIndex(next_idx);

    // If we are in edit mode, try to edit it.
    if (wasEdit) {
      QTreeView::edit(next_idx);
    }
  } else {
    // Check if we need to go in a sub-index.
    if (wasEdit && !cur_idx.parent().isValid() &&
        m_controller->rowCount(cur_idx))  // Parent to child
    {
      setCurrentIndex(m_controller->firstEditableIndex(
          m_controller->index(/* row    */ 0,
                              /* column */ 0,
                              /* parent */ cur_idx)));
    } else if (wasEdit && cur_idx.parent().isValid() &&
               m_controller->rowCount(cur_idx.parent()) >
                   cur_idx.row() + 1)  // Child to child
    {
      setCurrentIndex(m_controller->firstEditableIndex(
          m_controller->index(/* row    */ cur_idx.row() + 1,
                              /* column */ 0,
                              /* parent */ cur_idx.parent())));
    } else if (!wasEdit  // Otherwise, if there is no modified row, or if there
                         // is and we submit succesfully...
               || tryToSubmit()) {
      const int mainRow =
          cur_idx.parent().isValid() ? cur_idx.parent().row() : cur_idx.row();

      // Note that tryToSubmit will add a row if it's the last, so we're good
      // :-)
      if (mainRow < m_controller->rowCount() - 1) {
        setCurrentIndex(m_controller->firstEditableIndex(
            m_controller->index(/* row    */ mainRow + 1,
                                /* column */ 0)));
      } else if (wasEdit) {
        setCurrentIndex(m_controller->firstEditableIndex(
            m_controller->index(/* row    */ mainRow,
                                /* column */ 0)));
      } else {
        selectionModel()->clear();
      }
    }
  }

  return true;
}

bool LedgerWidget::handleBacktabEvent() {
  // Find the previous editable column
  const QModelIndex cur_idx = currentIndex();
  int prevCol = cur_idx.column() - 1;
  const bool wasEdit = m_controller->hasModifiedRow();

  while (!(m_controller->flags(
               m_controller->index(/* row    */ cur_idx.row(),
                                   /* column */ prevCol,
                                   /* parent */ cur_idx.parent())) &
           Qt::ItemIsEditable) &&
         prevCol >= 0) {
    --prevCol;
  }

  if (!hasFocus()) {
    setFocus();
  }

  // Depending on the next column...
  if (prevCol >= 0) {
    QModelIndex next_idx = m_controller->index(/* row    */ cur_idx.row(),
                                               /* column */ prevCol,
                                               /* parent */ cur_idx.parent());
    setCurrentIndex(next_idx);

    // If we are in edit mode, try to edit it.
    if (wasEdit) {
      QTreeView::edit(next_idx);
    }
  } else {
    // Check if we need to go in a parent index.
    if (wasEdit && cur_idx.parent().isValid() &&
        cur_idx.row() == 0)  // Child to parent
    {
      setCurrentIndex(m_controller->lastEditableIndex(
          m_controller->index(/* row    */ cur_idx.parent().row(),
                              /* column */ 0)));
    } else if (wasEdit && cur_idx.parent().isValid() &&
               cur_idx.row() > 0)  // Child to child
    {
      setCurrentIndex(m_controller->lastEditableIndex(
          m_controller->index(/* row    */ cur_idx.row() - 1,
                              /* column */ 0,
                              /* parent */ cur_idx.parent())));
    } else if (!wasEdit  // Otherwise, if there is no modified row, or if there
                         // is and we submit succesfully...
               || tryToSubmit()) {
      const int mainRow =
          cur_idx.parent().isValid() ? cur_idx.parent().row() : cur_idx.row();

      if (mainRow > 0) {
        setCurrentIndex(m_controller->lastEditableIndex(
            m_controller->index(/* row    */ mainRow - 1,
                                /* column */ 0)));
      } else {
        selectionModel()->clear();
      }
    }
  }

  return true;
}

void LedgerWidget::closeEditor(QWidget* _editor,
                               QAbstractItemDelegate::EndEditHint _hint) {
  if (_hint == QAbstractItemDelegate::EditNextItem) {
    // We will tab ourselves!
    QTreeView::closeEditor(_editor, QAbstractItemDelegate::NoHint);
    handleTabEvent();
  } else if (_hint == QAbstractItemDelegate::EditPreviousItem) {
    // We will tab ourselves!
    QTreeView::closeEditor(_editor, QAbstractItemDelegate::NoHint);
    handleBacktabEvent();
  } else {
    QTreeView::closeEditor(_editor, _hint);
  }
}

void LedgerWidget::onEditStarted(int _row) {
  m_controller->startEdit(_row);
  checkEnableActions(selectedRows());
}

void LedgerWidget::keyPressEvent(QKeyEvent* _event) {
  switch (_event->key()) {
    case Qt::Key_Return:
      if (m_controller->hasModifiedRow()) {
        // Check if an editor is opened, if yes, submit the data.
        if (m_delegate->currentEditor()) {
          commitData(m_delegate->currentEditor());
          closeEditor(m_delegate->currentEditor(),
                      QAbstractItemDelegate::NoHint);
        }

        if (!tryToSubmit()) {
          _event->accept();
          return;
        }

        const int mainRow = currentIndex().parent().isValid()
                                ? currentIndex().parent().row()
                                : currentIndex().row();

        // If at end of table
        if (mainRow == m_controller->newTransactionRow()) {
          _event->accept();
          setCurrentIndex(
              m_controller->index(/* row    */ mainRow,
                                  /* column */ m_controller->col_date()));
          return;
        } else {
          _event->accept();
          selectionModel()->clearSelection();
          return;
        }
      }
      break;

    case Qt::Key_Tab:
      if (handleTabEvent()) {
        _event->accept();
        return;
      }
      break;

    case Qt::Key_Backtab:
      if (handleBacktabEvent()) {
        _event->accept();
        return;
      }
      break;

      //        default:
      //            if (currentIndex().isValid() && edit(currentIndex(),
      //            EditKeyPressed, _event))
      //            {
      //                _event->accept();
      //                return;
      //            }

  }  // switch end

  QTreeView::keyPressEvent(_event);
}

void LedgerWidget::mousePressEvent(QMouseEvent* _event) {
  const QModelIndex index = indexAt(_event->pos());
  const int mainRow =
      index.parent().isValid() ? index.parent().row() : index.row();

  // First, check if we are trying to to to a row other than the current row,
  // try to save the transaction if it is the case.
  if (m_controller->hasModifiedRow() &&
      (!index.isValid() || mainRow != m_controller->modifiedRow())) {
    if (m_delegate->currentEditor()) {
      commitData(m_delegate->currentEditor());
      closeEditor(m_delegate->currentEditor(), QAbstractItemDelegate::NoHint);
    }

    if (!tryToSubmit()) {
      _event->accept();
      return;
    }
    //            else if (row != -1)
    //            {
    //                select(Row(row);
    //            }
    //            else
    //            {
    //                selectionModel()->clear();
    //            }
  }

  // Look if we are clicking on a flag/cleared status column, in which case the
  // row should NOT be selected.
  if (_event->button() == Qt::LeftButton &&
      m_controller->doClickEdition(index)) {
    _event->accept();
    return;
  }

  QTreeView::mousePressEvent(_event);
}

void LedgerWidget::mouseReleaseEvent(QMouseEvent* _event) {
  if (_event && _event->button() == Qt::RightButton &&
      indexAt(_event->pos()).isValid()) {
    // Open popup menu
    m_transactionMenu->popup(mapToGlobal(_event->pos()));
  }

  QTreeView::mouseReleaseEvent(_event);
}

void LedgerWidget::mouseMoveEvent(QMouseEvent* _event) {
  QModelIndex index = indexAt(_event->pos());

  if (m_controller->canClickEdit(index)) {
    setCursor(Qt::PointingHandCursor);
  } else if (cursor().shape() == Qt::PointingHandCursor) {
    unsetCursor();
  }

  QTreeView::mouseMoveEvent(_event);
}

bool LedgerWidget::edit(const QModelIndex& _index, EditTrigger _trigger,
                        QEvent* _event) {
  const int mainRow =
      _index.parent().isValid() ? _index.parent().row() : _index.row();

  if (m_controller->hasModifiedRow() &&
      mainRow != m_controller->modifiedRow()) {
    // Do not allow editing on cells that do not belong to the currently edited
    // row
    return false;
  } else if (QTreeView::edit(_index, _trigger, _event)) {
    // We want to get in Edit mode just as the editor is loaded.
    m_controller->startEdit(mainRow);
    return true;
  } else {
    // QTreeView::edit returned false, so do we!
    return false;
  }
}

void LedgerWidget::selectionChanged(const QItemSelection& _selected,
                                    const QItemSelection& _deselected) {
  QSet<int> selRows, deselectRows;

  // Find unique (main) rows only (we will have num_col indices per row!)
  for (const QModelIndex& i : selectedIndexes()) {
    selRows.insert(i.parent().isValid() ? i.parent().row() : i.row());
  }

  for (const QModelIndex& i : _deselected.indexes()) {
    deselectRows.insert(i.parent().isValid() ? i.parent().row() : i.row());
  }

  if (m_controller->modifiedRow() != -1 &&
      !selRows.contains(m_controller->modifiedRow())) {
    // If we selected a row other than the currently edited row...

    // Try to save the changes
    if (!tryToSubmit()) return;
  }

  // Compute total for selected rows
  Amount tot = 0;

  for (int row : selRows) {
    tot += m_controller->totalForAccount(row);
  }

  QTreeView::selectionChanged(_selected, _deselected);

  // Check actions
  checkEnableActions(selRows.values());

  if (deselectRows != selRows) {
    emit selectedRowsTotalChanged(tot);
    emit selectedRowsChanged(selRows.values());
  }

  // Paint the overlay
  //        if (!m_overlay)
  //        {
  //            m_overlay = new ControlOverlay(this);
  //        }

  //        if (currentIndex().row() >= 0)
  //        {

  //            QRect indexRect = visualRect(currentIndex());

  //            m_overlay->setFixedSize(300, indexRect.height());
  //            m_overlay->move(width() - 301, indexRect.bottom() +
  //            horizontalHeader()->height() + 3);
  //            //m_overlay->setVisible(true);
  //            m_overlay->setVisible(false);
  //        }
  //        else
  //        {
  //            m_overlay->setVisible(false);
  //        }
}

void LedgerWidget::checkEnableActions(const QList<int>& _selectedRows) {
  for (LedgerAction* a : m_basicActions) {
    a->action->setEnabled(a->onSelectionChanged(_selectedRows));
  }
  for (LedgerAction* a : m_extraActions) {
    a->action->setEnabled(a->onSelectionChanged(_selectedRows));
  }
}

void LedgerWidget::createTransactionMenu() {
  m_transactionMenu = new QMenu(this);

  // Add the basic actions
  for (LedgerAction* a : m_basicActions) {
    m_transactionMenu->addAction(a->action);
  }

  m_transactionMenu->addSeparator();

  for (LedgerAction* a : m_extraActions) {
    m_transactionMenu->addAction(a->action);
  }
}

bool LedgerWidget::tryToSubmit() {
  int wrongColumn;
  if (!m_controller->submitChanges(wrongColumn)) {
    // selectRow(oldOnes[0].row());

    QModelIndex idx =
        m_controller->index(/* row    */ m_controller->modifiedRow(),
                            /* column */ wrongColumn);

    setCurrentIndex(idx);
    QTreeView::edit(idx);
    return false;
  }

  return true;
}

void LedgerWidget::onRowsAdded() {
  QTimer::singleShot(1, this, SLOT(scrollToBottom()));
}

void LedgerWidget::createBasicActions() {
  auto askToContinue = [this]() {
    int ans = QMessageBox::question(
        this, tr("Save Changes"),
        tr("The current transaction has unsaved changes. Save it?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Yes);

    switch (ans) {
      case QMessageBox::Yes:
        this->submitEdit();
        return true;
      case QMessageBox::No:
        return true;
      default:
        return false;
    }
  };

  auto askForDelete = [this]() {
    QList<int> rows = selectedRows();
    int num = rows.count();
    int numSchedules = 0;
    int numTransactions = 0;

    // Check if a selected row is the (empty) new transaction row
    for (int i : rows) {
      if (i == newTransactionRow()) {
        --num;
        break;
      } else if (m_controller->rowIsSchedule(i)) {
        ++numSchedules;
      } else {
        ++numTransactions;
      }
    }

    QStringList scheduleOptions;

    scheduleOptions << (numSchedules > 1 ? tr("Skip these occurrences")
                                         : tr("Skip this occurrence"));
    scheduleOptions << (numSchedules > 1
                            ? tr("Skip these occurrences and cancel all future "
                                 "occurences of the schedules")
                            : tr("Skip this occurrence and cancel all future "
                                 "occurences of the schedule"));
    scheduleOptions << (numSchedules > 1 ? tr("Cancel these schedules.")
                                         : tr("Cancel this schedule."));

    if (numTransactions > 0 && numSchedules == 0) {
      if (QMessageBox::question(
              this, tr("Delete Transaction"),
              num == 1 ? tr("Delete this transaction?")
                       : tr("Delete these %1 transactions?").arg(num),
              QMessageBox::Yes | QMessageBox::No,
              QMessageBox::No) == QMessageBox::Yes) {
        deleteSelected(DeleteScheduleAction::NoScheduleAction);
      }
    } else if (numTransactions == 0 && numSchedules > 0) {
      QString title =
          numSchedules > 0 ? tr("Delete Schedules") : tr("Delete Schedule");

      QString msg = tr("What would you like to do?");

      if (numSchedules > 1)
        msg += " " +
               tr("This action will be applied to all %1 selected schedules.")
                   .arg(numSchedules);

      int answer = OptionsDialog::askForOptions(Core::pixmap("trash"), title,
                                                msg, scheduleOptions, 0, this);

      if (answer != -1) {
        deleteSelected((DeleteScheduleAction)answer);
      }
    } else if (numTransactions > 0 && numSchedules > 0) {
      QString title = tr("Delete Selected Items");

      QString msg =
          tr("Would you like to delete the selected transaction(s), and apply "
             "one of the following options"
             "to the selected schedule(s)?");

      int answer = OptionsDialog::askForOptions(Core::pixmap("trash"), title,
                                                msg, scheduleOptions, 0, this);

      if (answer != -1) {
        deleteSelected((DeleteScheduleAction)answer);
      }
    }
  };

  auto askForReassign = [this]() {
    // No schedules may be reaassigned.
    for (int row : selectedRows()) {
      if (m_controller->rowIsSchedule(row)) {
        QMessageBox::warning(this, tr("Reassign Transactions"),
                             tr("Schedules may not be reassign. Un-select "
                                "schedules before continuing."));
        return;
      }
    }

    int new_account_id = Constants::NO_ID;
    if (!FormSelectAccount::selectAccount(
            this, &new_account_id, tr("Select New Account"), Flag_None,
            AccountTypeFlags::Flag_AllButInvTrad)) {
      return;
    }

    if (new_account_id == m_controller->account()->id()) {
      return;
    }

    reassignSelected(new_account_id);
  };

  m_basicActions[BasicLedgerAction::NewTransaction] = new LedgerAction(
      new QAction(Core::icon("new-row"), tr("New &Transaction"), this), true,
      [this](const QList<int>&) { return m_account->isOpen(); },
      [this, askToContinue]() {
        if (!m_controller->hasModifiedRow() || askToContinue()) editNew();
      });

  m_basicActions[BasicLedgerAction::NewBillReminder] = new LedgerAction(
      new QAction(Core::icon("new-bill"), tr("New &Bill Reminder"), this), true,
      [this](const QList<int>&) { return m_account->isOpen(); },
      [this, askToContinue]() {
        if (!m_controller->hasModifiedRow() || askToContinue())
          editNewBillReminder();
      });

  m_basicActions[BasicLedgerAction::NewSchedule] = new LedgerAction(
      new QAction(Core::icon("new-schedule"), tr("New &Schedule"), this), true,
      [this](const QList<int>&) { return m_account->isOpen(); },
      [this, askToContinue]() {
        if (!m_controller->hasModifiedRow() || askToContinue())
          editNewSchedule();
      });

  m_basicActions[BasicLedgerAction::Edit] = new LedgerAction(
      new QAction(Core::icon("edit"), tr("&Edit"), this), true,
      [this](const QList<int>& rows) {
        return m_account->isOpen() && rows.count() == 1 &&
               !rowIsLocked(rows.first());
      },
      [this, askToContinue]() {
        if (!m_controller->hasModifiedRow() || askToContinue()) editCurrent();
      });

  m_basicActions[BasicLedgerAction::Delete] = new LedgerAction(
      new QAction(Core::icon("trash"), tr("&Delete"), this), true,
      [this](const QList<int>& rows) {
        return m_account->isOpen() && rows.count() >= 1 &&
               !rows.contains(newTransactionRow());
      },
      askForDelete);

  m_basicActions[BasicLedgerAction::Reassign] = new LedgerAction(
      new QAction(Core::icon("reassign"), tr("&Reassign"), this), true,
      [this](const QList<int>& rows) {
        return m_account->isOpen() && rows.count() >= 1 &&
               !rows.contains(newTransactionRow());
      },
      askForReassign);

  m_basicActions[BasicLedgerAction::Duplicate] = new LedgerAction(
      new QAction(Core::icon("copy"), tr("D&uplicate"), this), true,
      [this](const QList<int>& rows) {
        return m_account->isOpen() && rows.count() == 1 &&
               rows.first() != newTransactionRow() &&
               !rowIsLocked(rows.first());
      },
      [this]() { duplicateCurrent(); });

  // Extra actions

  LedgerAction* actEnter = new LedgerAction(
      new QAction(Core::icon("enter-modifications"), tr("En&ter"), this), true,
      [this](const QList<int>& rows) {
        return m_account->isOpen() && rows.count() == 1 &&
               (m_controller->rowIsSchedule(rows.first()) ||
                m_controller->hasModifiedRow());
      },
      [this]() {
        if (m_controller->hasModifiedRow()) {
          submitEdit();
        } else  // Must be entering a schedule
        {
          enterSchedule();
        }
      });

  LedgerAction* actDiscard = new LedgerAction(
      new QAction(Core::icon("erase"), tr("&Cancel Edit"), this), true,
      [this](const QList<int>& rows) {
        return m_account->isOpen() && rows.count() == 1 &&
               m_controller->hasModifiedRow();
      },
      [this]() { discardEdit(); });

  LedgerAction* actSchedule = new LedgerAction(
      new QAction(Core::icon("schedule"), tr("Sc&hedule"), this), true,
      [this](const QList<int>& rows) {
        return m_account->isOpen() && rows.count() == 1 &&
               !rowIsLocked(rows.first()) &&
               ((m_controller->hasModifiedRow() &&
                 m_controller->rowIsSchedule(rows.first())) ||
                (!m_controller->hasModifiedRow() &&
                 rows.first() != newTransactionRow()));
      },
      [this]() {
        m_controller->schedule(currentIndex(), m_delegate->lastUsedDate());
      });

  LedgerAction* actAttachments = new LedgerAction(
      new QAction(Core::icon("attachment"), tr("&Attachments"), this), false,
      [this](const QList<int>& rows) {
        return m_account->isOpen() && rows.count() == 1 &&
               !rowIsLocked(rows.first());
      },
      [this]() { m_controller->editAttachments(currentIndex()); });

  LedgerAction* actNote = new LedgerAction(
      new QAction(Core::icon("note"), tr("&Note"), this), false,
      [this](const QList<int>& rows) {
        return m_account->isOpen() && rows.count() == 1 &&
               !rowIsLocked(rows.first());
      },
      [this]() { m_controller->editNote(currentIndex()); });

  m_extraActions.insert(10, actEnter);
  m_extraActions.insert(20, actDiscard);
  m_extraActions.insert(30, actSchedule);
  m_extraActions.insert(100, actAttachments);
  m_extraActions.insert(200, actNote);
}

void LedgerWidget::saveColumnWidths() {
  //        QSettings settings;
  //        QStringList widths;

  //        for (int i = 0; i < model()->columnCount(); ++i)
  //        {
  //            widths << QString::number(columnWidth(i));
  //        }

  //        settings.setValue("Geometry/LedgerWidget/ColumnWidths",
  //        QVariant::fromValue<QList<int> >(widths));
}

bool LedgerWidget::inEdit() const { return m_controller->hasModifiedRow(); }

bool LedgerWidget::inNewEdit() const {
  return m_controller->hasModifiedRow() &&
         currentIndex().row() == m_controller->newTransactionRow();
}

QList<int> LedgerWidget::selectedRows() const {
  QSet<int> rows;

  // Find unique rows only (we will have num_col indices per row!)
  for (const QModelIndex& i : selectedIndexes()) {
    rows.insert(i.parent().isValid() ? i.parent().row() : i.row());
  }

  return rows.values();
}

int LedgerWidget::newTransactionRow() const {
  return m_controller->newTransactionRow();
}

bool LedgerWidget::rowIsLocked(int _row) const {
  return m_controller->rowIsLocked(_row);
}

bool LedgerWidget::rowIsSchedule(int _row) const {
  return m_controller->rowIsSchedule(_row);
}

void LedgerWidget::submitEdit() { tryToSubmit(); }

void LedgerWidget::editCurrent() {
  if (selectedRows().count() == 1) {
    QModelIndex idx =
        m_controller->index(/* row */ currentIndex().parent().isValid()
                                ? currentIndex().parent().row()
                                : currentIndex().row(),
                            /* col */ m_controller->col_date());

    setCurrentIndex(idx);
    QTreeView::edit(idx);
  }
}

void LedgerWidget::editNew() {
  setCurrentIndex(m_controller->index(m_controller->newTransactionRow(),
                                      m_controller->col_date()));
  QTreeView::edit(m_controller->index(m_controller->newTransactionRow(),
                                      m_controller->col_date()));
}

void LedgerWidget::editNewSchedule() {
  setCurrentIndex(m_controller->index(m_controller->newTransactionRow(),
                                      m_controller->col_date()));
  QTreeView::edit(m_controller->index(m_controller->newTransactionRow(),
                                      m_controller->col_date()));
  m_controller->editNewSchedule();
}

void LedgerWidget::editNewBillReminder() {
  setCurrentIndex(m_controller->index(m_controller->newTransactionRow(),
                                      m_controller->col_date()));
  QTreeView::edit(m_controller->index(m_controller->newTransactionRow(),
                                      m_controller->col_date()));
  m_controller->editNewBillReminder();
}

void LedgerWidget::commitAndCloseEditor() {
  if (m_delegate->currentEditor()) {
    commitData(m_delegate->currentEditor());
    closeEditor(m_delegate->currentEditor(), QAbstractItemDelegate::NoHint);
  }
}

void LedgerWidget::noCommitCloseEditor() {
  if (m_delegate->currentEditor()) {
    closeEditor(m_delegate->currentEditor(), QAbstractItemDelegate::NoHint);
  }
}

void LedgerWidget::checkActions() { checkEnableActions(selectedRows()); }

void LedgerWidget::discardEdit() {
  noCommitCloseEditor();
  m_controller->discardChanges();
  selectionModel()->clearSelection();
}

void LedgerWidget::deleteSelected(DeleteScheduleAction _action) {
  if (inEdit()) {
    m_controller->discardChanges();
  }

  // We need to get the transaction IDs BEFORE deleting anything, otherwise
  // the rows might not represent the same transactions again...
  std::vector<int> selected_transactions;
  std::vector<QPair<int, QDate>> selected_schedules_instances;
  QHash<int, QDate> selected_schedules;

  for (int row : selectedRows()) {
    if (row == newTransactionRow()) {
      continue;
    } else if (m_controller->rowIsSchedule(row)) {
      auto item = m_controller->cacheItemAtRow(row);
      selected_schedules_instances.push_back(
          QPair<int, QDate>(item.schedule->id(), item.date()));

      if (!selected_schedules.contains(item.schedule->id())) {
        selected_schedules.insert(item.schedule->id(), item.date());
      } else if (selected_schedules[item.schedule->id()] > item.date()) {
        selected_schedules[item.schedule->id()] = item.date();
      }
    } else {
      selected_transactions.push_back(
          m_controller->transactionAtRow(row)->id());
    }
  }

  // Delete the transactions
  QStringList errors;
  for (int id : selected_transactions) {
    try {
      LedgerManager::instance()->removeTransaction(id);
    } catch (ModelException e) {
      errors << tr("Unable to delete transaction %1: %2")
                    .arg(id)
                    .arg(e.description());
    }
  }

  switch (_action) {
    case DeleteScheduleAction::DeleteSchedule:
      for (int id : selected_schedules.keys()) {
        try {
          ScheduleManager::instance()->remove(id);
        } catch (ModelException e) {
          errors << tr("Unable to delete schedule %1: %2")
                        .arg(id)
                        .arg(e.description());
        }
      }
      break;

    case DeleteScheduleAction::SkipCurrentAndFutureInstances:
      for (auto i = selected_schedules.begin(); i != selected_schedules.end();
           ++i) {
        try {
          Schedule* s = ScheduleManager::instance()->get(i.key());
          Recurrence r = s->recurrence();
          r.stops = true;
          r.lastDate = i.value().addDays(-1);
          s->setRecurrence(r);
        } catch (ModelException e) {
          errors << tr("Unable to delete future occurrences of the schedule "
                       "%1: %2")
                        .arg(i.key())
                        .arg(e.description());
        }
      }
      break;

    case DeleteScheduleAction::SkipInstance:
      for (const QPair<int, QDate>& pair : selected_schedules_instances) {
        try {
          Schedule* s = ScheduleManager::instance()->get(pair.first);
          s->cancelOccurrenceOf(pair.second);
        } catch (ModelException e) {
          errors
              << tr("Unable to delete occurrences of the schedule %1 on %2: %3")
                     .arg(pair.first)
                     .arg(pair.second.toString())
                     .arg(e.description());
        }
      }
      break;

    default:
      break;
  }
  if (!errors.empty()) {
    QMessageBox::warning(
        this, tr("Delete Transactions"),
        tr("The following errors occured:\n\n %1").arg(errors.join("\n")));
  }

  selectionModel()->clearSelection();
}

void LedgerWidget::reassignSelected(int new_account_id) {
  if (inEdit()) {
    m_controller->discardChanges();
  }

  std::vector<int> selected_transactions;
  for (int row : selectedRows()) {
    if (row == newTransactionRow() || m_controller->rowIsSchedule(row)) {
      continue;
    }
    selected_transactions.push_back(m_controller->transactionAtRow(row)->id());
  }

  const int ledger_account_id = m_controller->account()->id();
  QStringList errors;
  for (int transaction_id : selected_transactions) {
    Transaction* transaction =
        TransactionManager::instance()->get(transaction_id);
    QList<KLib::Transaction::Split> splits = transaction->splits();
    bool modified = false;
    for (Transaction::Split& split : splits) {
      if (split.idAccount == ledger_account_id) {
        split.idAccount = new_account_id;
        modified = true;
      }
    }

    if (modified) {
      try {
        transaction->setSplits(splits);
      } catch (ModelException e) {
        errors << tr("Unable to update transaction %1: %2")
                      .arg(transaction_id)
                      .arg(e.description());
      }
    }
  }
  if (!errors.empty()) {
    QMessageBox::warning(
        this, tr("Delete Transactions"),
        tr("The following errors occured:\n\n %1").arg(errors.join("\n")));
  }
}

void LedgerWidget::duplicateCurrent() {
  if (selectedRows().count() == 1 &&
      m_controller->duplicateTransaction(currentIndex(),
                                         m_delegate->lastUsedDate())) {
    QTimer::singleShot(1, this, SLOT(scrollToBottom()));
    QTreeView::edit(m_controller->index(m_controller->newTransactionRow(),
                                        m_controller->col_date()));
  }
}

void LedgerWidget::enterSchedule() {
  if (selectedRows().count() == 1) {
    m_controller->enterSchedule(currentIndex());
  }
}
}  // namespace KLib

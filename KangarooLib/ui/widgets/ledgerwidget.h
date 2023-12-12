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

#ifndef LEDGERWIDGET_H
#define LEDGERWIDGET_H

#include <QHeaderView>
#include <QItemDelegate>
#include <QTreeView>
#include <functional>

#include "../../amount.h"

namespace KLib {
class Account;
class Ledger;
class LedgerController;
class AmountEdit;
//    class ControlOverlay;
class LedgerWidgetDelegate;

enum class DeleteScheduleAction;

/**
 * @brief Standard action across all ledgers
 */
enum class BasicLedgerAction {
  NewTransaction,
  NewBillReminder,
  NewSchedule,
  Edit,
  Delete,
  Duplicate,
  Reassign
};

struct LedgerAction {
  typedef std::function<bool(const QList<int>&)> sel_func;
  typedef std::function<void()> trig_func;

  LedgerAction(QAction* _action, bool _isPrimary, sel_func _selChanged,
               trig_func _trig);

  QAction* action;  ///< The action

  bool isPrimary;

  /**
   * @brief Called when selection is changed, will activate or desactivate the
   * action depending on the return value.
   *
   * First param: Rows selected
   * Second param: LedgerController
   */
  sel_func onSelectionChanged;

  /**
   * @brief This function will be called when the action is triggered.
   *
   * First param: Ledger Widget
   * Second param: LedgerController
   */
  trig_func triggered;
};

class LedgerWidgetHorizontalHeader : public QHeaderView {
  Q_OBJECT

 public:
  LedgerWidgetHorizontalHeader(LedgerController* _controller, QWidget* _parent);

  void paintSection(QPainter* _painter, const QRect& _rect,
                    int _logicalIndex) const;

 private:
  LedgerController* m_controller;
};

class LedgerWidget : public QTreeView {
  Q_OBJECT

 public:
  explicit LedgerWidget(KLib::Account* _account, QWidget* parent = nullptr);

  virtual ~LedgerWidget();

  /**
   * @return Returns if a transaction is currently being edited (including new
   * transactions).
   */
  bool inEdit() const;

  /**
   * @return Returns if a new transaction is currently being edited.
   */
  bool inNewEdit() const;

  /**
   * @brief Returns the (main) rows that are currently selected in the ledger.
   * This is NOT guaranteed to be sorted.
   * @return
   */
  QList<int> selectedRows() const;

  /**
   * @brief Returns the (main) row in the ledger that corresponds to the new
   * (blank) transaction.
   * @return
   */
  int newTransactionRow() const;

  /**
   * @return If the transaction at row _row is locked.
   *
   * @param _row The main row (not a sub-row)
   */
  bool rowIsLocked(int _row) const;

  /**
   * @return If the transaction at row _row is a schedule.
   *
   * @param _row The main row (not a sub-row)
   */
  bool rowIsSchedule(int _row) const;

  QList<LedgerAction*> basicActions() const { return m_basicActions.values(); }
  QList<LedgerAction*> extraActions() const { return m_extraActions.values(); }

  LedgerAction* basicAction(BasicLedgerAction _action) const {
    return m_basicActions.value(_action, nullptr);
  }

 public slots:
  /**
   * @brief Submits the changes of the current modified row.
   */
  void submitEdit();

  /**
   * @brief Starts editing the current row.
   *
   * Only works if a single row is selected.
   */
  void editCurrent();

  /**
   * @brief Starts editing the new row.
   */
  void editNew();

  /**
   * @brief Starts editing the new row as a schedule
   */
  void editNewSchedule();

  /**
   * @brief Starts editing the new row as a bill reminder
   */
  void editNewBillReminder();

  /**
   * @brief Discards the changes and returns the current row to non-edit mode.
   */
  void discardEdit();

  /**
   * @brief Deletes the selected rows.
   */
  void deleteSelected(DeleteScheduleAction _action);

  /**
   * @brief Reassign the selected rows to a different account.
   */
  void reassignSelected(int new_account_id);

  /**
   * @brief Duplicates the current row.
   *
   * Only works if a single row is selected.
   */
  void duplicateCurrent();

  /**
   * @brief Enters the currently selected schedule.
   *
   * Only works if a single row is selected and that row is a scheduled
   * transaction.
   */
  void enterSchedule();

  void commitAndCloseEditor();
  void noCommitCloseEditor();

  void checkActions();

 protected:
  void keyPressEvent(QKeyEvent* _event) override;
  void focusOutEvent(QFocusEvent* _event) override;

  void mousePressEvent(QMouseEvent* _event) override;

  void mouseMoveEvent(QMouseEvent* _event) override;

  void mouseReleaseEvent(QMouseEvent* _event) override;

  bool edit(const QModelIndex& _index, EditTrigger _trigger,
            QEvent* _event) override;

  bool askSaveDiscard();

  void rowsInserted(const QModelIndex& _parent, int _start, int _end) override;

  void drawRow(QPainter* _painter, const QStyleOptionViewItem& _option,
               const QModelIndex& _index) const override;

  bool eventFilter(QObject* _object, QEvent* _event) override;

 signals:
  void selectedRowsTotalChanged(const KLib::Amount& _amount);
  void selectedRowsChanged(const QList<int>& _rows);

  void showMessage(const QString& _message);

 protected slots:
  void selectionChanged(const QItemSelection& _selected,
                        const QItemSelection& _deselected) override;
  void onRowsAdded();

  void closeEditor(QWidget* _editor,
                   QAbstractItemDelegate::EndEditHint _hint) override;

  void onEditStarted(int _row);

 private:
  void createBasicActions();
  void checkEnableActions(const QList<int>& _selectedRows);
  void createTransactionMenu();
  void saveColumnWidths();

  bool tryToSubmit();

  bool handleTabEvent();
  bool handleBacktabEvent();

  QMap<BasicLedgerAction, LedgerAction*> m_basicActions;
  QMultiMap<int, LedgerAction*> m_extraActions;  // Sorted by weight.

  Account* m_account;
  Ledger* m_ledger;
  LedgerController* m_controller;
  LedgerWidgetDelegate* m_delegate;
  QMenu* m_transactionMenu;
};
}  // namespace KLib

#endif  // LEDGERWIDGET_H

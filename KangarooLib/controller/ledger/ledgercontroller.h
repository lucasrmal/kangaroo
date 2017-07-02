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

#ifndef LEDGERCONTROLLER_H
#define LEDGERCONTROLLER_H

#include <QAbstractItemModel>
#include <QDate>
#include <functional>
#include <QStyledItemDelegate>
#include "ledgertransactioncache.h"
#include "../../amount.h"
#include "../../model/transaction.h"
#include "../../model/schedule.h"
#include "../../klib.h"

namespace KLib
{
    class Ledger;
    class LedgerWidgetDelegate;
    class LedgerController;
    class LedgerWidget;
    class LedgerAction;
    class LedgerTransactionCache;

    typedef QPair<int, QString> AccountCurrency;

    class LedgerBuffer : public QObject
    {
        Q_OBJECT

        public:

            //Row
            int         row;

            //Transaction infos
            QString     no;
            bool        flagged;
            int         clearedStatus;
            QDate       date;
            QString     memo;
            QString     payee;
            int         idTransfer;
            Amount      debit;
            Amount      credit;
            QString     note;
            QSet<int>   attachments;
            QList<Transaction::Split> splits;

            //Multi currency stuff
            bool        multiCurrency;
            Amount      exchTransfer;
            QString     transferCurrency;

            //Schedule
            bool        isSchedule;
            Schedule    schedule;
            int         idSchedule;

            //Methods
            virtual void clear();

            virtual void makeSchedule();

            virtual bool setData(int _column, int _row, const QVariant& _value, LedgerController* _controller);
            virtual QVariant data(int _column, int _row, bool _editRole, const LedgerController* _controller) const;

            virtual void load(const Transaction* _transaction, const LedgerController* _controller);
            virtual void load(const Schedule* _schedule, const LedgerController* _controller);

            //virtual void loadFromSplits(const QList<Transaction::Split>& _splits, const LedgerController* _controller);

            virtual QStringList validate(int& _firstErrorColumn, const LedgerController* _controller);

            virtual QList<Transaction::Split> splitsForSaving(LedgerController* _controller, bool& _ok) const;
            virtual QHash<QString, QVariant> propertiesForSaving(LedgerController* _controller) const;

            /**
             * @brief Saves the changes to the transaction.
             *
             * This transaction may be a newly created transaction, or it may be an existing transaction.
             * It may also be a scheduled transaction.
             *
             * Do not catch ModelExceptions, they will be caught and handled by the LedgerController.
             *
             * @param _transaction
             * @return True if succesfull, False otherwise.
             */
            virtual bool saveToTransaction(Transaction* _transaction, LedgerController* _controller);

            virtual Transaction* createTransaction() const { return new Transaction(); }

            virtual Amount totalForAccount(const LedgerController* _controller) const;

            virtual void changeToSplitTransaction(const LedgerController* _controller);

            virtual int rowCount() const;

            virtual void removeRowAt(int _row);

            /**
             * @brief Saves the buffer to a new/modified transaction. Must clear the buffer before leaving if added succesfully.
             * Do not catch ModelException; it will be caught and handled by LedgerController.
             * @param _controller The controller
             * @return True if changes saved succesfully, false otherwise
             */
            virtual bool save(LedgerController* _controller);

            LedgerBuffer() { clear(); }

            virtual ~LedgerBuffer() {}

            static const int NO_ROW;

        signals:
            void rowInserted(int _row);
            void rowRemoved(int _row);
            void showMessage(const QString& _message);

        protected:
            void showImbalances(const QList<Transaction::Split>& splits);

            virtual void loadSplits(const QList<Transaction::Split>& _splits,
                                    const LedgerController* _controller);

            virtual bool rowIsEmpty(int _row);
            virtual void ensureOneEmptyRow();

            virtual void insertRowAt(int _row);

            virtual void removeRow(int _row);

    };

    enum class CacheItemType
    {
        PlainTransaction,
        LockedTransaction,
        FutureSchedule,
        OverdueSchedule,
        LockedSchedule,
        BillReminder,
        OverdueBill,
        LockedBill
    };

    enum class DeleteScheduleAction
    {
        SkipInstance,
        SkipCurrentAndFutureInstances,
        DeleteSchedule,

        NoScheduleAction = -1
    };

    class LedgerController : public QAbstractItemModel
    {
        Q_OBJECT

        protected:
            explicit LedgerController(Ledger* _ledger, LedgerBuffer* _buffer, QObject* _parent = nullptr);

        public:
            virtual         ~LedgerController();

            virtual void setup();

            /**
             * @brief This function will be called by LedgerWidget to ask the controller if any extra actions are provided by this controller.
             *
             * Parameters: the LedgerWidger, and a function int, LedgerAction* : first param is the weight, second is the action to add.
             *
             * Three extra actions are already defined:
             * - Enter, with weight 10,
             * - Discard Edit, with weight 20
             * - Schedule, with weight 30
             * - Attachments, with weight 100
             * - Note, with weight 200
             *
             * Heavier actions are to the right/bottom.
             */
            virtual void addExtraActions(LedgerWidget*, std::function<void(int, LedgerAction*)>) {}

            // --------------------- Ledger stuff ---------------------

            virtual LedgerWidgetDelegate* buildDelegate(LedgerWidget* _widget) const;

            Account*    account() const;
            Ledger*     ledger() const;

            virtual bool    duplicateTransaction(const QModelIndex& _index, const QDate& _date = QDate());

            virtual bool    enterSchedule(const QModelIndex& _index);

            virtual void    editNote(const QModelIndex& _index);

            virtual void    editAttachments(const QModelIndex& _index);

            /**
             * @brief Creates a schedule from the transaction or opens the schedule dialog
             * @param _index Index of transaction
             * @param _date If creating a new schedule, date at which the schedule will start. Otherwise,
             * this is ignored.
             *
             * If in edit and currently edited transaction is a schedule, opens the schedule dialog.
             *
             * Otherwise, if not in edit and if index is a transaction, creates a new schedule from it.
             *
             * Otherwise, if not in edit and if index is a transaction instance, opens the schedule dialog to
             * edit the schedule.
             */
            virtual bool    schedule(const QModelIndex& _index, const QDate& _date = QDate());


            virtual bool    doClickEdition(const QModelIndex& _index);
            virtual bool    canClickEdit(const QModelIndex& _index) const;

            virtual bool    canEditColumn(int _column) const;

            /**
             * @brief Returns first editable index in the same row (main or sub) as _index.
             */
            virtual QModelIndex     firstEditableIndex(const QModelIndex& _index) const = 0;

            /**
             * @brief Returns last editable index in the same row (main or sub) as _index.
             */
            virtual QModelIndex     lastEditableIndex(const QModelIndex& _index) const = 0;

            int     modifiedRow() const     { return m_buffer->row; }
            bool    hasModifiedRow() const  { return m_buffer->row != LedgerBuffer::NO_ROW; }

            const Transaction*  transactionAtRow(int _row) const { return m_cache[_row].transaction(); }
            CacheItem&          cacheItemAtRow(int _row) { return m_cache[_row]; }
            virtual bool        rowIsSchedule(int _row) const;

            Amount          totalForAccount(int _row) const;

            virtual int     newTransactionRow() const;

            QString         dateFormat() const { return m_dateFormat; }
            QWidget*        parentWidget() const { return m_parentWidget; }
            const Balances& balance(int _row) const { return m_cache.balanceAt(_row); }

            virtual bool    rowIsLocked(int _row) const;

            int             accountCurrencyPrecision() const { return m_precision; }



            // --------------------- Row stuff ---------------------
            virtual int col_status() const = 0;     ///< Status icons of transaction
            virtual int col_flag() const = 0;       ///< Flagged status
            virtual int col_no() const = 0;         ///< Check #
            virtual int col_date() const = 0;       ///< Date
            virtual int col_memo() const = 0;       ///< Memo
            virtual int col_payee() const = 0;      ///< Payee
            virtual int col_cleared() const = 0;    ///< Cleared status (Cleared/Reconciled/Empty)
            virtual int col_transfer() const = 0;   ///< Other part of transfer, or splits
            virtual int col_debit() const = 0;      ///< Debit to this account
            virtual int col_credit() const = 0;     ///< Credit to this account
            virtual int col_balance() const = 0;    ///< Current balance

            virtual bool alignRight(int _column) const = 0;
            virtual bool alignCenter(int _column) const;


            // --------------------- TableModel stuff ---------------------

            int             rowCount(const QModelIndex& _parent = QModelIndex()) const override;
            QVariant        data(const QModelIndex& _index, int _role) const override;

            bool            setData(const QModelIndex& _index,
                                    const QVariant& _value,
                                    int _role = Qt::EditRole) override;

            QVariant        headerData(int _section,
                                       Qt::Orientation _orientation,
                                       int _role = Qt::DisplayRole) const override;

            QModelIndex     index(int _row, int _column,
                                  const QModelIndex& _parent = QModelIndex()) const override;

            QModelIndex     parent(const QModelIndex& _child) const override;

            int             accountHeightDisplayed() const { return m_accountHeightDisplayed; }

            QString         formatCurrency(const Amount& _amount, const QString& _currency) const;

            int             mapToCacheRow(const QModelIndex& _index) const;

            virtual bool    canRemoveSplitAt(const QModelIndex& _index);


            static const QString SPLIT_TEXT;

        public slots:
            virtual bool submitChanges(int &_firstErrorColumn);
            virtual void discardChanges();

            /**
             * @brief Starts editing the transaction as a split transaction.
             * @param _index The index to edit
             *
             * If currently in edit, _index must correspond to the current edited row. Will not do anything
             * if the transaction currently edited is a split.
             *
             * If not currently in edit, starts editing the transaction at _index as a split transaction.
             */
            virtual void editSplits(const QModelIndex& _index);

            virtual bool startEdit(int _row);

            /**
             * @brief Starts editing a new schedule.
             *
             * Must not be currently in any other edit. Will start editing a new schedule in the New Transaction row.
             *
             * Returns True if started editing new schedule successfully.
             */
            virtual bool editNewSchedule();

            virtual bool editNewBillReminder();

            virtual void removeSplitAt(const QModelIndex& _index);

        signals:
            void changesSaved();
            void changesDiscarded();
            void editExistingStarted();
            void editNewStarted();

            void editStarted(int _row);
            void editFinished(int _row);

            void requestCheckActions() const;

            void showMessage(const QString& _message);

        protected slots:
            virtual void onBufferRowInserted(int _row);
            virtual void onBufferRowRemoved(int _row);

            virtual void onSettingsChanged(const QString& _key);

        protected:
            /**
             * @brief Returns a list of all the splits to be displayed for this transaction.
             * @param _tr The transaction
             *
             * The first split <b>MUST</b> be the split related to this transaction.
             *
             * In the base implementation (this class), this method will be used by cacheData
             * to display the splits and by subRowCount to count the number of split rows to display.
             */
            virtual QList<Transaction::Split> displayedSplits(const Transaction* _tr) const;

            /**
             * @brief Data for an existing transaction
             * @param _column    The column
             * @param _cacheRow  The transaction's position in the cache
             * @param _row       The row (main=0, secondary rows start at 1)
             * @param _editRole  If the role is Edit (true) or Display (false)
             */
            virtual QVariant    cacheData(int _column, int _cacheRow, int _row, bool _editRole) const;

            virtual int         cacheSubRowCount(int _cacheRow) const;
            virtual int         subRowCount(const Transaction* _tr) const;
            virtual void        loadBuffer(int _row);

            virtual QString     formatBalances(int _cacheRow) const;

            /**
             * @brief Balance for cached transaction at row _cacheRow.
             * @param _cacheRow Row, will ALWAYS be a valid cache row (not newTransactionRow()).
             */
            virtual Balances    cacheBalance(int _cacheRow) const;

            /**
             * @brief Checks if the transaction at row _row can be edited by this ledger. Ex: investment transactions
             * cannot be edited by GenericLedgerController.
             * @param _row The row, no need to check, it is valid.
             */
            virtual bool        canEditTransaction(const Transaction* _transaction, QString* _message = nullptr) const = 0;

            bool                canEdit(int _row, QString *_message = nullptr) const;

            Ledger* m_ledger;
            QWidget* m_parentWidget;

            LedgerBuffer* m_buffer;
            LedgerTransactionCache m_cache;

            int m_precision;

            friend class LedgerTransactionCache;

        public:
            //Settings section
            QString m_dateFormat;
            int m_accountHeightDisplayed;
            bool m_askBeforeAddingNewPayee;
            QFont m_defaultFont;
            bool m_expandAllSplits;

    };

    class LedgerWidgetDelegate : public QStyledItemDelegate
    {
        Q_OBJECT

        public:
            LedgerWidgetDelegate(const LedgerController* _controller,
                                 LedgerWidget* _widget);

            virtual ~LedgerWidgetDelegate() {}

            /* OVERLOADED VIRTUAL METHODS */

            void        paint(QPainter* _painter,
                              const QStyleOptionViewItem& _option,
                              const QModelIndex& _index) const override;

            QWidget*    createEditor(QWidget* _parent,
                                     const QStyleOptionViewItem& _option,
                                     const QModelIndex& _index) const override;

            void        setEditorData(QWidget* _editor,
                                      const QModelIndex& _index) const;

            void        setModelData(QWidget* _editor,
                                     QAbstractItemModel* _model,
                                     const QModelIndex& _index) const override;

            QSize       sizeHint(const QStyleOptionViewItem& _option,
                                 const QModelIndex& _index) const override;

            void        updateEditorGeometry(QWidget* _editor,
                                             const QStyleOptionViewItem& _option,
                                             const QModelIndex& _index) const override;

            /* VIRTUAL METHODS */

            virtual QSize   iconSize() const { return QSize(16,16); }

            virtual void    setColumnWidth(LedgerWidget* _view) const;

            /* NON-VIRTUAL METHODS */

            QWidget* currentEditor() const      { return m_currentEditor; }

            static const QDate& lastUsedDate()  { return m_lastUsedDate; }

        signals:
            void editStarted(int _row) const;

        private slots:
            void onEditorDestroyed(QObject* _o);

        protected:
            /**
             * @brief Subclasses that allow other kinds of editors MUST set the current editor using this method.
             * @param _editor The editor
             */
            void setCurrentEditor(QWidget* _editor) const;

            const LedgerController* m_controller;
            const LedgerWidget* m_widget;

        private:
            int m_rowHeight;
            mutable QWidget* m_currentEditor;
            static QDate m_lastUsedDate;
    };

}

Q_DECLARE_METATYPE(KLib::AccountCurrency)

#endif // LEDGERCONTROLLER_H

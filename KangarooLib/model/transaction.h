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

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include <QDate>
#include <QLinkedList>
#include <QSet>
#include <QScriptEngine>
#include "stored.h"
#include "properties.h"
#include "../amount.h"
#include "../util/balances.h"

namespace KLib
{
/*
 *
 * ASSET, EXPENSE: debits increase the balance and
 *                 credits decrease the balance.
 *
 * LIABILITY, EQUITY, INCOME credits increase the balance
 *                           debits decrease the balance.
 *
 * POSITIVE amount: DEBIT
 * NEGATIVE amount: CREDIT
 *
 * */

    class Account;

    namespace ClearedStatus
    {
        enum Status
        {
            None = 0,
            Cleared,
            Reconciled,
            NumStatus
        };

        QString statusToDisplay(int _status);
    }

    class Transaction : public IStored
    {
        Q_OBJECT

        Q_PROPERTY(int splitCount READ splitCount)
        Q_PROPERTY(QString no READ no WRITE setNo)
        Q_PROPERTY(QDate date READ date WRITE setDate)
        Q_PROPERTY(QString memo READ memo WRITE setMemo)
        Q_PROPERTY(int idPayee READ idPayee WRITE setIdPayee)

    public:

        class Split
        {
        public:

            Split(const KLib::Amount& _amount, int _idAccount, const QString& _currency, const QString& _memo = QString()) :
                amount(_amount),
                idAccount(_idAccount),
                currency(_currency),
                memo(_memo) {}

            Split() : idAccount(Constants::NO_ID) {}

            KLib::Amount amount;
            int idAccount;
            QString currency;
            QString memo;
            QString userData;

            QString formattedAmount() const;
            QString invertedFormattedAmount() const;

            static QScriptValue toScriptValue(QScriptEngine *engine, const Split &s)
            {
              QScriptValue obj = engine->newObject();
              obj.setProperty("amount", s.amount.toDouble());
              obj.setProperty("memo", s.memo);
              obj.setProperty("idAccount", s.idAccount);
              obj.setProperty("currency", s.currency);
              obj.setProperty("userData", s.userData);
              return obj;
            }

            static void fromScriptValue(const QScriptValue &obj, Split &s)
            {
              s.amount = obj.property("amount").toNumber();
              s.memo = obj.property("memo").toString();
              s.idAccount = obj.property("idAccount").toInt32();
              s.currency = obj.property("currency").toString();
              s.userData = obj.property("userData").toString();
            }
        };


        Transaction();
        virtual ~Transaction();

        /**
         * @brief Creates a copy of the transaction
         * @return
         */
        virtual Transaction* copyTo(int _idTo = Constants::NO_ID) const;

        /**
         * @brief Check number/transaction method (ex: ATM, POS)
         */
        const QString& no() const { return m_no; }

        /**
         * @brief Cleared status
         * @see ClearedStatus::Status
         */
        int clearedStatus() const { return m_clearedStatus; }

        /**
         * @brief Flag status of the transaction
         */
        bool isFlagged() const { return m_flagged; }

        /**
         * @brief Transaction date
         */
        const QDate& date() const { return m_date; }

        /**
         * @brief Transaction memo (short description)
         */
        const QString& memo() const { return m_memo; }

        /**
         * @brief Note (long description)
         */
        const QString& note() const { return m_note; }

        /**
         * @brief Payee id, or Constant::NO_ID if no payee
         */
        int idPayee() const { return m_idPayee; }

        /**
         * @brief Attachments
         * @return List of ID of the documents attached to this transaction.
         */
        QSet<int> attachments() const { return m_attachments; }

        virtual QString autoMemo() const { return m_memo; }

        /**
         * @brief Sets the check number
         * @see no()
         * @param _no
         */
        virtual void setNo(const QString& _no);

        /**
         * @brief Sets the cleared status
         * @see clearedStatus()
         * @param _status The status. Must be valid, otherwise will throw a ModelException.
         */
        virtual void setClearedStatus(int _status);

        /**
         * @brief Sets the transaction flagged (true) or not (false)
         * @see flagged()
         * @param _flagged
         */
        virtual void setFlagged(bool _flagged);

        /**
         * @brief Sets the transaction date.
         * @see date()
         * @param _date The date, must be valid.
         */
        virtual void setDate(const QDate& _date);

        /**
         * @brief Sets the memo (short description)
         * @see memo()
         * @param _memo
         */
        void setMemo(const QString& _memo);

        /**
         * @brief Sets the note (long description)
         * @see note()
         * @param _note
         */
        void setNote(const QString& _note);

        /**
         * @brief Sets the payee id
         * @param _idPayee The payee id (must exist or will throw ModelException), or Constants::NO_ID if no payee
         */
        void setIdPayee(int _idPayee);

        /**
         * @brief Sets the list of document IDs attached to this transaction.
         *
         * Documents no longer attached to the transaction will be deleted.
         * The list of attachments passed must be valid. An exception will be throwed if this is not the case.
         */
        void setAttachments(const QSet<int>& _attachments);

        Q_INVOKABLE KLib::Properties* properties() const { return m_properties; }

        Q_INVOKABLE virtual const QList<KLib::Transaction::Split>& splits() const { return m_splits; }
        Q_INVOKABLE virtual KLib::Transaction::Split split(int _i) const;

        Q_INVOKABLE virtual void setSplits(const QList<KLib::Transaction::Split>& _splits);
        virtual int  splitCount() const { return m_splits.length(); }

        Q_INVOKABLE virtual bool relatedTo(int _idAccount) const;
        Q_INVOKABLE virtual KLib::Balances totalFor(int _account) const;
        Q_INVOKABLE virtual KLib::Amount totalForInAccountCurrency(int _account) const;
        Q_INVOKABLE virtual KLib::Amount totalForInMainCurrency(int _account) const;


        /**
          @brief Returns if the transaction is a currency exchange.

          A transaction is a currency exchange if it is a transfer from one account to another,
          with the accounts using distinct currencies, such that the currency transfer goes through 2 trading accounts.
          Thus, the transaction has exactly 4 splits, 2 of them being with trading accounts.
        */
        Q_INVOKABLE virtual bool isCurrencyExchange() const { return m_isCurrencyExchange; }

        /**
         * @return A custom color to use for the transaction, or empty string to use the defaults.
         */
        virtual QString transactionColor() const { return ""; }

        static bool splitsBalance(const QList<KLib::Transaction::Split>& _splits);

        /**
         * @brief Total in main account currency
         */
        static Amount totalForAccount(int _idAccount, const QList<KLib::Transaction::Split>& _splits);
        static Balances totalsForAccount(int _idAccount, const QList<KLib::Transaction::Split>& _splits);
        static QString splitsImbalances(const QList<KLib::Transaction::Split>& _splits);
        static bool isCurrencyExchange(const QList<KLib::Transaction::Split>& _splits);

        static void addTradingSplits(QList<KLib::Transaction::Split>& _splits);

    signals:
        void splitAdded(const KLib::Transaction::Split& _split);
        void splitRemoved(const KLib::Transaction::Split& _split);
        void splitAmountChanged(const KLib::Transaction::Split& _split);
        void splitMemoChanged(const KLib::Transaction::Split& _split);

        void transactionDateChanged(const QDate& _new);

    protected:
        void load(QXmlStreamReader& _reader) override;
        void save(QXmlStreamWriter& _writer) const override;

        virtual void copyTo(Transaction* _other) const;

        void checkIfCurrencyExchange();

        QString      m_no;
        int          m_clearedStatus;
        bool         m_flagged;
        QDate        m_date;
        QString      m_memo;
        int          m_idPayee;
        bool         m_isCurrencyExchange;
        QList<Split> m_splits;
        QSet<int>    m_attachments;
        Properties*  m_properties;
        QString      m_note;

        friend class Schedule;
        friend class LedgerManager;
        friend class TransactionManager;
    };

}

Q_DECLARE_METATYPE(KLib::Transaction*)
Q_DECLARE_METATYPE(KLib::Transaction::Split)
Q_DECLARE_METATYPE(QList<KLib::Transaction*>)
Q_DECLARE_METATYPE(QLinkedList<KLib::Transaction*>)

#endif // TRANSACTION_H

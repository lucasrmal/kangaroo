#ifndef BILLREMINDER_H
#define BILLREMINDER_H

#include <QAbstractListModel>
#include <QDate>
#include <KangarooLib/amount.h>
//#include <KangarooLib/model/transaction.h>
#include "../ihomewidget.h"
#include "../../AccountCreateEdit/accountedittab.h"

class QListView;
class QCheckBox;

namespace KLib
{
    class Account;
    class Schedule;
    class AmountEdit;
}

class WarningBillReminderModel : public QAbstractListModel
{
    Q_OBJECT

        enum class EventType
        {
            Warning_FutureLowBalance,
            Warning_CurrentLowBalance,

            Warning_FutureHighBalance,
            Warning_CurrentHighBalance,

            Warning_FutureOverCreditLimit,
            Warning_CurrentOverCreditLimit,

            BillDueReminder
        };

        struct Event
        {
            Event(EventType _type, const QDate& _date, const KLib::Amount& _amount, const KLib::Account* _account) :
                type(_type),
                date(_date),
                amount(_amount),
                relatedAccount(_account),
                relatedSchedule(nullptr) {}

            Event(EventType _type, const QDate& _date, const KLib::Schedule* _schedule) :
                type(_type),
                date(_date),
                relatedAccount(nullptr),
                relatedSchedule(_schedule) {}

            EventType    type;
            QDate        date;
            KLib::Amount amount;

            const KLib::Account* relatedAccount;
            const KLib::Schedule* relatedSchedule;
        };

    public:
        explicit    WarningBillReminderModel(QObject* _parent = nullptr);

        int         rowCount(const QModelIndex& = QModelIndex()) const { return m_events.count(); }
        QVariant    data(const QModelIndex& _index, int _role) const;

        static const QString LOW_BALANCE_PROPERTY;
        static const QString HIGH_BALANCE_PROPERTY;
        static const QString OVERLIMIT_PROPERTY;

    public slots:
        void refresh();

    private slots:
//        //Transaction signals
//        void onSplitAdded(const KLib::Transaction::Split& _split, KLib::Transaction* _tr);
//        void onSplitRemoved(const KLib::Transaction::Split& _split, KLib::Transaction* _tr);
//        void onSplitAmountChanged(const KLib::Transaction::Split& _split, KLib::Transaction* _tr);
//        void onTransactionDateChanged(KLib::Transaction* _tr, const QDate& _old);

//        //Schedule signals
//        void onScheduleAdded(KLib::Schedule* s);
//        void onScheduleRemoved(KLib::Schedule* s);
//        void onScheduleModified(KLib::Schedule* s); ///< @todo What if transaction changes...

//        void onScheduleOccurrenceEnteredOrCanceled(KLib::Schedule* s, const QDate& _instanceDate);

    private:
        void loadData();
//        void rebuildIndexFrom(int _pos);
        bool handlesAccount(const KLib::Account* _a) const;
        bool balanceReaches(const KLib::Account* _account, const KLib::Amount& _amount, bool _under, QDate& _atDate) const;
        QList<Event> warningsFor(const KLib::Account* _a) const;

        QList<Event> m_events;
        int m_daysInFutureBalanceWarnings;

//        QHash<KLib::Account*, int> m_accountBalanceWarningIndex;
//        QHash<KLib::Schedule*, QHash<QDate, int> > m_billsDueIndex;
};

class AccountWarningsTab : public AccountEditTab
{
    Q_OBJECT

    public:
        explicit AccountWarningsTab(QWidget* _parent = nullptr);

        QString tabName() const { return tr("Balance Warnings"); }
        QString tabIcon() const { return "dialog-warning"; }

        bool enableIf(int _accountType) const;

        void fillData(const KLib::Account* _account);

        void save(KLib::Account* _account) const;

        QStringList validate() const;

    private:
        QCheckBox* m_chkLowWarning;
        QCheckBox* m_chkHighWarning;
        QCheckBox* m_chkOverLimit;

        KLib::AmountEdit* m_amtLow;
        KLib::AmountEdit* m_amtHigh;

};

class BillReminder : public IHomeWidget
{
    Q_OBJECT

    public:
        explicit BillReminder(QWidget* _parent = nullptr);

    public slots:
        void configure() override {}
        void refresh();

    protected:
        QString title() const override { return tr("Warnings and Bill Reminders"); }

    private:
        WarningBillReminderModel* m_model;
        QListView* m_view;
};

#endif // BILLREMINDER_H

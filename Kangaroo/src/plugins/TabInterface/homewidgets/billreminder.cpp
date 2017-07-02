#include "billreminder.h"
#include <KangarooLib/ui/core.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/schedule.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/ui/widgets/amountedit.h>
#include <QListView>
#include <QVBoxLayout>
#include <QPushButton>
#include <QGridLayout>
#include <QCheckBox>
#include <algorithm>

using namespace KLib;

const QString WarningBillReminderModel::LOW_BALANCE_PROPERTY = "LowBalanceWarning";
const QString WarningBillReminderModel::HIGH_BALANCE_PROPERTY = "HighBalanceWarning";
const QString WarningBillReminderModel::OVERLIMIT_PROPERTY = "OverLimitWarning";

BillReminder::BillReminder(QWidget* _parent) :
    IHomeWidget(new QWidget(), _parent),
    m_model(new WarningBillReminderModel(this))
{
    QPushButton* btnRefresh = new QPushButton(Core::icon("view-refresh"), "", this);
    QPushButton* btnConfigure = new QPushButton(Core::icon("configure"), "", this);

    QVBoxLayout* layoutBottom = new QVBoxLayout();
    layoutBottom->addStretch(2);
    layoutBottom->addWidget(btnRefresh);
    layoutBottom->addWidget(btnConfigure);

    QHBoxLayout* layout = new QHBoxLayout(centralWidget());
    layout->setContentsMargins(0,0,0,0);
    m_view = new QListView(this);
    layout->addWidget(m_view);
    layout->addLayout(layoutBottom);

    m_view->setModel(m_model);

    connect(btnRefresh, &QPushButton::clicked, this, &BillReminder::refresh);
    connect(btnConfigure, &QPushButton::clicked, this, &BillReminder::configure);
}

void BillReminder::refresh()
{
    m_model->refresh();
}

//*********************************** ACCOUNT EDIT TAB ***********************************

AccountWarningsTab::AccountWarningsTab(QWidget* _parent) :
    AccountEditTab(_parent)
{
    QGridLayout* mainLayout = new QGridLayout(this);
    m_chkLowWarning = new QCheckBox(tr("Warn me if the balance drops under: "), this);
    m_chkHighWarning = new QCheckBox(tr("Warn me if the balance goes over: "), this);
    m_chkOverLimit = new QCheckBox(tr("Warn me if the balance goes over the credit limit: "), this);

    m_chkLowWarning->setChecked(true);

    m_amtLow = new AmountEdit(this);
    m_amtHigh = new AmountEdit(this);

    m_amtHigh->setEnabled(false);

    mainLayout->addWidget(m_chkLowWarning,  0, 0);
    mainLayout->addWidget(m_amtLow,         0, 1);
    mainLayout->addWidget(m_chkHighWarning, 1, 0);
    mainLayout->addWidget(m_amtHigh,        1, 1);
    mainLayout->addWidget(m_chkOverLimit,   2, 0);
    mainLayout->addItem(new QSpacerItem(1, 10, QSizePolicy::Minimum, QSizePolicy::Expanding), 3, 0);

    connect(m_chkLowWarning,  &QCheckBox::toggled, m_amtLow,  &AmountEdit::setEnabled);
    connect(m_chkHighWarning, &QCheckBox::toggled, m_amtHigh, &AmountEdit::setEnabled);
}

bool AccountWarningsTab::enableIf(int _accountType) const
{
    bool isCredit = _accountType == AccountType::CREDITCARD;
    m_chkLowWarning->setEnabled(!isCredit);
    m_amtLow->setEnabled(!isCredit);
    m_chkOverLimit->setEnabled(isCredit);

    return (Account::generalType(_accountType) == AccountType::ASSET || isCredit)
            && _accountType != AccountType::INVESTMENT;
}

void AccountWarningsTab::fillData(const KLib::Account* _account)
{
    m_amtLow->setAmount(0);
    m_amtHigh->setAmount(1000);

    if (_account->properties()->contains(WarningBillReminderModel::LOW_BALANCE_PROPERTY))
    {
        if (_account->properties()->get(WarningBillReminderModel::LOW_BALANCE_PROPERTY).toString() == "OFF")
        {
            m_chkLowWarning->setChecked(false);
        }
        else
        {
            m_amtLow->setAmount(Amount::fromQVariant(_account->properties()->get(WarningBillReminderModel::LOW_BALANCE_PROPERTY)));
        }
    }

    if (_account->properties()->contains(WarningBillReminderModel::HIGH_BALANCE_PROPERTY))
    {
        m_chkHighWarning->setChecked(true);
        m_amtHigh->setAmount(Amount::fromQVariant(_account->properties()->get(WarningBillReminderModel::HIGH_BALANCE_PROPERTY)));
    }

    if (_account->type() == AccountType::CREDITCARD
        && _account->properties()->contains(WarningBillReminderModel::OVERLIMIT_PROPERTY)
        && _account->properties()->get(WarningBillReminderModel::OVERLIMIT_PROPERTY) == "ON")
    {
        m_chkOverLimit->setChecked(true);
    }
}

void AccountWarningsTab::save(KLib::Account* _account) const
{
    if (!m_chkLowWarning->isEnabled())
    {
        _account->properties()->remove(WarningBillReminderModel::LOW_BALANCE_PROPERTY);
    }
    else if (m_chkLowWarning->isChecked())
    {
        _account->properties()->set(WarningBillReminderModel::LOW_BALANCE_PROPERTY, m_amtLow->amount().toQVariant());
    }
    else
    {
        _account->properties()->set(WarningBillReminderModel::LOW_BALANCE_PROPERTY, "OFF");
    }

    if (m_chkHighWarning->isChecked())
    {
        _account->properties()->set(WarningBillReminderModel::HIGH_BALANCE_PROPERTY, m_amtHigh->amount().toQVariant());
    }
    else
    {
        _account->properties()->remove(WarningBillReminderModel::HIGH_BALANCE_PROPERTY);
    }

    if (m_chkOverLimit->isChecked())
    {
        _account->properties()->set(WarningBillReminderModel::OVERLIMIT_PROPERTY, "ON");
    }
    else
    {
        _account->properties()->remove(WarningBillReminderModel::OVERLIMIT_PROPERTY);
    }
}

QStringList AccountWarningsTab::validate() const
{
    if (m_chkLowWarning->isEnabled() && m_chkLowWarning->isChecked() && m_chkHighWarning->isChecked()
        && m_amtLow->amount() > m_amtHigh->amount())
    {
        return {tr("The low warning amount must be less than or equal to the high warning amount.") };
    }
    else
    {
        return {};
    }
}


//*********************************** MODEL ***********************************

WarningBillReminderModel::WarningBillReminderModel(QObject* _parent) :
    QAbstractListModel(_parent),
    m_daysInFutureBalanceWarnings(14)
{
    loadData();
}

bool WarningBillReminderModel::handlesAccount(const KLib::Account* _a) const
{
    return _a
            && (Account::generalType(_a->type()) == AccountType::ASSET
                || _a->type() == AccountType::CREDITCARD)
            && _a->type() != AccountType::INVESTMENT;
}

bool WarningBillReminderModel::balanceReaches(const KLib::Account* _account, const KLib::Amount& _amount, bool _under, QDate& _atDate) const
{
    _atDate = QDate::currentDate();

    for (int i = 0; i <= m_daysInFutureBalanceWarnings; ++i)
    {
        if ((_under && _account->balanceAt(_atDate) < _amount)
            || (!_under && _account->balanceAt(_atDate) > _amount))
        {
            return true;
        }
        else
        {
            _atDate = _atDate.addDays(1);
        }
    }

    return false;
}

QList<WarningBillReminderModel::Event> WarningBillReminderModel::warningsFor(const KLib::Account* _a) const
{
    QList<Event> list;
    QDate _date;

    auto addEvent = [&_date, _a, &list] (EventType t, const Amount& amount)
    {
        list << Event(t, _date, amount, _a);
    };

    if (handlesAccount(_a))
    {
        //Find how much the limit is
        Amount lowBalance, highBalance, creditLimit;
        bool lowBalanceOn = true;
        bool highBalanceOn = false;
        bool overLimitOn = false;

        if (_a->properties()->contains(LOW_BALANCE_PROPERTY))
        {
            if (_a->properties()->get(LOW_BALANCE_PROPERTY).toString() == "OFF")
            {
                lowBalanceOn = false;
            }
            else
            {
                lowBalance = Amount::fromQVariant(_a->properties()->get(LOW_BALANCE_PROPERTY));
            }
        }

        if (_a->properties()->contains(HIGH_BALANCE_PROPERTY))
        {
            highBalanceOn = true;
            highBalance = Amount::fromQVariant(_a->properties()->get(HIGH_BALANCE_PROPERTY));
        }

        if (_a->type() == AccountType::CREDITCARD)
        {
            lowBalanceOn = false;

            if (_a->properties()->contains(OVERLIMIT_PROPERTY)
                && _a->properties()->get(OVERLIMIT_PROPERTY) == "ON")
            {
                overLimitOn = true;
                creditLimit = Amount::fromQVariant(_a->properties()->get("creditlimit"));
            }
        }

        if (lowBalanceOn && balanceReaches(_a, lowBalance, true, _date))
        {
            addEvent(_date == QDate::currentDate() ? EventType::Warning_CurrentLowBalance
                                                   : EventType::Warning_FutureLowBalance,
                     lowBalance);
        }

        if (highBalanceOn && balanceReaches(_a, highBalance, false, _date))
        {
            addEvent(_date == QDate::currentDate() ? EventType::Warning_CurrentHighBalance
                                                   : EventType::Warning_FutureHighBalance,
                     highBalance);
        }

        if (overLimitOn && balanceReaches(_a, creditLimit, false, _date))
        {
            addEvent(_date == QDate::currentDate() ? EventType::Warning_CurrentOverCreditLimit
                                                   : EventType::Warning_FutureOverCreditLimit,
                     creditLimit);
        }
    }

    return list;
}

//void WarningBillReminderModel::onSplitAdded(const KLib::Transaction::Split& _split, KLib::Transaction* _tr)
//{

//}

//void WarningBillReminderModel::onSplitRemoved(const KLib::Transaction::Split& _split, KLib::Transaction* _tr)
//{

//}

//void WarningBillReminderModel::onSplitAmountChanged(const KLib::Transaction::Split& _split, KLib::Transaction* _tr)
//{

//}

//void WarningBillReminderModel::onTransactionDateChanged(KLib::Transaction* _tr, const QDate& _old)
//{

//}

//void WarningBillReminderModel::onScheduleAdded(KLib::Schedule* s)
//{
//    if (s->isActive() && s->remindBefore() >= 0)
//    {
//        //Check if the next date is in the near future
//        auto nextOcc = s->nextOccurrencesDates(Schedule::MAX_FUTURE_ENTERED);

//        for (const QDate& d : nextOcc)
//        {
//            if (d.addDays(-s->remindBefore()) <= QDate::currentDate())
//            {
//                //Add it at the right location
//                int i = 0;
//                for (; i < m_events.size(); ++i)
//                {
//                    if (m_events[i].date > d)
//                        break;
//                }

//                m_events.insert(i, Event(EventType::BillDueReminder, d, s));
//                rebuildIndexFrom(i);
//            }
//            else //If that one is too far enough, there is no hope for the next ones!
//            {
//                break;
//            }
//        }
//    }
//}

//void WarningBillReminderModel::onScheduleRemoved(KLib::Schedule* s)
//{
//    if (m_billsDueIndex.contains(s))
//    {
//        //Sort by last to first
//        QList<int> indices = m_billsDueIndex[s].values();
//        std::sort(indices.begin(), indices.end(), [](int a, int b) { return b < a; });

//        int leastPos = m_billsDueIndex[s].isEmpty() ? m_events.size()
//                                                    : indices.last();

//        for (int pos : indices)
//        {
//            m_events.removeAt(pos); //Since list is sorted in DESC order, the next indices are not affected by the removal
//        }

//        m_billsDueIndex.remove(s);

//        rebuildIndexFrom(leastPos);
//    }
//}

//void WarningBillReminderModel::onScheduleModified(KLib::Schedule* s)
//{
//    if (m_billsDueIndex.contains(s))
//    {
//        //Check if must be removed
//        if (!s->isActive() || s->remindBefore() <)
//        {
//            onScheduleRemoved(s);
//        }
//        else //Check to match the dates
//        {
//            auto nextOcc = s->nextOccurrencesDates(Schedule::MAX_FUTURE_ENTERED);

//            for (const QDate& d : nextOcc)
//            {

//            }
//        }

//    }
//    else //Check if must be added
//    {
//        onScheduleAdded(s);
//    }
//}

//void WarningBillReminderModel::onScheduleOccurrenceEnteredOrCanceled(KLib::Schedule* s, const QDate& _instanceDate)
//{
//}

void WarningBillReminderModel::loadData()
{
    //Load all bill reminders
    for (Schedule* s : ScheduleManager::instance()->schedules())
    {
        if (s->isActive() && s->remindBefore() >= 0)
        {
            //Check if the next date is in the near future
            auto nextOcc = s->nextOccurrencesDates(Schedule::MAX_FUTURE_ENTERED);

            for (const QDate& d : nextOcc)
            {
                if (d.addDays(-s->remindBefore()) <= QDate::currentDate())
                {
                    m_events.append(Event(EventType::BillDueReminder, d, s));
                }
                else //If that one is too far enough, there is no hope for the next ones!
                {
                    break;
                }
            }
        }
    }

    //Load all balance warnings
    for (Account* a : Account::getTopLevel()->accounts())
    {
        m_events.append(warningsFor(a));
    }

    //Sort everything
    std::sort(m_events.begin(), m_events.end(), [](const Event& a, const Event& b)
    {
        return a.date < b.date;
    });

    //Build the indexes
    //rebuildIndexFrom(0);
}

//void WarningBillReminderModel::rebuildIndexFrom(int _pos)
//{
//    for (int i = _pos; i < m_events.size(); ++i)
//    {
//        const Event& e = m_events[i];
//        if (e.relatedAccount)
//        {
//            m_accountBalanceWarningIndex[e.relatedAccount] = i;
//        }
//        else
//        {
//            m_billsDueIndex[e.relatedSchedule][e.date] = i;
//        }
//    }
//}

void WarningBillReminderModel::refresh()
{
    beginResetModel();

    m_events.clear();
    loadData();

    endResetModel();
}

QVariant WarningBillReminderModel::data(const QModelIndex& _index, int _role) const
{
    if (!_index.isValid())
        return QVariant();

    const Event& event = m_events[_index.row()];

    try
    {
        switch (_role)
        {
        case Qt::DecorationRole:
            switch (event.type)
            {
            case EventType::Warning_CurrentLowBalance:
            case EventType::Warning_FutureLowBalance:
            case EventType::Warning_CurrentHighBalance:
            case EventType::Warning_FutureHighBalance:
            case EventType::Warning_CurrentOverCreditLimit:
            case EventType::Warning_FutureOverCreditLimit:
                return Core::icon("dialog-warning");

            case EventType::BillDueReminder:
                return event.date < QDate::currentDate() ? Core::icon("bill-overdue-16")
                                                         : Core::icon("bill");
            }
            break;

        case Qt::DisplayRole:
        {
            QString message;
            switch (event.type)
            {
            case EventType::Warning_CurrentLowBalance:
                message = tr("Balance of \"%1\" is under %2.")
                                .arg(event.relatedAccount->name())
                                .arg(event.relatedAccount->formatAmount(event.amount));
                break;

            case EventType::Warning_FutureLowBalance:
                message = tr("Balance of \"%1\" will fall under %2.")
                                .arg(event.relatedAccount->name())
                                .arg(event.relatedAccount->formatAmount(event.amount));
                break;

            case EventType::Warning_CurrentHighBalance:
                message = tr("Balance of \"%1\" is over %2.")
                                .arg(event.relatedAccount->name())
                                .arg(event.relatedAccount->formatAmount(event.amount));
                break;

            case EventType::Warning_FutureHighBalance:
                message = tr("Balance of \"%1\" will be over %2.")
                                .arg(event.relatedAccount->name())
                                .arg(event.relatedAccount->formatAmount(event.amount));
                break;

            case EventType::Warning_CurrentOverCreditLimit:
                message = tr("Balance of your credit card \"%1\" is over the credit limit of %2.")
                                .arg(event.relatedAccount->name())
                                .arg(event.relatedAccount->formatAmount(event.amount));
                break;

            case EventType::Warning_FutureOverCreditLimit:
                message = tr("Balance of your credit card \"%1\" will be over the credit limit of %2.")
                                .arg(event.relatedAccount->name())
                                .arg(event.relatedAccount->formatAmount(event.amount));
                break;

            case EventType::BillDueReminder:
            {
                //Find amount of bill. Typically taken from/to asset account, so search for the first asset account. If
                //multiple, then do not mention the amount.
                Amount a = 0;
                QString cur;

                for (auto s : event.relatedSchedule->transaction()->splits())
                {
                    if (Account::generalType(Account::getTopLevel()->account(s.idAccount)->type()) == AccountType::ASSET)
                    {
                        a = s.amount;
                        cur = s.currency;
                    }
                }

                QString name = event.relatedSchedule->description().isEmpty() ? event.relatedSchedule->transaction()->memo()
                                                                              : event.relatedSchedule->description();

                if (a != 0)
                {
                    message = tr("Bill \"%1\" of %2 is %3.")
                                    .arg(name)
                                    .arg(cur.isEmpty() ? a.toString()
                                                       : CurrencyManager::instance()->get(cur)->formatAmount(a))
                                    .arg(event.date < QDate::currentDate() ? tr("overdue") : tr("due"));
                }
                else
                {
                    message = tr("Bill \"%1\" is %2.")
                                    .arg(name)
                                    .arg(event.date < QDate::currentDate() ? tr("overdue") : tr("due"));
                }
            }

            }

            return QString("%1: %2")
                    .arg(event.date.toString(Qt::TextDate))
                    .arg(message);
        }
        }
    }
    catch (...) {}

    return QVariant();
}


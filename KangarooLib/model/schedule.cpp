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

#include "schedule.h"
#include "modelexception.h"
#include "ledger.h"
#include "../controller/io.h"
#include "investmenttransaction.h"

#include <QXmlStreamReader>

namespace KLib
{

ScheduleManager* ScheduleManager::m_instance = new ScheduleManager();
int ScheduleManager::m_nextId = 0;

const int Recurrence::FIRST_WEEKDAY    = -1;
const int Recurrence::LAST_WEEKDAY     = -2;
const int Recurrence::LAST_DAY         = -3;

const int Schedule::MAX_FUTURE_ENTERED = 100;

namespace Frequency
{

    QString frequencyToString(int _freq)
    {
        switch (_freq)
        {
        case Once:
            return QObject::tr("Once");
        case Daily:
            return QObject::tr("Daily");
        case Weekly:
            return QObject::tr("Weekly");
        case Monthly:
            return QObject::tr("Monthly");
        case Yearly:
            return QObject::tr("Yearly");
        default:
            return "";
        }

    }
}

int lastWeekday(const QDate& _date)
{
    QDate temp(_date.year(), _date.month(), _date.daysInMonth());

    if (temp.dayOfWeek() == Qt::Saturday)
    {
        temp = temp.addDays(-1);
    }
    else if (temp.dayOfWeek() == Qt::Sunday)
    {
        temp = temp.addDays(-2);
    }

    return temp.day();
}

int firstWeekday(const QDate& _date)
{
    QDate temp(_date.year(), _date.month(), 1);

    if (temp.dayOfWeek() == Qt::Saturday)
    {
        temp = temp.addDays(2);
    }
    else if (temp.dayOfWeek() == Qt::Sunday)
    {
        temp = temp.addDays(1);
    }

    return temp.day();
}

bool Recurrence::daysOfMonthValid(const QSet<int>& _days)
{
    for (int d : _days)
    {
        if (d > 31 || d < -3 || d == 0)
        {
            return false;
        }
    }

    return true;
}

bool Recurrence::daysOfYearValid(const QSet<DayMonth>& _days)
{
    for (DayMonth d : _days)
    {
        if (d.second < -3 || d.second == 0)
        {
            return false;
        }
        else
        {
            switch (d.first)
            {
            case 1:
                return d.second < 32;
            case 2:
                return d.second < 30; //Allow 29th
            case 3:
                return d.second < 32;
            case 4:
                return d.second < 31;
            case 5:
                return d.second < 32;
            case 6:
                return d.second < 31;
            case 7:
                return d.second < 32;
            case 8:
                return d.second < 32;
            case 9:
                return d.second < 31;
            case 10:
                return d.second < 32;
            case 11:
                return d.second < 31;
            case 12:
                return d.second < 32;
            default:
                return false;
            }
        }
    }

    return true;
}

QList<int> Recurrence::orderedDaysOfMonth(QSet<int> _days, const QDate& _dateInMonth)
{
    QList<int> list;

    if (_days.contains(FIRST_WEEKDAY))
    {
        _days.remove(FIRST_WEEKDAY);
        _days.insert(firstWeekday(_dateInMonth));
    }

    if (_days.contains(LAST_WEEKDAY))
    {
        _days.remove(LAST_WEEKDAY);
        _days.insert(lastWeekday(_dateInMonth));
    }

    if (_days.contains(LAST_DAY))
    {
        _days.remove(LAST_DAY);
        _days.insert(_dateInMonth.daysInMonth());
    }

    list = _days.values();
    std::sort(list.begin(), list.end());
    return list;
}

QList<DayMonth> Recurrence::orderedDaysOfMonthYear(QSet<DayMonth> _days, QDate _dateInYear)
{
    QList<DayMonth> list;

    for (int m = 1; m < 13; ++m)
    {
        _dateInYear.setDate(_dateInYear.year(), m, 1);

        if (_days.contains(DayMonth(m, FIRST_WEEKDAY)))
        {
            _days.remove(DayMonth(m, FIRST_WEEKDAY));
            _days.insert(DayMonth(m, firstWeekday(_dateInYear)));
        }

        if (_days.contains(DayMonth(m, LAST_WEEKDAY)))
        {
            _days.remove(DayMonth(m, LAST_WEEKDAY));
            _days.insert(DayMonth(m, lastWeekday(_dateInYear)));
        }

        if (_days.contains(DayMonth(m, LAST_DAY)))
        {
            _days.remove(DayMonth(m, LAST_DAY));
            _days.insert(DayMonth(m, _dateInYear.daysInMonth()));
        }
    }

    list = _days.toList();
    std::sort(list.begin(), list.end());
    return list;
}

DateList Recurrence::nextOccurrencesDates(const std::list<QDate>& _skip,
                                          int _returnAtMost,
                                          const QDate& _atMostDate) const
{
    if (_returnAtMost < 1)
    {
        return {};
    }

    DateList nextDates;
    QDate next;

    //Check the total num of occurences
    if (stops && numRemaining != -1)
    {
        _returnAtMost = std::min(_returnAtMost, numRemaining);
    }

    //Cannot return more than the limit of future schedules
    _returnAtMost = std::min(_returnAtMost, Schedule::MAX_FUTURE_ENTERED);


    auto iter_skip = _skip.begin();

    bool isFirstIteration = true;

    //Loop
    while (nextDates.size() < _returnAtMost)
    {
        //==== Get the next date ====

        switch (frequency)
        {
        case Frequency::Once:
            next = next.isValid() ? QDate() : beginDate;
            break;

        case Frequency::Daily:
            if (next.isValid())
            {
                next = next.addDays(every);
            }
            else
            {
                next = beginDate;
            }
            break;
        case Frequency::Weekly:
        {
            if (!next.isValid())
            {
                //Start at the day BEFORE the first day, since otherwise it won't include this day if it matches.
                next = beginDate.addDays(-1);
            }

            int day = next.dayOfWeek()+1;

            //Safety check :-)
            if (weekdays.isEmpty())
            {
                return {};
            }

            //Find if the next day comes after the last entered day
            for (;day < 8 && !weekdays.contains((Qt::DayOfWeek) day); ++day) {}

            //If not, find the next one in the following week
            if (day > 7)
            {
                //Go to the next week
                for (day = 1; day < 8 && !weekdays.contains((Qt::DayOfWeek) day); ++day) {}

                //Add the number of weeks, + the difference in days.
                next = next.addDays(((isFirstIteration ? 1 : every) * 7) + (day - next.dayOfWeek()));
            }
            else
            {
                next = next.addDays(day - next.dayOfWeek());
            }

            break;
        }
        case Frequency::Monthly:
        {
            if (!next.isValid())
            {
                //Start at the day BEFORE the first day, since otherwise it won't include this day if it matches.
                next = beginDate.addDays(-1);
            }

            //Get the list of occurrences for this month
            QList<int> occurrencesThisMonth = Recurrence::orderedDaysOfMonth(daysOfMonth, next);

            //Safety check :-)
            if (occurrencesThisMonth.isEmpty())
            {
                return {};
            }

            //Find if the next day comes after the last entered day
            auto d = std::upper_bound(occurrencesThisMonth.begin(), occurrencesThisMonth.end(), next.day());

            if (d != occurrencesThisMonth.end())
            {
                next.setDate(next.year(), next.month(), *d);
            }
            else //Go to the next month, and select the 1st there.
            {
                next.setDate(next.year(), next.month(), 1);
                next = next.addMonths(isFirstIteration ? 1 : every);
                occurrencesThisMonth = Recurrence::orderedDaysOfMonth(daysOfMonth, next);
                next.setDate(next.year(), next.month(), occurrencesThisMonth.first());
            }

            break;
        }
        case Frequency::Yearly:
        {
            if (!next.isValid())
            {
                //Start at the day BEFORE the first day, since otherwise it won't include this day if it matches.
                next = beginDate.addDays(-1);
            }

            //Get the list of occurrences for this year
            QList<DayMonth> occurrencesThisYear = Recurrence::orderedDaysOfMonthYear(daysOfYear, next);

            //Safety check :-)
            if (occurrencesThisYear.isEmpty())
            {
                return {};
            }

            //Find if the next day comes after the last entered day
            auto d = std::upper_bound(occurrencesThisYear.begin(), occurrencesThisYear.end(), DayMonth(next.month(), next.day()));

            if (d != occurrencesThisYear.end())
            {
                next.setDate(next.year(), (*d).first, (*d).second);
            }
            else //Go to the next year, and select the 1st there.
            {
                next.setDate(next.year()+(isFirstIteration ? 1 : every), 1, 1);
                occurrencesThisYear = Recurrence::orderedDaysOfMonthYear(daysOfYear, next);
                next.setDate(next.year(), occurrencesThisYear[0].first, occurrencesThisYear[0].second);
            }

            break;
        }
        } //switch end

        //Check if the date found is valid, and if it is, then if matches the stop settings
        if (!next.isValid()
            || (_atMostDate.isValid() && next > _atMostDate)
            || (stops && lastDate.isValid() && next > lastDate))
        {
            break;
        }

        //==== Check if this date was canceled or already entered =====

        //Update the position of the indices
        while (iter_skip != _skip.end() && *iter_skip < next) ++iter_skip;

        //If we did not match anything...
        if (iter_skip == _skip.end() || *iter_skip != next)
        {
            nextDates << next;
        }

        isFirstIteration = false;
    }

    return nextDates;
}

bool Recurrence::operator==(const Recurrence& _other) const
{
    return beginDate == _other.beginDate
            && frequency == _other.frequency
            && every == _other.every
            && weekdays == _other.weekdays
            && daysOfYear == _other.daysOfYear
            && daysOfMonth == _other.daysOfMonth
            && stops == _other.stops
            && (!stops || (lastDate == _other.lastDate
                           && numRemaining == _other.numRemaining));
}

bool Recurrence::isValid() const
{
    if (beginDate.isValid()
        && (every > 0 || (every == 0 && frequency == Frequency::Once)))
    {
        if (stops && numRemaining < 0 && !lastDate.isValid())
        {
            //Either the last date or the number of remaining occurences must be valid
            return false;
        }

        switch (frequency)
        {
        case Frequency::Once:
            return true;

        case Frequency::Weekly:
            return !weekdays.isEmpty();

        case Frequency::Monthly:
            return !daysOfMonth.isEmpty() && daysOfMonthValid(daysOfMonth);

        case Frequency::Yearly:
            return !daysOfYear.isEmpty() && daysOfYearValid(daysOfYear);

        default:
            return false;
        }
    }
    else
    {
        return false;
    }
}

Schedule::Schedule() :
    m_active(false),
    m_remindBefore(-1),
    m_autoEnter(false),
    m_transaction(nullptr)
{
}

void Schedule::copyFrom(const Schedule& _schedule)
{
    m_description   = _schedule.m_description;
    m_active        = _schedule.m_active;
    m_autoEnter     = _schedule.m_autoEnter;
    m_recurrence    = _schedule.m_recurrence;
}

Schedule::~Schedule()
{
    delete m_transaction;
}

void Schedule::setDescription(const QString& _description)
{
    m_description = _description;

    if (!onHoldToModify())
        emit modified();
}

void Schedule::setActive(bool _active)
{
    m_active = _active;

    if (!onHoldToModify())
        emit modified();
}

void Schedule::setAutoEnter(bool _autoEnter)
{
    m_autoEnter = _autoEnter;

    if (!onHoldToModify())
        emit modified();
}

void Schedule::setRemindBefore(int _days)
{
    m_remindBefore = _days;

    if (!onHoldToModify())
        emit modified();
}

void Schedule::setRecurrence(const Recurrence& _rec)
{
    if (!_rec.isValid())
    {
        ModelException::throwException(tr("The recurrence settings are invalid."), this);
    }
    else if (_rec != m_recurrence)
    {
        m_recurrence = _rec;

        if (!onHoldToModify())
        {
            emit modified();
            emit recurrenceModified();
        }
        else
        {
            m_recurrenceModified = true;
        }
    }
}

void Schedule::setTransaction(Transaction* _transaction)
{
    if (!_transaction
        || !_transaction->splitCount())
    {
        ModelException::throwException(tr("The transaction is invalid."), this);
    }

    delete m_transaction;
    m_transaction = _transaction;
    connect(m_transaction, &Transaction::modified, this, &Schedule::modified);

    if (!onHoldToModify())
        emit modified();
}

QList<QDate> Schedule::nextOccurrencesDates(int _returnAtMost, const QDate& _atMostDate) const
{
    if (!m_active || (_returnAtMost < 1 && !_atMostDate.isValid()))
    {
        return {};
    }

    //Merge the entered and canceled lists (they are sorted and merge will keep that property nicely! Thanks STD!)
    std::list<QDate> skip = m_canceledOccurences;
    std::list<QDate> other = m_enteredOccurences; //Need a copy since merge empties out other
    skip.merge(other);

    return m_recurrence.nextOccurrencesDates(skip,
                                             _returnAtMost > 0 ? _returnAtMost
                                                               : MAX_FUTURE_ENTERED,
                                             _atMostDate);
}

void Schedule::enterNextOccurrence()
{
    QList<QDate> next = nextOccurrencesDates(2);

    if (next.isEmpty())
    {
        ModelException::throwException(tr("There are no future occurrences of this schedule."), this);
    }
    else
    {
        Transaction* temp = m_transaction->copyTo(Constants::NO_ID);
        temp->setDate(next[0]);
        priv_enterOccurrence(next[0], temp, next);
    }
}

void Schedule::enterOccurrenceOf(const QDate& _date)
{
    Transaction* temp = m_transaction->copyTo();
    temp->setDate(_date);
    enterOccurrenceOf(_date, temp);
}

void Schedule::enterOccurrenceOf(const QDate& _date, Transaction* _trans)
{
    //Check if _date is a valid occurrence date
    QList<QDate> next = nextOccurrencesDates(MAX_FUTURE_ENTERED);

    if (!next.contains(_date))
    {
        delete _trans;
        ModelException::throwException(tr("The occurrence date is not valid."), this);
    }
    else
    {
        priv_enterOccurrence(_date, _trans, next);
    }
}

void Schedule::cancelNextOccurrence()
{
    QList<QDate> next = nextOccurrencesDates(2);

    if (next.isEmpty())
    {
        ModelException::throwException(tr("There are no future occurrences of this schedule."), this);
    }
    else
    {
        priv_cancelOccurrence(next.first(), next);
    }
}

void Schedule::cancelOccurrenceOf(const QDate& _date)
{
    //Check if _date is a valid occurrence date
    QList<QDate> next = nextOccurrencesDates(MAX_FUTURE_ENTERED);

    if (!next.contains(_date))
    {
        ModelException::throwException(tr("The occurrence date is not valid."), this);
    }
    else
    {
        priv_cancelOccurrence(_date, next);
    }
}

void Schedule::doneHoldToModify()
{
    IStored::doneHoldToModify();

    if (m_recurrenceModified)
    {
        m_recurrenceModified = false;
        emit recurrenceModified();
    }
}

void Schedule::priv_enterOccurrence(const QDate& _date, Transaction* _trans, const QList<QDate>& _nextOccurrences)
{
    auto i = std::upper_bound(m_enteredOccurences.begin(),
                              m_enteredOccurences.end(),
                              _date);

    m_enteredOccurences.insert(i, _date);

    if (m_recurrence.stops && m_recurrence.numRemaining != -1)
    {
        --m_recurrence.numRemaining;
    }

    if (_date == _nextOccurrences.first()) //Increment the start date
    {
        if (_nextOccurrences.count() > 1)
        {
            m_recurrence.beginDate = _nextOccurrences[1];
        }
        else
        {
            m_recurrence.beginDate = QDate();
            m_active = false;
        }
        priv_eraseOccurrencesUntilBegin();
    }

    emit occurrenceEntered(_date);

    //Enter the transaction...
    LedgerManager::instance()->addTransaction(_trans);
}

void Schedule::priv_cancelOccurrence(const QDate& _date, const QList<QDate>& _nextOccurrences)
{
    auto i = std::upper_bound(m_canceledOccurences.begin(),
                              m_canceledOccurences.end(),
                              _date);

    m_canceledOccurences.insert(i, _date);

    if (m_recurrence.stops && m_recurrence.numRemaining != -1)
    {
        --m_recurrence.numRemaining;
    }

    if (_date == _nextOccurrences.first()) //Increment the start date
    {
        if (_nextOccurrences.count() > 1)
        {
            m_recurrence.beginDate = _nextOccurrences[1];
        }
        else
        {
            m_recurrence.beginDate = QDate();
            m_active = false;
        }
        priv_eraseOccurrencesUntilBegin();
    }

    emit occurrenceCanceled(_date);
}

void Schedule::priv_eraseOccurrencesUntilBegin()
{
    auto deleteUntil = [] (std::list<QDate>& list, const QDate& until)
    {
        while (!list.empty() && list.front() < until)
        {
            list.pop_front();
        }
    };

    deleteUntil(m_canceledOccurences, m_recurrence.beginDate);
    deleteUntil(m_enteredOccurences, m_recurrence.beginDate);
}

void Schedule::load(QXmlStreamReader& _reader)
{
    QXmlStreamAttributes attributes = _reader.attributes();

    m_id            = IO::getAttribute("id", attributes).toInt();
    m_description   = IO::getAttribute("name", attributes);
    m_active        = IO::getAttribute("active", attributes) == "true";
    m_autoEnter     = IO::getAttribute("autoenter", attributes) == "true";
    m_remindBefore  = IO::getOptAttribute("remindbefore", attributes, -1).toInt();

    if (m_remindBefore < 0)
    {
      m_remindBefore = -1;
    }
    else if (m_remindBefore > 30)
    {
      m_remindBefore = 30;
    }

    QStringList entered_str  = IO::getOptAttribute("entered", attributes, "").split(",");
    QStringList canceled_str = IO::getOptAttribute("canceled", attributes, "").split(",");

    //The lists may not be ordered. Add them first, then sort them.
    m_enteredOccurences.clear();
    m_canceledOccurences.clear();

    for (QString s: entered_str)
    {
        m_enteredOccurences.push_back(QDate::fromString(s, Qt::ISODate));
    }

    for (QString s: canceled_str)
    {
        m_canceledOccurences.push_back(QDate::fromString(s, Qt::ISODate));
    }

    m_enteredOccurences.sort(); //Specialized O(logn) sort for linked lists
    m_canceledOccurences.sort();

    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::SCHEDULE))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::TRANSACTION)
        {
            delete m_transaction;

            m_transaction = new Transaction();
            m_transaction->load(_reader);
            connect(m_transaction, &Transaction::modified, this, &Schedule::modified);
        }        
        else if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::INVEST_TRANSACTION)
        {
            delete m_transaction;

            m_transaction = new InvestmentTransaction();
            m_transaction->load(_reader);
            connect(m_transaction, &Transaction::modified, this, &Schedule::modified);
        }
        else if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::SCHEDULE_REC)
        {
            attributes = _reader.attributes();
            Recurrence r;

            r.beginDate     = QDate::fromString(IO::getAttribute("firstdate", attributes), Qt::ISODate);
            r.frequency     = IO::getAttribute("frequency", attributes).toInt();
            r.every         = IO::getAttribute("every", attributes).toInt();
            r.stops         = IO::getOptAttribute("stops", attributes, "false") == "true";

            if (r.stops)
            {
                r.lastDate      = QDate::fromString(IO::getOptAttribute("lastdate", attributes, QDate()), Qt::ISODate);
                r.numRemaining  = IO::getOptAttribute("numremaining", attributes, -1).toInt();
            }

            QString wd      = IO::getOptAttribute("weekdays", attributes, "");

            QStringList daysMonth = IO::getOptAttribute("monthdays", attributes, "").split(",");
            QStringList daysYear = IO::getOptAttribute("yeardays", attributes, "").split(",");

            for (int i = 0; i < wd.count(); ++i)
            {
                char c = wd[i].toLatin1();

                switch (c)
                {
                case 'M':
                    r.weekdays.insert(Qt::Monday);
                    break;
                case 'T':
                    r.weekdays.insert(Qt::Tuesday);
                    break;
                case 'W':
                    r.weekdays.insert(Qt::Wednesday);
                    break;
                case 'R':
                    r.weekdays.insert(Qt::Thursday);
                    break;
                case 'F':
                    r.weekdays.insert(Qt::Friday);
                    break;
                case 'S':
                    r.weekdays.insert(Qt::Saturday);
                    break;
                case 'N':
                    r.weekdays.insert(Qt::Sunday);
                    break;
                }
            }

            for (QString d : daysMonth)
            {
                r.daysOfMonth.insert(d.toInt());
            }

            for (QString d : daysYear)
            {
                QStringList dm = d.split(":");

                if (dm.count() == 2)
                {
                    r.daysOfYear.insert(DayMonth(dm[0].toInt(), dm[1].toInt()));
                }
            }

            m_recurrence = r;
        }

        _reader.readNext();
    }
}

void Schedule::save(QXmlStreamWriter& _writer) const
{
    _writer.writeStartElement(StdTags::SCHEDULE);
    _writer.writeAttribute("id", QString::number(m_id));
    _writer.writeAttribute("name", m_description);
    _writer.writeAttribute("active", m_active ? "true" : "false");
    _writer.writeAttribute("autoenter", m_autoEnter ? "true" : "false");
    _writer.writeAttribute("remindbefore", QString::number(m_remindBefore));

    QStringList entered, canceled;

    for (QDate d : m_enteredOccurences)
    {
        entered << d.toString(Qt::ISODate);
    }

    for (QDate d : m_canceledOccurences)
    {
        canceled << d.toString(Qt::ISODate);
    }

    if (entered.count())
    {
        _writer.writeAttribute("entered", entered.join(","));
    }

    if (canceled.count())
    {
        _writer.writeAttribute("canceled", canceled.join(","));
    }

    if (!m_transaction)
    {
        ModelException::throwException(tr("The transaction of schedule %1 does not exists!").arg(m_description), this);
    }

    m_transaction->save(_writer);



    _writer.writeEmptyElement(StdTags::SCHEDULE_REC);
    _writer.writeAttribute("firstdate", m_recurrence.beginDate.toString(Qt::ISODate));
    _writer.writeAttribute("frequency", QString::number(m_recurrence.frequency));
    _writer.writeAttribute("every", QString::number(m_recurrence.every));
    _writer.writeAttribute("stops", m_recurrence.stops ? "true" : "false");

    if (m_recurrence.stops)
    {
        if (m_recurrence.lastDate.isValid())
        {
            _writer.writeAttribute("lastdate", m_recurrence.lastDate.toString(Qt::ISODate));
        }
        else /*!m_recurrence.lastDate.isValid()*/
        {
            _writer.writeAttribute("numremaining", QString::number(m_recurrence.numRemaining));
        }

    }


    switch (m_recurrence.frequency)
    {
    case Frequency::Weekly:
    {
        QString weekDays;

        for (Qt::DayOfWeek d : m_recurrence.weekdays)
        {
            switch (d)
            {
            case Qt::Monday:
                weekDays += "M";
                break;
            case Qt::Tuesday:
                weekDays += "T";
                break;
            case Qt::Wednesday:
                weekDays += "W";
                break;
            case Qt::Thursday:
                weekDays += "R";
                break;
            case Qt::Friday:
                weekDays += "F";
                break;
            case Qt::Saturday:
                weekDays += "S";
                break;
            case Qt::Sunday:
                weekDays += "N";
                break;
            }
        }

        _writer.writeAttribute("weekdays", weekDays);
        break;
    }
    case Frequency::Monthly:
    {
        QStringList daysMonth;

        for (int d : m_recurrence.daysOfMonth)
        {
            daysMonth << QString::number(d);
        }

        _writer.writeAttribute("monthdays", daysMonth.join(","));
        break;
    }
    case Frequency::Yearly:
    {
        QStringList daysYear;

        for (DayMonth d : m_recurrence.daysOfYear)
        {
            daysYear << QString::number(d.first) + ":" + QString::number(d.second);
        }

        _writer.writeAttribute("yeardays", daysYear.join(","));
        break;
    }
    } //end switch

    _writer.writeEndElement();
}

/////////////////////////// ScheduleManager /////////////////////////////////////

Schedule* ScheduleManager::add(const QString& _name,
                               bool _autoEnter,
                               const Recurrence& _rec,
                               Transaction* _transaction)
{
    if (!_transaction
        || !_transaction->splitCount())
    {
        ModelException::throwException(tr("The transaction is invalid."), this);
    }

    if (!_rec.isValid())
    {
        ModelException::throwException(tr("The recurrence settings are invalid."), this);
    }



    Schedule* o = new Schedule();
    o->m_id = m_nextId++;
    o->m_description = _name;
    o->m_autoEnter = _autoEnter;
    o->m_recurrence = _rec;
    o->m_transaction = _transaction;
    o->m_active       = true;

    if (_rec.stops && _rec.lastDate.isValid())
    {
        o->m_recurrence.numRemaining = -1;
    }

    m_schedules.append(o);
    emit modified();
    emit scheduleAdded(o);

    connectSignals(o);
    return o;

}

void ScheduleManager::connectSignals(Schedule* _schedule)
{
    connect(_schedule, &Schedule::modified,           this, &ScheduleManager::onModified);
    connect(_schedule, &Schedule::occurrenceEntered,    this, &ScheduleManager::onEntered);
    connect(_schedule, &Schedule::occurrenceCanceled,   this, &ScheduleManager::onCanceled);
    connect(_schedule, &Schedule::recurrenceModified, this, &ScheduleManager::onRecurrenceModified);
}

void ScheduleManager::onModified()
{
    Schedule* sch = qobject_cast<Schedule*>(sender());

    if (sch)
    {
        emit scheduleModified(sch);
        emit modified();
    }
}

void ScheduleManager::onRecurrenceModified()
{
    Schedule* sch = qobject_cast<Schedule*>(sender());

    if (sch)
    {
        emit scheduleRecurrenceModified(sch);
    }
}

void ScheduleManager::onEntered(const QDate& _occurrenceDate)
{
    Schedule* sch = qobject_cast<Schedule*>(sender());

    if (sch)
    {
        emit scheduleOccurrenceEntered(sch, _occurrenceDate);
    }
}

void ScheduleManager::onCanceled(const QDate& _occurrenceDate)
{
    Schedule* sch = qobject_cast<Schedule*>(sender());

    if (sch)
    {
        emit scheduleOccurrenceCanceled(sch, _occurrenceDate);
    }
}

Schedule* ScheduleManager::get(int _id) const
{
    foreach (Schedule* i, m_schedules)
    {
        if (i->id() == _id)
        {
            return i;
        }
    }

    ModelException::throwException(tr("No such schedule."), this);
    return nullptr;
}

void ScheduleManager::remove(int _id)
{
    for (int i = 0; i < m_schedules.size(); ++i)
    {
        if (m_schedules[i]->id() == _id)
        {
            emit scheduleRemoved(m_schedules[i]);
            m_schedules[i]->deleteLater();
            m_schedules.removeAt(i);

            emit modified();
            break;
        }
    }
}

void ScheduleManager::removeSchedulesForAccount(int _idAccount)
{
    auto i = m_schedules.begin();

    while (i != m_schedules.end())
    {
        //Check if related to the account
        if ((*i)->transaction()->relatedTo(_idAccount))
        {
            emit scheduleRemoved(*i);
            (*i)->deleteLater();
            i = m_schedules.erase(i);
            emit modified();
        }
        else
        {
            ++i;
        }
    }
}

QList<Schedule*> ScheduleManager::schedulesFor(int _idAccount) const
{
    QList<Schedule*> list;

    for (Schedule* s : m_schedules)
    {
        if (s->transaction()->relatedTo(_idAccount))
        {
            list << s;
        }
    }

    return list;
}

QList<KLib::Schedule*> ScheduleManager::dueSchedules(QList<QDate>& _dates) const
{
    QList<Schedule*> list;
    _dates.clear();

    for (Schedule* s : m_schedules)
    {
        if (s->isActive())
        {
            QList<QDate> d = s->nextOccurrencesDates(1);

            if (d.count() && d[0] <= QDate::currentDate())
            {
                list << s;
                _dates << d;
            }
        }
    }

    return list;
}

void ScheduleManager::load(QXmlStreamReader& _reader)
{
    unload();

    // While not at end
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::SCHEDULE_MGR))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::SCHEDULE)
        {
            Schedule* o = new Schedule();
            o->load(_reader);
            m_nextId = std::max(m_nextId, o->m_id + 1);
            m_schedules.append(o);
            connectSignals(o);
        }

        _reader.readNext();
    }
}

void ScheduleManager::save(QXmlStreamWriter &_writer) const
{
    for (Schedule* o : m_schedules)
    {
        o->save(_writer);
    }
}

void ScheduleManager::unload()
{
    for (Schedule* i : m_schedules)
    {
        i->deleteLater();
    }

    m_schedules.clear();
    m_nextId = 0;
}

void ScheduleManager::afterLoad()
{
    //Check if some schedules do not have any more occurrences, in which case they can be deleted.
    auto i = m_schedules.begin();

    while (i != m_schedules.end())
    {
        //Check if at least 1 occurrence remaining...
        if ((*i)->nextOccurrencesDates(1).isEmpty())
        {
            emit scheduleRemoved(*i);
            (*i)->deleteLater();
            i = m_schedules.erase(i);
            emit modified();
        }
        else
        {
            ++i;
        }
    }
}

}

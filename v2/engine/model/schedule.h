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

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <QDate>
#include <list>
#include <QList>
#include "stored.h"
#include "transaction.h"

namespace KLib
{

    namespace Frequency
    {
        enum Type
        {
            Once        = 0,
            Daily       = 10,
            Weekly      = 20,
            Monthly     = 30,
            Yearly      = 40
        };

        const int NumFrequencies = 5;

        QString frequencyToString(int _freq);
    }

    typedef QPair<int, int> DayMonth; //First is month, second is day
    typedef QList<QDate> DateList;

    struct Recurrence
    {
        Recurrence() : frequency(Frequency::Once),
                       every(0),
                       stops(false),
                       numRemaining(-1) {}

        Recurrence(const QDate& _firstDate, int _frequency, int _every)
            : beginDate(_firstDate),
              frequency(_frequency),
              every(_every) ,
              stops(false),
              numRemaining(-1) {}

        bool operator==(const Recurrence& _other) const;
        bool operator!=(const Recurrence& _other) const { return !operator==(_other); }

        bool isValid() const;

        static bool daysOfMonthValid(const QSet<int>& _days);
        static bool daysOfYearValid(const QSet<DayMonth>& _days);

        static QList<int> orderedDaysOfMonth(QSet<int> _days, const QDate& _dateInMonth);
        static QList<DayMonth> orderedDaysOfMonthYear(QSet<DayMonth> _days, QDate _dateInYear);

        DateList nextOccurrencesDates(const std::list<QDate>& _skip, int _returnAtMost, const QDate& _atMostDate = QDate()) const;

        QDate beginDate; ///< Date at which starts the schedule. It is not necessarily the date of the first occurrence.

        int frequency;

        /**
         * @brief every
         *
         * Must be > 0. If 1, then will occur every frequency.
         * EX: frequency = Month, every=2 : will occur every 2 months, Jan,Mar,May,...
         */
        int every;

        /**
         * @brief weekdays
         *
         * If Weekly, contains a list of days the recurrence occurs during the week.
         */
        QSet<Qt::DayOfWeek> weekdays;

        /**
         * @brief For Yearly: all the days for the year.
         */
        QSet<DayMonth>      daysOfYear;

        /**
         * @brief For Monthly: all the days for the month
         */
        QSet<int>           daysOfMonth;

        /**
         * @brief If the recurrence stops at some point (num of occurrences or specific date)
         *
         * Exactly one of lastDate or numRemaining must be valid if stops is true.
         */
        bool                stops;

        /**
         * @brief The last possible date of an occurrence, if stops is true.
         *
         * It is not necessarily the last date of the schedule, but no occurrences can happen after it.
         */
        QDate               lastDate;

        /**
         * @brief Number of remaining occurrences.
         *
         * Invalid if -1. Can be 0, in that case no more occurrences of the schedule can occur.
         *
         * This value will be decremented each time an occurrence is canceled or entered.
         */
        int                 numRemaining;

        static const int FIRST_WEEKDAY;
        static const int LAST_WEEKDAY;
        static const int LAST_DAY;
    };

    class Schedule : public IStored
    {
        Q_OBJECT

        public:
            Schedule();
            ~Schedule();

            /**
             * @brief Copies basic infos from _schedule: name, reccurence, etc.
             *
             * Does NOT copy the occurrences nor the transaction, etc.
             */
            void copyFrom(const Schedule& _schedule);

            QString             description() const  { return m_description; }
            bool                isActive() const     { return m_active; }
            bool                autoEnter() const    { return m_autoEnter; }
            const Recurrence&   recurrence() const   { return m_recurrence; }
            const Transaction*  transaction() const  { return m_transaction; }
            Transaction*        transaction()        { return m_transaction; }

            /**
             * @brief Number of days in advance to remaind of the schedule.
             *
             * If -1 (or any negative value), no remainder will be displayed.
             */
            int remindBefore() const { return m_remindBefore; }

            void setDescription(const QString& _description);
            void setActive(bool _active);
            void setAutoEnter(bool _autoEnter);
            void setRecurrence(const Recurrence& _rec);
            void setTransaction(Transaction* _transaction);

            void setRemindBefore(int _days);

            /**
             * @brief Gets the future dates of the schedule
             * @param _returnAtMost The maximum number of occurrences to show. Must be <= Schedule::MAX_FUTURE_ENTERED
             * @param _atMostDate The upper bound on the date to search for occurrences.
             * @return The dates of the next occurrence, or an empty list if it is not scheduled anymore.
             *
             * One can call the function with a valid date and -1 for _returnAtMost, in which case only
             * the date will be used as a condition, with both valid set, or with _returnAtMost > 0 and _atMostDate
             * invalid, which will only consider the number of occurrences condition. It will return an empty list
             * if both conditions are invalid (negative _returnAtMost and invalid _atMostDate.
             */
            QList<QDate> nextOccurrencesDates(int _returnAtMost, const QDate& _atMostDate = QDate()) const;


            /**
             * @brief Enters the next occurrence "as is".
             * @return If was succesfull.
             *
             * It can only be unsuccesfull if there was no future occurrence (nextOccurrencesDates() returns an empty list).
             */
            void enterNextOccurrence();

            /**
             * @brief Enters the occurrence that is on _date "as is"
             * @param _date The occurrence date. Must be a scheduled date.
             * @return If was succesfull.
             *
             * Will not be succesfull if _date is too far in the future
             * (see MAX_FUTURE_ENTERED) or if the date is not a valid scheduled date.
             */
            void enterOccurrenceOf(const QDate& _date);

            /**
             * @brief Enters the occurrence that is on _date using the transaction _trans
             * @param _date The occurrence date. Must be a scheduled date.
             * @param _trans The transaction to use for this occurrence
             * @return If was succesfull.
             *
             * Will not be succesfull if _date is too far in the future
             * (see MAX_FUTURE_ENTERED) or if the date is not a valid scheduled date.
             *
             * Do not delete the transaction _trans, even if the function was not successfull.
             * It will be added to the ledger and handled by it, or destroyed accordingly.
             *
             * If _trans does not have the same date as _date, it changes this occurrence to have this date.
             * Thus, if other occurrences were scheduled before _trans->date() but after _date, they will still be
             * scheduled.
             */
            void enterOccurrenceOf(const QDate& _date, Transaction* _trans);

            void cancelNextOccurrence();
            void cancelOccurrenceOf(const QDate& _date);

            void doneHoldToModify() override;

            /**
             * @brief The limit on entering future occurrences.
             *
             * For example, if one tries to enter the Nth future occurrence, but N is greater than
             * MAX_FUTURE_ENTERED, entering the occurrence will fail. If N <= MAX_FUTURE_ENTERED, it will work.
             */
            static const int MAX_FUTURE_ENTERED;

        signals:
            void occurrenceEntered(const QDate& _occurrenceDate);
            void occurrenceCanceled(const QDate& _occurrenceDate);
            void recurrenceModified();

        private:
            QString     m_description;
            bool        m_active;
            int         m_remindBefore;
            bool        m_autoEnter;
            Recurrence  m_recurrence;


            Transaction* m_transaction;

            std::list<QDate> m_canceledOccurences;   ///< Canceled occurences
            std::list<QDate> m_enteredOccurences;    ///< Entered occurences

            bool m_recurrenceModified; //For on-hold modifications

            void priv_enterOccurrence(const QDate& _date, Transaction* _trans, const QList<QDate>& _nextOccurrences);
            void priv_cancelOccurrence(const QDate& _date, const QList<QDate>& _nextOccurrences);

            /**
             * @brief Clears everything in canceled/entered occurences until the begin date (not inclusive of it)
             */
            void priv_eraseOccurrencesUntilBegin();

        protected:
            void load(QXmlStreamReader& _reader) override;
            void save(QXmlStreamWriter& _writer) const override;

            friend class ScheduleManager;
    };

    class ScheduleManager : public IStored
    {
        Q_OBJECT

        public:
            Schedule*   add(const QString& _name,
                            bool _autoEnter,
                            const Recurrence& _rec,
                            Transaction* _transaction);
            Schedule*   get(int _id) const;
            Schedule*   at(int _i) const { return m_schedules[_i]; }
            void        remove(int _id);

            void        removeSchedulesForAccount(int _idAccount);

            int         count() const { return m_schedules.size(); }

            const QList<Schedule*>& schedules() const { return m_schedules; }

            /**
             * @brief Schedules related to an account
             * @param _idAccount The related account
             * @return All the active schedules matching the criteria.
             *
             * Returns a list of <b>active</b> schedules that include the account identified by _idAccount in a split
             * of the scheduled transaction.
             */
            QList<Schedule*> schedulesFor(int _idAccount) const;

            /**
             * @brief dueSchedules
             * @param[out] _dates Will contain the dates the schedules are due, same order as returned list.
             * @return A list of the schedules due to be added.
             */
            QList<Schedule*> dueSchedules(QList<QDate>& _dates) const;

            static ScheduleManager* instance() { return m_instance; }

        signals:
            void scheduleAdded(Schedule* s);
            void scheduleRemoved(Schedule* s);
            void scheduleModified(Schedule* s);

            void scheduleOccurrenceEntered(Schedule* s, const QDate& _occurrenceDate);
            void scheduleOccurrenceCanceled(Schedule* s, const QDate& _occurrenceDate);
            void scheduleRecurrenceModified(Schedule* s);

        private slots:
            void onModified();
            void onRecurrenceModified();

            void onEntered(const QDate& _occurrenceDate);
            void onCanceled(const QDate& _occurrenceDate);

        private:
            void connectSignals(Schedule* _schedule);

            QList<Schedule*> m_schedules;

            static ScheduleManager* m_instance;
            static int m_nextId;

        protected:
            void load(QXmlStreamReader& _reader) override;
            void save(QXmlStreamWriter& _writer) const override;
            void unload() override;

            void afterLoad();
    };

} //Namespace

#endif // SCHEDULE_H

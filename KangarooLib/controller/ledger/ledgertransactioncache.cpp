#include "ledgertransactioncache.h"
#include "ledgercontroller.h"

#include "../../model/account.h"
#include "../../model/ledger.h"
#include "../../ui/settingsmanager.h"

namespace KLib
{
    LedgerTransactionCache::LedgerTransactionCache(LedgerController* _controller) :
        QObject(_controller),
        m_controller(_controller),
        m_filterIsApplied(false),
        m_filterFunction([](const_reference) { return false; }),
        m_displayPolicy(ScheduleDisplayPolicy::FixedNumber),
        m_displayPolicyCount(15)
    {
        //Connect signals and slots
        connect(LedgerManager::instance(), &LedgerManager::splitAdded,
                this,                      &LedgerTransactionCache::onSplitAdded);
        connect(LedgerManager::instance(), &LedgerManager::splitRemoved,
                this,                      &LedgerTransactionCache::onSplitRemoved);
        connect(LedgerManager::instance(), &LedgerManager::splitAmountChanged,
                this,                      &LedgerTransactionCache::onSplitAmountChanged);
        connect(LedgerManager::instance(), &LedgerManager::transactionDateChanged,
                this,                      &LedgerTransactionCache::onTransactionDateChanged);

        connect(ScheduleManager::instance(), &ScheduleManager::scheduleAdded,
                this,                        &LedgerTransactionCache::onScheduleAdded);
        connect(ScheduleManager::instance(), &ScheduleManager::scheduleRemoved,
                this,                        &LedgerTransactionCache::onScheduleRemoved);
        connect(ScheduleManager::instance(), &ScheduleManager::scheduleModified,
                this,                        &LedgerTransactionCache::onScheduleModified);
        connect(ScheduleManager::instance(), &ScheduleManager::scheduleOccurrenceCanceled,
                this,                        &LedgerTransactionCache::onScheduleOccurrenceEnteredOrCanceled);
        connect(ScheduleManager::instance(), &ScheduleManager::scheduleOccurrenceEntered,
                this,                        &LedgerTransactionCache::onScheduleOccurrenceEnteredOrCanceled);
    }

    void LedgerTransactionCache::setFilter(fn_filter _keep)
    {
        m_filteredSortedCache.clear();

        m_controller->beginResetModel();

        int i = 0;
        for (const_reference c : m_cache)
        {
            if (_keep(c))
            {
                m_cacheToFilteredMap[i] = m_filteredSortedCache.size();
                m_filteredSortedCache.append(c);
            }

            ++i;
        }

        m_filterFunction = _keep;
        m_filterIsApplied = true;

        m_controller->endResetModel();
    }

    void LedgerTransactionCache::removeFilter()
    {
        m_controller->beginResetModel();

        m_filterIsApplied = false;
        m_filteredSortedCache.clear();
        m_cacheToFilteredMap.clear();

        m_controller->endResetModel();
    }

    void LedgerTransactionCache::setSorting(fn_compare _compare)
    {

    }

    bool LedgerTransactionCache::compareItems(const_reference _first, const_reference _second,
                                              const CacheItem* _item, const QDate& _date) const
    {
        const QDate& firstDate  = &_first == _item ? _date : _first.date();
        const QDate& secondDate = &_second == _item ? _date : _second.date();

        if (firstDate == secondDate)
        {
            if (_first.schedule && !_second.schedule)
            {
                return false;
            }
            else if (!_first.schedule && _second.schedule)
            {
                return true;
            }
            else //If both are schedules/not schedules. Order from largest to smallest amounts
            {
                return Transaction::totalForAccount(m_controller->account()->id(), _first.transaction()->splits())
                        > Transaction::totalForAccount(m_controller->account()->id(), _second.transaction()->splits());
            }
        }
        else
        {
            return firstDate < secondDate;
        }
    }

    void LedgerTransactionCache::reloadData()
    {
        //Tell the controller we're resetting!
        m_controller->beginResetModel();

        m_displayPolicy = ScheduleDisplayPolicy(SettingsManager::instance()->value("Ledger/ScheduleDisplayPolicy").toInt());

        if (m_displayPolicy == ScheduleDisplayPolicy::DaysInFuture)
        {
            m_displayPolicyCount = SettingsManager::instance()->value("Ledger/ScheduleDisplayDays").toInt();
        }
        else
        {
            m_displayPolicyCount = SettingsManager::instance()->value("Ledger/ScheduleDisplayInstances").toInt();
        }

        //Clear everything
        m_cache.clear();
        m_filteredSortedCache.clear();
        m_cacheToFilteredMap.clear();
        m_filterIsApplied = false;

        //Load...
        int i = 0;

        //Get the schedules related to this account
        QList<QPair<QDate, Schedule*>> futureInstances;

        if (m_displayPolicy != ScheduleDisplayPolicy::NoSchedules)
        {
            QList<Schedule*> schedules = ScheduleManager::instance()->schedulesFor(m_controller->account()->id());

            for (Schedule* s : schedules)
            {
//                if (!m_controller->canEditTransaction(s->transaction()))
//                    continue;

                QList<QDate> instances = nextOccurrences(s);

                for (const QDate& d : instances)
                {
                    futureInstances << QPair<QDate, Schedule*>(d, s);
                }
            }

            //Sort the schedules by date
            std::sort(futureInstances.begin(), futureInstances.end());
        }

        //For each day, we want to show, in this order: DEBITS, CREDITS, SCHEDULES
        auto is = futureInstances.begin();
        auto transactions = m_controller->ledger()->transactions();

        for (auto it = transactions->begin(); it != transactions->end(); ++it, ++i)
        {
            //Add all the schedules with a date < this schedule.
            while (is != futureInstances.end() && is->first < (*it)->date())
            {
                m_cache.append(CacheItem(is->second,
                                         is->first,
                                         m_controller->subRowCount(is->second->transaction())));

                ++is;
            }

            //Now add the next transaction.
            bool before = Transaction::totalForAccount(m_controller->account()->id(), (*it)->splits()) > 0;

            if (before && m_cache.count() > 0)
            {
                auto j = m_cache.end();
                for (; j != m_cache.begin(); --j)
                {
                    if ((*(j-1)).date() < (*it)->date())
                    {
                        break;
                    }
                }

                m_cache.insert(j, CacheItem(*it, m_controller->subRowCount(*it)));
            }
            else
            {
                m_cache.append(CacheItem(*it, m_controller->subRowCount(*it)));
            }
        }

        //Add all the remaining schedules
        while (is != futureInstances.end())
        {
            m_cache.append(CacheItem(is->second,
                                     is->first,
                                     m_controller->subRowCount(is->second->transaction())));

            ++is;
        }

        //Build the index and the running balances
        reloadBalanceAndIndexFrom(0);

        //We're done!
        m_controller->endResetModel();
    }

    //not fixed yet
    void LedgerTransactionCache::reloadBalanceAndIndexFrom(int _from)
    {
        for (int i = std::max(0, _from); i < m_cache.size(); ++i)
        {
            //Update the balance
            m_cache[i].runningBalance = i > 0 ? m_cache[i-1].runningBalance : Balances(); //Balance before the current row

            if (i != m_controller->newTransactionRow()
                && (!m_cache[i].schedule || m_cache[i].date() > QDate::currentDate()))
            {
                if (Account::negativeDebits(m_controller->account()->type()))
                {
                    m_cache[i].runningBalance -= m_controller->cacheBalance(i);
                }
                else
                {
                    m_cache[i].runningBalance += m_controller->cacheBalance(i);
                }
            }

            //Update the index
            if (!m_cache[i].schedule)
            {
                m_transactionIndex[m_cache[i].transaction()->id()] = i;
            }
            else
            {
                m_scheduleIndex[m_cache[i].schedule->id()][m_cache[i].dueDate] = i;
            }
        }
    }

    /////////////////////////////////////// ROW HELPERS //////////////////////////////////////

    int LedgerTransactionCache::cachedSubRowCount(int _cacheRow) const
    {
        return currentList()[_cacheRow].cachedSubRowCount;
    }

    int LedgerTransactionCache::addItem(const CacheItem& _item, bool _reloadIndex)
    {
        //First, insert it in the regular cache
        using namespace std::placeholders;

        auto i = std::upper_bound(m_cache.begin(), m_cache.end(), _item,
                                  std::bind(&LedgerTransactionCache::compareItems, this, _1,_2, nullptr, QDate()));

        int row = i-m_cache.begin();

        if (!m_filterIsApplied)
        {
            emit m_controller->beginInsertRows(QModelIndex(), row, row);
            m_cache.insert(i, _item);

            if (_reloadIndex)
            {
                reloadBalanceAndIndexFrom(row);
            }

            emit m_controller->endInsertRows();
        }
        else
        {
            m_cache.insert(i, _item);

            if (_reloadIndex)
            {
                reloadBalanceAndIndexFrom(row);
            }

            if (m_filterFunction(_item))
            {
                i = std::upper_bound(m_filteredSortedCache.begin(), m_filteredSortedCache.end(), _item,
                                     std::bind(&LedgerTransactionCache::compareItems, this, _1,_2, nullptr, QDate()));

                int row = i-m_filteredSortedCache.begin();

                emit m_controller->beginInsertRows(QModelIndex(), row, row);
                m_filteredSortedCache.insert(i, _item);
                emit m_controller->endInsertRows();
            }
        }

        return row;
//        using namespace std::placeholders;

//        auto i = std::upper_bound(m_cache.begin(), m_cache.end(), _item,
//                                  std::bind(&LedgerTransactionCache::compareItems, this, _1,_2, nullptr, QDate()));
//        int row = i-m_cache.begin();

//        emit m_controller->beginInsertRows(QModelIndex(), row, row);
//        m_cache.insert(i, _item);

//        if (_reloadIndex)
//        {
//            reloadBalanceAndIndexFrom(row);
//        }

//        emit m_controller->endInsertRows();

//        return row;
    }

    void LedgerTransactionCache::removeRow(int _row, bool _reloadIndex)
    {
        int cacheRow = _row;

        //See if the row is also in the filter, in which case we remove it there instead
        if (m_filterIsApplied && m_cacheToFilteredMap.contains(_row))
        {
            _row = m_cacheToFilteredMap[_row];
        }
        else if (m_filterIsApplied) //No need to emit a signal
        {
            _row = -1;
        }

        if (_row != -1)
        {
            emit m_controller->beginRemoveRows(QModelIndex(), _row, _row);
        }

        CacheItem item = m_cache.takeAt(cacheRow);

        if (item.schedule && m_scheduleIndex.contains(item.schedule->id()))
        {
            m_scheduleIndex[item.schedule->id()].remove(item.dueDate);

            if (m_scheduleIndex[item.schedule->id()].isEmpty())
            {
                m_scheduleIndex.remove(item.schedule->id());
            }
        }
        else if (!item.schedule)
        {
            m_transactionIndex.remove(item.transaction()->id());
        }

        if (_reloadIndex)
        {
            reloadBalanceAndIndexFrom(cacheRow);
        }

        if (_row != -1)
        {
            emit m_controller->endRemoveRows();
        }
    }

    //filtering: did up to here...
    void LedgerTransactionCache::replaceItemAt(int _row, const QDate& _oldDate, bool _reloadIndex)
    {
        using namespace std::placeholders;

        //Check if we need to move it or not.
        //We need to remove it first

        auto i = std::upper_bound(m_cache.begin(), m_cache.end(), CacheItem(m_cache[_row]),
                                  std::bind(&LedgerTransactionCache::compareItems, this, _1,_2,
                                            _oldDate.isValid() ? &(m_cache[_row]) : nullptr, _oldDate));

        int newRow = i-m_cache.begin();

        //Check if the search was accurate (may be off a bit if the transaction amount has changed
        //from debit to credit or vice-versa). We just check if in order, otherwise go up and down as required.
//        while (newRow != 0 && !compareItems(m_cache[newRow-1], m_cache[_row]))
//        {
//            --newRow;
//        }
//        while (newRow < m_cache.count()-1 && !compareItems(m_cache[_row], m_cache[newRow+1]))
//        {
//            ++newRow;
//        }

        if (newRow != _row && newRow != _row+1) //No no-op moves
        {
            int reloadFrom = std::min(_row, newRow);

            //Shift the cached number of sub-rows and reload each
            //std::swap(m_cache[_row].cachedSubRowCount, m_cache[newRow].cachedSubRowCount);

            m_controller->beginMoveRows(/* src  */ QModelIndex(), _row, _row,
                                        /* dest */ QModelIndex(), newRow);

            m_cache.move(_row, _row < newRow ? newRow-1 : newRow); //When inserting back, does as if the "removed"
                                                                 //row does not exists and everything was shifted.


            if (_reloadIndex)
            {
                reloadBalanceAndIndexFrom(reloadFrom);
            }

            m_controller->endMoveRows();

            //Update the balance
            emit m_controller->dataChanged(m_controller->index(reloadFrom,               m_controller->col_balance()),
                                           m_controller->index(m_controller->rowCount(), m_controller->col_balance()));
        }
        else
        {
            updateRow(_row);

            if (_reloadIndex)
            {
                reloadBalanceAndIndexFrom(_row);
            }
        }
    }

    void LedgerTransactionCache::updateRow(int _row)
    {
        const QModelIndex parent = m_controller->index(_row, 0);

        if (cachedSubRowCount(_row) < m_controller->cacheSubRowCount(_row))
        {
            //There is not enough sub-rows, add enough.

            m_controller->beginInsertRows(parent,
                                          cachedSubRowCount(_row),
                                          m_controller->cacheSubRowCount(_row)-1);

            m_cache[_row].cachedSubRowCount = m_controller->cacheSubRowCount(_row);

            m_controller->endInsertRows();

            emit m_controller->dataChanged(parent,
                             m_controller->index(/* row    */ m_controller->cacheSubRowCount(_row)-1,
                                                 /* column */ m_controller->columnCount()-1,
                                                 /* parent */ parent));
        }
        else if (cachedSubRowCount(_row) > m_controller->cacheSubRowCount(_row))
        {
            //There is too many sub-rows, remove the extra
            m_controller->beginRemoveRows(parent,
                                          m_controller->cacheSubRowCount(_row),
                                          cachedSubRowCount(_row)-1);

            m_cache[_row].cachedSubRowCount = m_controller->cacheSubRowCount(_row);

            m_controller->endRemoveRows();

            emit m_controller->dataChanged(parent,
                             m_controller->index(/* row    */ m_controller->cacheSubRowCount(_row)-1,
                                                 /* column */ m_controller->columnCount()-1,
                                                 /* parent */ parent));
        }
        else if (m_controller->cacheSubRowCount(_row)) //If there is sub rows, reload while including them
        {
            emit m_controller->dataChanged(parent,
                                           m_controller->index(/* row    */ m_controller->cacheSubRowCount(_row)-1,
                                                               /* column */ m_controller->columnCount()-1,
                                                               /* parent */ parent));
        }
        else //No sub rows, simply reload the main element.
        {
            emit m_controller->dataChanged(parent,
                                           m_controller->index(/* row    */ _row,
                                                               /* column */ m_controller->columnCount()-1));
        }
    }

    QList<QDate> LedgerTransactionCache::nextOccurrences(const Schedule* _schedule) const
    {
        switch (m_displayPolicy)
        {
        case ScheduleDisplayPolicy::FixedNumber:
            return _schedule->nextOccurrencesDates(m_displayPolicyCount, QDate());

        case ScheduleDisplayPolicy::DaysInFuture:
            return _schedule->nextOccurrencesDates(-1, QDate::currentDate().addDays(m_displayPolicyCount));

        default:
            return {};
        }
    }

    /////////////////////////////////// DATA CHANGED SLOTS ///////////////////////////////////

    void LedgerTransactionCache::onSplitAdded(const Transaction::Split& _split, Transaction* _tr)
    {
        if (_tr->relatedTo(m_controller->account()->id()))
        {
            if (!m_transactionIndex.contains(_tr->id()))
            {
                addItem(CacheItem(_tr, m_controller->subRowCount(_tr)));
            }
            else
            {
                updateRow(m_transactionIndex[_tr->id()]);

                if (_split.idAccount == m_controller->account()->id())
                {
                    reloadBalanceAndIndexFrom(m_transactionIndex[_tr->id()]);
                }
            }
        }
    }

    void LedgerTransactionCache::onSplitRemoved(const Transaction::Split& _split, Transaction* _tr)
    {
        if (m_transactionIndex.contains(_tr->id()))
        {
            if (_tr->relatedTo(m_controller->account()->id()))
            {
                updateRow(m_transactionIndex[_tr->id()]);

                if (_split.idAccount == m_controller->account()->id())
                {
                    reloadBalanceAndIndexFrom(m_transactionIndex[_tr->id()]);
                }
            }
            else //Nothing to do with this anymore...
            {
                removeRow(m_transactionIndex[_tr->id()]);
            }
        }
    }

    void LedgerTransactionCache::onSplitAmountChanged(const Transaction::Split& _split, Transaction* _tr)
    {
        if (m_transactionIndex.contains(_tr->id()))
        {
            if (_split.idAccount == m_controller->account()->id())
            {
                replaceItemAt(m_transactionIndex[_tr->id()]);
            }
            else
            {
                //Simply reload it
                updateRow(m_transactionIndex[_tr->id()]);
            }
        }
        else if (_split.idAccount == m_controller->account()->id())
        {
            onSplitAdded(_split, _tr);
        }
    }

    void LedgerTransactionCache::onTransactionDateChanged(Transaction* _tr, const QDate& _old)
    {
        if (m_transactionIndex.contains(_tr->id()))
        {
            replaceItemAt(m_transactionIndex[_tr->id()], _old);
        }
        else if (_tr->relatedTo(m_controller->account()->id()))
        {
            onSplitAdded(_tr->splits().first(), _tr);
        }
    }

    void LedgerTransactionCache::onScheduleAdded(Schedule* s)
    {
        if (m_scheduleIndex.contains(s->id()))
        {
            onScheduleModified(s);
        }
        else if (s->transaction()->relatedTo(m_controller->account()->id()))
                 //&& m_controller->canEditTransaction(s->transaction()))
        {
            //Add the schedule
            QList<QDate> next = nextOccurrences(s);
            int min_row = m_cache.count();

            for (const QDate& d : next)
            {
                min_row = std::min(min_row,
                                   addItem(CacheItem(s, d, m_controller->subRowCount(s->transaction())), false));
            }

            //Reconstruct the balances and index
            reloadBalanceAndIndexFrom(min_row);
        }
    }

    void LedgerTransactionCache::onScheduleRemoved(Schedule* s)
    {
        //Check if we have it somewhere
        auto schedule_iter = m_scheduleIndex.find(s->id());

        if (schedule_iter != m_scheduleIndex.end())
        {
            if (schedule_iter->count())
            {
                //Remove all occurences of the schedule, starting at the last one.
                for (auto i = schedule_iter->end()-1; ; --i)
                {
                    //Reload balance and cache only if smallest row, otherwise waste of time
                    //The index entry will be removed by removeRow once it removes the last occurrence of the schedule.
                    bool isLast = i == schedule_iter->begin(); // Compute this before, as the iterator will become
                                                               // invalid if it's the last (whole QMap will be deleted)

                    removeRow(i.value(), isLast);

                    if (isLast)
                        break;
                }
            }
        }
    }

    void LedgerTransactionCache::onScheduleModified(Schedule* s)
    {
        //Check if we have it somewhere
        auto schedule_iter = m_scheduleIndex.find(s->id());

        if (schedule_iter == m_scheduleIndex.end())
        {
            onScheduleAdded(s); //Try to add it. This will check if the schedule relates to this ledger or not.
        }
        else if (!s->transaction()->relatedTo(m_controller->account()->id()))
                // || !m_controller->canEditTransaction(s->transaction()))
        {
            //The schedule has changed such as it does not relates to this ledger anymore - remove it.
            onScheduleRemoved(s);
        }
        else
        {
            //Check if list of occurences have changed, remove or add in consequence.
            QList<QDate> next = nextOccurrences(s);
            QList<QDate> add;
            QList<int> remove;

            //Find new occurrences and occurrences that are not valid anymore
            auto i_cur = next.begin();
            auto i_prev = schedule_iter->begin();

            while (i_cur != next.end() || i_prev != schedule_iter->end())
            {
                if (i_prev == schedule_iter->end() || (i_cur != next.end() && *i_cur < i_prev.key())) //New, add
                {
                    add << *i_cur;
                    ++i_cur;
                }
                else if (i_cur == next.end() || *i_cur > i_prev.key()) //Old, remove
                {
                    remove << *i_prev;
                    ++i_prev;
                }
                else
                {
                    ++i_cur;
                    ++i_prev;
                }
            }

            int min_row = m_cache.count();

            //First remove
            for (int i = remove.count()-1; i >= 0; --i)
            {
                min_row = std::min(min_row, remove[i]);
                removeRow(remove[i], add.isEmpty() && i == 0);
            }

            //Then add
            for (const QDate& d : add)
            {
                min_row = std::min(min_row,
                                   addItem(CacheItem(s, d, m_controller->subRowCount(s->transaction())), false));
            }

            reloadBalanceAndIndexFrom(min_row); //Nothing will happen if we did not move rows around.
        }
    }

    void LedgerTransactionCache::onScheduleOccurrenceEnteredOrCanceled(Schedule* s, const QDate& _instanceDate)
    {
        //Check if we have it somewhere
        if (m_scheduleIndex.contains(s->id())
            && m_scheduleIndex[s->id()].contains(_instanceDate))
        {
            // Remove it...
            removeRow(m_scheduleIndex[s->id()].take(_instanceDate));

            //Check if the display policy is the number of instances, in which case we may need to show more instances.
            if (m_displayPolicy == ScheduleDisplayPolicy::FixedNumber)
            {
                onScheduleModified(s);
            }

            if (m_scheduleIndex[s->id()].isEmpty())
            {
                m_scheduleIndex.remove(s->id());
            }
        }
    }


}


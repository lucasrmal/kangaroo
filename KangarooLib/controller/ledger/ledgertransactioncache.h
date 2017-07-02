#ifndef LEDGERTRANSACTIONCACHE_H
#define LEDGERTRANSACTIONCACHE_H

#include <QDate>
#include <QHash>
#include <QList>
#include <functional>
#include "../../amount.h"
#include "../../model/transaction.h"
#include "../../model/schedule.h"

namespace KLib
{
    class LedgerController;

    struct CacheItem
    {
        CacheItem(Transaction* _tr, int _cachedSubRowCount) :
            schedule(nullptr),
            cachedSubRowCount(_cachedSubRowCount),
            m_transaction(_tr) {}

        CacheItem(Schedule* _schedule, const QDate& _date, int _cachedSubRowCount) :
            schedule(_schedule),
            dueDate(_date),
            cachedSubRowCount(_cachedSubRowCount),
            m_transaction(nullptr) {}

        const Transaction* transaction() const { return schedule ? schedule->transaction() : m_transaction; }
        const QDate& date() const { return schedule ? dueDate : m_transaction->date(); }

        Transaction* editableTransaction() { return m_transaction; }

        Schedule* schedule;
        QDate dueDate;
        Balances runningBalance;

        int cachedSubRowCount;

    private:
        Transaction* m_transaction;

    };

    enum class ScheduleDisplayPolicy
    {
        DaysInFuture = 0,
        FixedNumber  = 1,
        NoSchedules  = 2
    };


    /**
     * @brief Cache for transactions
     *
     * This class is the cache for the transactions/schedule occurences in a LedgerController. It take
     * care of representing correctly the underlying data and updating as transactions and schedules are
     * added, modified, removed, etc.
     *
     * It also provides filtering and sorting functions, in addition to keeping track of the running balance.
     *
     * We want O(1) index access. Other things may be slower, but should still be efficient. Keep in mind that
     * most changes occur at the end of the list.
     */
    class LedgerTransactionCache : public QObject
    {
        Q_OBJECT

        public:            
            typedef QList<CacheItem> CacheItemList;
            typedef const CacheItem& const_reference;
            typedef CacheItem& reference;

            /**
             * @brief Returns true if first < second, false otherwise.
             */
            typedef std::function<bool(const_reference, const_reference)> fn_compare;
            typedef std::function<bool(const_reference)> fn_filter;


            LedgerTransactionCache(LedgerController* _controller);

            const_reference operator[](int _index) const { return currentList()[_index]; }
            reference       operator[](int _index)       { return currentList()[_index]; }

            const Balances& balanceAt(int _index) const { return currentList()[_index].runningBalance; }

            bool    isEmpty() const { return currentList().isEmpty(); }
            int     count() const   { return currentList().count(); }
            int     size() const    { return count(); }

            void    setFilter(fn_filter _keep);
            void    removeFilter();
            void    setSorting(fn_compare _compare);

            /**
             * @brief compareItems
             *
             * The params _tr and _date can be used to override the date of a specific transaction.
             */
            bool compareItems(const_reference _first, const_reference _second,
                              const CacheItem* _item = nullptr, const QDate& _date = QDate()) const;

            void reloadData();

        private slots:
            //Transaction signals
            void onSplitAdded(const Transaction::Split& _split, Transaction* _tr);
            void onSplitRemoved(const Transaction::Split& _split, Transaction* _tr);
            void onSplitAmountChanged(const Transaction::Split& _split, Transaction* _tr);
            void onTransactionDateChanged(Transaction* _tr, const QDate& _old);

            //Schedule signals
            void onScheduleAdded(Schedule* s);
            void onScheduleRemoved(Schedule* s);
            void onScheduleModified(Schedule* s); ///< @todo What if transaction changes...

            void onScheduleOccurrenceEnteredOrCanceled(Schedule* s, const QDate& _instanceDate);

        private:
            void reloadBalanceAndIndexFrom(int _from);

            int cachedSubRowCount(int _cacheRow) const;

            /**
             * @brief Adds an item and returns the row of the inserted item.
             */
            int addItem(const CacheItem& _item, bool _reloadIndex = true);

            /// @todo: Remove and modify should also work on the filters...
            void removeRow(int _row, bool _reloadIndex = true);

            /**
             * @brief Checks if the item at _row is at the correct location, if not, replaces it correctly.
             * @param _row
             */
            void replaceItemAt(int _row, const QDate& _oldDate = QDate(), bool _reloadIndex = true);


            void updateRow(int _row);

            QList<QDate> nextOccurrences(const Schedule* _schedule) const;


            const CacheItemList& currentList() const    { return m_filterIsApplied ? m_filteredSortedCache : m_cache; }
            CacheItemList& currentList()                { return m_filterIsApplied ? m_filteredSortedCache : m_cache; }

            LedgerController* m_controller;

            QHash<int, int>                m_transactionIndex;  ///< Transaction ID, Row
            QHash<int, QMap<QDate, int> >  m_scheduleIndex;     ///< Schedule ID, (Date, Row)

            QHash<int, int> m_cacheToFilteredMap; ///< index in cache, index in filtered cache

            //need to reload m_cacheToFilteredMap too!!!

            CacheItemList m_cache;
            CacheItemList m_filteredSortedCache;
            bool m_filterIsApplied;
            fn_filter m_filterFunction;

            ScheduleDisplayPolicy m_displayPolicy;
            int m_displayPolicyCount;     ///< Either num of days in future or number
                                          ///< of fixed occurences depending on policy

    };

}

#endif // LEDGERTRANSACTIONCACHE_H

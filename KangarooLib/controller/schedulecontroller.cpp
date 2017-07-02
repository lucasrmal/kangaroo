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

#include "schedulecontroller.h"
#include "../ui/core.h"

namespace KLib
{

    ScheduleController::ScheduleController(QObject *parent) :
        QAbstractItemModel(parent)
    {
        loadData();

        connect(ScheduleManager::instance(), SIGNAL(scheduleAdded(KLib::Schedule*)), this, SLOT(onScheduleAdded(KLib::Schedule*)));
        connect(ScheduleManager::instance(), SIGNAL(scheduleRemoved(KLib::Schedule*)), this, SLOT(onScheduleRemoved(KLib::Schedule*)));
        connect(ScheduleManager::instance(), SIGNAL(scheduleModified(KLib::Schedule*)), this, SLOT(onScheduleModified(KLib::Schedule*)));
    }

    int ScheduleController::rowCount(const QModelIndex& _parent) const
    {
        if (_parent.column() > 0)
            return 0;

        if (!_parent.isValid())
        {
            return Frequency::NumFrequencies;
        }
        else if (_parent.internalPointer() == NULL
                 && _parent.row() < Frequency::NumFrequencies
                 && _parent.row() >= 0)
        {
            return m_schedules[_parent.row()].count();
        }
        else
        {
            return 0;
        }
    }

    int ScheduleController::columnCount(const QModelIndex& _parent) const
    {
        Q_UNUSED(_parent)

        return 1;
    }

    QVariant ScheduleController::data(const QModelIndex& _index, int _role) const
    {
        if (!_index.isValid())
        {
            return QVariant();
        }

        if (_index.internalPointer() == NULL)
        {
            if (_role == Qt::DisplayRole)
            {
                return Frequency::frequencyToString(m_frequencies[_index.row()]);
            }
            else if (_role == Qt::DecorationRole)
            {
                return Core::icon("folder-yellow");
            }
        }
        else // _index.internalPointer() != NULL
        {
            Schedule* s = static_cast<Schedule*>(_index.internalPointer());

            if (_role == Qt::DisplayRole)
            {
                return s->description();
            }
            else if (_role == Qt::UserRole)
            {
                return s->id();
            }
        }

        return QVariant();
    }

    QVariant ScheduleController::headerData(int _section, Qt::Orientation _orientation, int _role) const
    {
        Q_UNUSED(_section)

        if (_orientation == Qt::Vertical || _role != Qt::DisplayRole)
            return QVariant();

        return tr("Schedules");
    }

    QModelIndex ScheduleController::index(int _row, int _column, const QModelIndex& _parent) const
    {
        if (!hasIndex(_row, _column, _parent))
            return QModelIndex();

        if (!_parent.isValid())
        {
            return createIndex(_row, _column, (void*)NULL);
        }
        else
        {
            int idxPar = _parent.row();

            if (idxPar >= 0 && idxPar < Frequency::NumFrequencies && _row < m_schedules[idxPar].count())
            {
                return createIndex(_row, _column, m_schedules[idxPar].at(_row));
            }
            else
            {
                return QModelIndex();
            }
        }
    }

    QModelIndex ScheduleController::parent(const QModelIndex& _child) const
    {
        if (!_child.isValid() || _child.internalPointer() == NULL)
        {
            return QModelIndex();
        }

        Schedule* s = static_cast<Schedule*>(_child.internalPointer());

        int parentRow = m_freqToIndex[s->recurrence().frequency];

        return createIndex(parentRow, 0, (void*)NULL);
    }

    void ScheduleController::onScheduleAdded(KLib::Schedule* s)
    {
        beginInsertRows(createIndex(m_freqToIndex[s->recurrence().frequency], 0, (void*)NULL),
                        m_schedules[m_freqToIndex[s->recurrence().frequency]].count(),
                        m_schedules[m_freqToIndex[s->recurrence().frequency]].count());
        m_schedules[m_freqToIndex[s->recurrence().frequency]].append(s);
        endInsertRows();
    }

    void ScheduleController::onScheduleModified(KLib::Schedule* s)
    {
        int curFreqIdx = -1;

        for (int i = 0; curFreqIdx == -1 && i < Frequency::NumFrequencies; ++i)
        {
            if (m_schedules[i].contains(s))
            {
                curFreqIdx = i;
            }
        }

        if (curFreqIdx == -1)
        {
            onScheduleAdded(s);
        }
        else if (m_frequencies[curFreqIdx] != s->recurrence().frequency)
        {
            int schIdx = m_schedules[curFreqIdx].indexOf(s);

            if (schIdx != -1)
            {
                beginRemoveRows(createIndex(curFreqIdx, 0, (void*)NULL),
                                schIdx, schIdx);
                m_schedules[curFreqIdx].removeAt(schIdx);
                endRemoveRows();
            }

            onScheduleAdded(s);
        }
        else
        {
            emit dataChanged(createIndex(m_schedules[curFreqIdx].indexOf(s), 0, s),
                             createIndex(m_schedules[curFreqIdx].indexOf(s), 0, s));
        }

    }

    void ScheduleController::onScheduleRemoved(KLib::Schedule* s)
    {
        int curFreqIdx = m_freqToIndex[s->recurrence().frequency];
        int schIdx = m_schedules[curFreqIdx].indexOf(s);

        if (schIdx != -1)
        {
            beginRemoveRows(createIndex(curFreqIdx, 0, (void*)NULL),
                            schIdx, schIdx);
            m_schedules[curFreqIdx].removeAt(schIdx);
            endRemoveRows();
        }
    }

    void ScheduleController::loadData()
    {
        //Load frequencies
        m_frequencies << Frequency::Once    << Frequency::Daily     << Frequency::Weekly
                      << Frequency::Monthly << Frequency::Yearly;

        m_freqToIndex[Frequency::Once]      = 0;
        m_freqToIndex[Frequency::Daily]     = 1;
        m_freqToIndex[Frequency::Weekly]    = 2;
        m_freqToIndex[Frequency::Monthly]   = 3;
        m_freqToIndex[Frequency::Yearly]    = 4;

        //Load the data
        for (Schedule* s : ScheduleManager::instance()->schedules())
        {
            m_schedules[m_freqToIndex[s->recurrence().frequency]].append(s);
        }

    }

} //Namespace

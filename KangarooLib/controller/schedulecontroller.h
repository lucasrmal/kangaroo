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

#ifndef SCHEDULEMODEL_H
#define SCHEDULEMODEL_H

#include <QAbstractItemModel>
#include "../model/schedule.h"

namespace KLib
{

    class ScheduleController : public QAbstractItemModel
    {
        Q_OBJECT

        public:
            explicit ScheduleController(QObject* _parent = 0);

            int rowCount(const QModelIndex& _parent) const override;
            int columnCount(const QModelIndex& _parent) const override;

            QVariant data(const QModelIndex& _index, int _role) const override;
            QVariant headerData(int _section, Qt::Orientation _orientation, int _role) const override;

            QModelIndex index(int _row, int _column, const QModelIndex& _parent) const override;
            QModelIndex parent(const QModelIndex& _child) const override;

        signals:

        private slots:
            void onScheduleAdded(KLib::Schedule* s);
            void onScheduleModified(KLib::Schedule* s);
            void onScheduleRemoved(KLib::Schedule* s);

        private:
            void loadData();

            QList<KLib::Schedule*> m_schedules[Frequency::NumFrequencies];
            QList<int> m_frequencies;
            QHash<int, int> m_freqToIndex;

    };

} //Namespace

#endif // SCHEDULEMODEL_H

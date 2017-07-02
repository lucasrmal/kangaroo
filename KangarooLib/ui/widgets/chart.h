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

#ifndef CHART_H
#define CHART_H

#include <QScrollArea>
class QAbstractItemModel;

namespace KLib
{
    class ChartView;

    namespace ChartType
    {
        enum Type
        {
            VBar,
            HBar,
            Line,
            Area,
            Dots,
            Pie
        };
    }

    namespace GridFlag
    {
        enum Flag
        {
            None = 0x00,
            V    = 0x01,
            H    = 0x02,
            All  = V | H
        };
    }

    class Chart : public QScrollArea
    {
        Q_OBJECT
        public:

            explicit Chart(QWidget* _parent = 0);

            void setModel(QAbstractItemModel* _model, Qt::Orientation _orientation, int _section);
            QAbstractItemModel* model() { return m_model; }

            double maximumValue() const { return m_maxValue; }
            double minimumValue() const { return m_minValue; }
            double averageValue() const { return m_averageValue; }

            Qt::Orientation orientation() const { return m_orientation; }
            int section() const { return m_section; }
            QColor defaultColor() const { return QColor(Qt::darkBlue); }

            ChartType::Type chartType() const { return m_type; }
            void setChartType(ChartType::Type _type) { m_type = _type; update(); }

            bool keyVisible() const { return m_keyVisible; }
            void setKeyVisible(bool _visible) { m_keyVisible = _visible; update(); }

            int grid() const { return m_grid; }
            void setGrid(int _flags) { m_grid = _flags; update(); }

            bool showLabels() const { return m_showLabels; }
            void setShowLabels(bool _show) { m_showLabels = _show; update(); }

            bool showAverage() const { return m_showAverage; }
            void setShowAverage(bool _show) { m_showAverage = _show; update(); }

            int ceilVal() const { return m_ceiling; }
            int floorVal() const { return m_floor; }
            double intervalSize() const { return m_intervalSize; }
            int intervalCount() const { return m_intervalCount; }

        protected slots:
            void onDataModified();
            void onColumnRowCountChanged();


        signals:

        protected:
            void updateIntervals();

        private:
            QAbstractItemModel* m_model;
            ChartView* m_view;

            double m_maxValue;
            double m_minValue;
            double m_averageValue;
            Qt::Orientation m_orientation;
            int m_section;
            bool m_keyVisible;
            int m_grid;
            bool m_showLabels;
            bool m_showAverage;
            ChartType::Type m_type;

            int m_ceiling;
            int m_floor;
            double m_intervalSize;
            int m_intervalCount;

    };

    class ChartView : public QWidget
    {
        Q_OBJECT

        public:
            ChartView(Chart* _parent);

            QSize minimumSizeHint() const;
            QSize sizeHint() const;

        protected:
            void paintEvent(QPaintEvent *event);

        private:
            Chart* m_chart;

            static const quint32 MAX_INTERVALS      = 10;
            static const quint32 MARGIN_SIZE        = 5;
            static const quint32 FONT_SIZE          = 8;
    };

}

#endif // CHART_H

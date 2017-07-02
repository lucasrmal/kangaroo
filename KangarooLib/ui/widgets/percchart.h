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
 *
 */

/*
 * THIS FILE CONTAINS CODE FROM THE Qt Toolkit Examples. SEE FOLLOWING LICENCE:
 */

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
****************************************************************************/

#ifndef PERCCHART_H
#define PERCCHART_H

#include <QAbstractItemView>
#include <QFont>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QRect>
#include <QSize>
#include <QPoint>
#include <QWidget>

class QRubberBand;

namespace KLib
{

    enum class ChartStyle
    {
        Pie,
        HorizontalBar
    };

    class PercChart : public QAbstractItemView
    {
        Q_OBJECT

            typedef std::tuple<QString, double, int, double> ItemTuple; //Caption, %, row, value

        public:
            explicit PercChart(ChartStyle _style, int _captionCol, int _dataCol, Qt::ItemDataRole _dataRole, QWidget *parent = nullptr);

            QRect visualRect(const QModelIndex &index) const override;
            void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
            QModelIndex indexAt(const QPoint &point) const override;

            void setModel(QAbstractItemModel* _model) override;

            void setShowTotals(bool _show) { m_showTotals = _show; viewport()->update(); }
            bool totalsShown() const { return m_showTotals; }

            void setCaptionColumn(int _column);
            void setLegendColumn(int _column);
            void setDataColumn(int _column);

        public slots:
            void resetModel();

        protected slots:
            void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> & roles = QVector<int>()) override;
            void rowsInserted(const QModelIndex &parent, int start, int end) override;
            void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;

        protected:
            bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);
            QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
                                Qt::KeyboardModifiers modifiers);

            int horizontalOffset() const override;
            int verticalOffset() const override;

            bool isIndexHidden(const QModelIndex&) const override { return false; }

            void setSelection(const QRect&, QItemSelectionModel::SelectionFlags) override {}

            void paintEvent(QPaintEvent *event) override;
            void resizeEvent(QResizeEvent *event) override;
            void scrollContentsBy(int dx, int dy) override;

            void mouseMoveEvent(QMouseEvent * event) override;

            QRegion visualRegionForSelection(const QItemSelection &selection) const override;

        private:
            QRect itemRect(const QModelIndex &item) const;
            int rows(const QModelIndex &index = QModelIndex()) const;
            void updateGeometries();

            void updateItems();

            QList<ItemTuple> m_items;
            bool lastIsOther;
            int currentHoverRow;

            QColor lineColor;

            int marginLegendH;
            int marginLegendV;

            int marginPieH;
            int marginPieV;

            int marginBarH;
            int marginBarV;

            int totalSizeH;
            int totalSizeV;

            int pieSize;
            int pieWidth;

            int armLength;
            int otherArmLength;
            int maxCaptionWidth;
            int validItems;
            double totalValue;
            QPoint origin;
            QRubberBand *rubberBand;

            ChartStyle m_style;
            int m_captionCol;
            int m_dataCol;
            int m_legendCol;
            Qt::ItemDataRole m_dataRole;

            bool m_showTotals;

            static const QStringList CHART_COLORS;
    };

}

#endif // PERCCHART_H

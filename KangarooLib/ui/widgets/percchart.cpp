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

#include "percchart.h"

#include <QPainter>
#include <QScrollBar>
#include <QMouseEvent>
#include <QPaintEvent>
#include <cmath>
#include <tuple>
#include <QDebug>

#define TO_RAD(angle) (angle)*(M_PI/180.0)

#define MIN_HEIGHT 500

namespace KLib
{

const QStringList PercChart::CHART_COLORS = {"#59B5CA", "#179443", "#71B62F", "#FABF00", "#E98248",
                                            "#D0444D", "#B92049", "#B156B8", "#7066D2", "#1358A7"};

PercChart::PercChart(ChartStyle _style, int _captionCol, int _dataCol, Qt::ItemDataRole _dataRole, QWidget *parent) :
    QAbstractItemView(parent),
    lastIsOther(false),
    currentHoverRow(-1),
    m_style(_style),
    m_captionCol(_captionCol),
    m_dataCol(_dataCol),
    m_legendCol(_captionCol),
    m_dataRole(_dataRole),
    m_showTotals(true)
{
    horizontalScrollBar()->setRange(0, 0);
    verticalScrollBar()->setRange(0, 0);

    lineColor = QColor("#E6E6E6");

    totalSizeH =  MIN_HEIGHT;
    totalSizeV = 400;

    marginLegendH = 10;
    marginLegendV = 10;

    marginBarH = 90;
    marginBarV = 40;

    marginPieH = 110;
    marginPieV = 50;
    pieSize = totalSizeV-(2*marginPieV);

    armLength = 30;
    otherArmLength = 6;
    maxCaptionWidth = 100;
    pieWidth = 50;
    validItems = 0;
    totalValue = 0.0;
    rubberBand = 0;

    setMouseTracking(true);
}

void PercChart::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> & roles)
{
    QAbstractItemView::dataChanged(topLeft, bottomRight, roles);

    validItems = 0;
    totalValue = 0.0;

    for (int row = 0; row < model()->rowCount(rootIndex()); ++row)
    {
        QModelIndex index = model()->index(row, m_dataCol, rootIndex());
        double value = model()->data(index, m_dataRole).toDouble();

        if (value > 0.0)
        {
            totalValue += value;
            validItems++;
        }
    }

    updateItems();

    viewport()->update();
}

bool PercChart::edit(const QModelIndex&, EditTrigger, QEvent*)
{
    return false;
}

void PercChart::resetModel()
{
    totalValue = 0.0;
    validItems = 0;

    for (int row = 0; row < model()->rowCount(rootIndex()); ++row)
    {
        QModelIndex index = model()->index(row, m_dataCol, rootIndex());
        double value = model()->data(index, m_dataRole).toDouble();

        if (value > 0.0) {
            totalValue += value;
            validItems++;
        }
    }

    updateItems();

    viewport()->update();
}

void PercChart::setModel(QAbstractItemModel* _model)
{
    QAbstractItemView::setModel(_model);
    resetModel();

    connect(_model, SIGNAL(modelReset()), this, SLOT(resetModel()));
}

void PercChart::setCaptionColumn(int _column)
{
    m_captionCol = _column;
    viewport()->update();
}

void PercChart::setLegendColumn(int _column)
{
    m_legendCol = _column;
    viewport()->update();
}

void PercChart::setDataColumn(int _column)
{
    m_dataCol = _column;
    updateItems();
    viewport()->update();
}

/*
     Returns the item that covers the coordinate given in the view.
 */

QModelIndex PercChart::indexAt(const QPoint &point) const
{
    if (validItems == 0)
        return QModelIndex();

    // Transform the view coordinates into contents widget coordinates.
    int wx = point.x() + horizontalScrollBar()->value();
    int wy = point.y() + verticalScrollBar()->value();

    if (wx < totalSizeH) //Pie section
    {
        double cx = wx - totalSizeH/2;
        double cy = totalSizeV/2 - wy; // positive cy for items above the center

        // Determine the distance from the center point of the pie chart.
        double d = pow(pow(cx, 2) + pow(cy, 2), 0.5);

        //If too far or too close to the center point
        if (d == 0 || d > pieSize/2 || d < (pieSize/2)-pieWidth)
            return QModelIndex();

        // Determine the angle of the point.
        double angle = (180 / M_PI) * acos(cx/d);
        if (cy < 0)
            angle = 360 - angle;

        // Find the relevant slice of the pie.
        double startAngle = 0.0;

        for (const ItemTuple& item : m_items)
        {
            if (std::get<1>(item) > 0.0)
            {
                double sliceAngle = 360.0*std::get<1>(item);

                if (angle >= startAngle && angle < (startAngle + sliceAngle))
                {
                    return model()->index(std::get<2>(item), 1, rootIndex());
                }

                startAngle += sliceAngle;
            }
        }
    }
    else if (wx > totalSizeH+marginLegendH && wx < 2*totalSizeH-marginLegendH) //Legend section
    {
        int rowHeight = (totalSizeV-(2*marginLegendV))/(CHART_COLORS.size()+2);
        int curRow = ((wy - marginLegendV) / rowHeight) - 1; //First row is title so does not count

        if (curRow >= 0 && curRow < m_items.size())
        {
            return model()->index(std::get<2>(m_items[curRow]), 1, rootIndex());
        }
    }
//    else
//    {
//        double itemHeight = QFontMetrics(viewOptions().font).height();
//        int listItem = int((wy - margin) / itemHeight);
//        int validRow = 0;

//        for (int row = 0; row < model()->rowCount(rootIndex()); ++row)
//        {
//            QModelIndex index = model()->index(row, m_dataRow, rootIndex());
//            if (model()->data(index).toDouble() > 0.0)
//            {

//                if (listItem == validRow)
//                    return model()->index(row, 0, rootIndex());

//                // Update the list index that corresponds to the next valid row.
//                validRow++;
//            }
//        }
//    }

    return QModelIndex();
}

/*
     Returns the rectangle of the item at position \a index in the
     model. The rectangle is in contents coordinates.
 */

QRect PercChart::itemRect(const QModelIndex &index) const
{
    if (!index.isValid())
        return QRect();

    // Check whether the index's row is in the list of rows represented
    // by slices.
    QModelIndex valueIndex;

    if (index.column() != 1)
        valueIndex = model()->index(index.row(), 1, rootIndex());
    else
        valueIndex = index;

    if (model()->data(valueIndex).toDouble() > 0.0)
    {
        int listItem = 0;
        for (int row = index.row()-1; row >= 0; --row)
        {
            if (model()->data(model()->index(row, 1, rootIndex())).toDouble() > 0.0)
                listItem++;
        }

        double itemHeight;

        switch (index.column()) {
        case 0:
            itemHeight = QFontMetrics(viewOptions().font).height();

            return QRect(totalSizeH,
                         int(marginPieV + listItem*itemHeight),
                         totalSizeH - marginPieH,
                         int(itemHeight));
        case 1:
            return viewport()->rect();
        }

    }
    return QRect();
}

//QRegion Chart::itemRegion(const QModelIndex &index) const
//{
//    if (!index.isValid())
//        return QRegion();

//    if (index.column() != 1)
//        return itemRect(index);

//    if (model()->data(index).toDouble() <= 0.0)
//        return QRegion();

//    double startAngle = 0.0;
//    for (int row = 0; row < model()->rowCount(rootIndex()); ++row) {

//        QModelIndex sliceIndex = model()->index(row, m_dataRow, rootIndex());
//        double value = model()->data(sliceIndex).toDouble();

//        if (value > 0.0) {
//            double angle = 360*value/totalValue;

//            if (sliceIndex == index) {
//                QPainterPath slicePath;
//                slicePath.moveTo(totalSize/2, totalSize/2);
//                slicePath.arcTo(margin, margin, margin+pieSize, margin+pieSize,
//                                startAngle, angle);
//                slicePath.closeSubpath();

//                return QRegion(slicePath.toFillPolygon().toPolygon());
//            }

//            startAngle += angle;
//        }
//    }

//    return QRegion();
//}

int PercChart::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

//void Chart::mousePressEvent(QMouseEvent *event)
//{
//    QAbstractItemView::mousePressEvent(event);
//    origin = event->pos();
//    if (!rubberBand)
//        rubberBand = new QRubberBand(QRubberBand::Rectangle, viewport());
//    rubberBand->setGeometry(QRect(origin, QSize()));
//    rubberBand->show();
//}

//void Chart::mouseMoveEvent(QMouseEvent *event)
//{
//    if (rubberBand)
//        rubberBand->setGeometry(QRect(origin, event->pos()).normalized());
//    QAbstractItemView::mouseMoveEvent(event);
//}

//void Chart::mouseReleaseEvent(QMouseEvent *event)
//{
//    QAbstractItemView::mouseReleaseEvent(event);
//    if (rubberBand)
//        rubberBand->hide();
//    viewport()->update();
//}

QModelIndex PercChart::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                Qt::KeyboardModifiers /*modifiers*/)
{
    QModelIndex current = currentIndex();

    switch (cursorAction) {
    case MoveLeft:
    case MoveUp:
        if (current.row() > 0)
            current = model()->index(current.row() - 1, current.column(),
                                     rootIndex());
        else
            current = model()->index(0, current.column(), rootIndex());
        break;
    case MoveRight:
    case MoveDown:
        if (current.row() < rows(current) - 1)
            current = model()->index(current.row() + 1, current.column(),
                                     rootIndex());
        else
            current = model()->index(rows(current) - 1, current.column(),
                                     rootIndex());
        break;
    default:
        break;
    }

    viewport()->update();
    return current;
}

void PercChart::updateItems()
{
    m_items.clear();

    for (int i = 0; i < model()->rowCount(rootIndex()); ++i)
    {
        QModelIndex captionIndex = model()->index(i, m_captionCol);
        QModelIndex dataIndex = model()->index(i, m_dataCol);

        double value = model()->data(dataIndex, m_dataRole).toDouble();

        if (value > 0)
        {
            m_items.append(std::make_tuple(model()->data(captionIndex, Qt::DisplayRole).toString(),
                                           value / totalValue,
                                           i,
                                           value));
        }
    }

    //Sort
    std::sort(m_items.begin(), m_items.end(), [] (const ItemTuple& _first, const ItemTuple& _second) {
        return std::get<1>(_first) > std::get<1>(_second); //Largest first
    });

    //Check if too many items
    if (m_style == ChartStyle::HorizontalBar || m_items.size() <= CHART_COLORS.size())
    {
        lastIsOther = false;

        if (m_items.size() > CHART_COLORS.size())
        {
            totalSizeV = std::max(MIN_HEIGHT, 20*m_items.size());
            updateGeometries();
        }
    }
    else
    {
        std::get<0>(m_items[CHART_COLORS.size()-1]) = tr("Other");
        lastIsOther = true;

        while (m_items.size() > CHART_COLORS.size())
        {
            std::get<1>(m_items[CHART_COLORS.size()-1]) += std::get<1>(m_items.last());
            std::get<3>(m_items[CHART_COLORS.size()-1]) += std::get<3>(m_items.last());
            m_items.removeLast();
        }
    }


}

void PercChart::mouseMoveEvent(QMouseEvent* event)
{
    QModelIndex indexUnderMouse = indexAt(viewport()->mapFromParent(event->pos()));
    setCurrentIndex(indexUnderMouse);

    if (currentHoverRow != indexUnderMouse.row())
    {
        currentHoverRow = indexUnderMouse.row();
        viewport()->update();
    }
}

void PercChart::paintEvent(QPaintEvent *event)
{
    QStyleOptionViewItem option = viewOptions();

    QBrush background = option.palette.base();
    QPen foreground(option.palette.color(QPalette::WindowText));

    QLocale locale;

    QPen noPen;
    noPen.setStyle(Qt::NoPen);

    QPen linePen;
    linePen.setColor(lineColor);
    linePen.setWidth(1);

    QFont captionFont;
    captionFont.setPointSize(7);
    QFontMetrics captionMetrics(captionFont);

    QFont barPercFont;
    barPercFont.setBold(true);
    barPercFont.setPointSize(10);

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(event->rect(), background);


    const int FULL_WIDTH = std::max(viewport()->width(), totalSizeH*2);


    if (validItems > 0)
    {

        //============================================== Paint the chart ==============================================

        painter.save();

        switch (m_style)
        {
        case ChartStyle::Pie:
        {
            // Viewport rectangles
            QRect pieRect = QRect(marginPieH, marginPieV, pieSize, pieSize);

            painter.translate(pieRect.x() - horizontalScrollBar()->value(),
                              pieRect.y() - verticalScrollBar()->value());
            painter.drawEllipse(0, 0, pieSize, pieSize);
            double startAngle = 0.0;
            const double radius = ((double)pieSize)/2;

            int selected = -1;

            int i = 0;
            for (const ItemTuple& item : m_items)
            {
                if (std::get<2>(item) == currentHoverRow) //currentIndex().row()
                {
                    selected = i;
                }

                double angle = 360*std::get<1>(item);

                painter.setPen(QPen(CHART_COLORS[i]));
                painter.setBrush(QColor(CHART_COLORS[i]));

                //Draw the pie slice
                painter.drawPie(0, 0, pieSize, pieSize,
                                int(startAngle*16), int(angle*16));

                //Draw the arm
                double angleRad = TO_RAD(startAngle+(angle/2));

                //If on left side of pie, arm extends left, otherwise it extends right.
                bool toLeft = angleRad > M_PI/2 && angleRad < 1.5*M_PI;

                QPointF lineStart(radius + (cos(angleRad)*radius),
                                 radius - (sin(angleRad)*radius));
                QPointF lineElbow(radius + (cos(angleRad)*(radius+armLength)),
                                 radius - (sin(angleRad)*(radius+armLength)));
                QPointF lineEnd(lineElbow.x() + (toLeft ? -otherArmLength
                                                        :  otherArmLength),
                                lineElbow.y());

                QPointF polyLine[3] = {lineStart, lineElbow, lineEnd};

                painter.setPen(linePen);
                painter.drawPolyline(polyLine, 3);

                //Draw the caption at the end of the arm
                QRectF textRect(/* left */ toLeft ? lineEnd.x()-(maxCaptionWidth+5)
                                                   : lineEnd.x()+5,
                                /* top  */ lineEnd.y()-5,
                                /* widt */ maxCaptionWidth,
                                /* heig */ captionMetrics.height()*2 + 4);

                QString text = std::get<0>(item);

                if (std::get<1>(item) >= 0.02)
                {
                    text += "\n" + locale.toString(std::get<3>(item), 'f', 2);
                }

                painter.setPen(foreground);
                painter.setFont(captionFont);

                if (std::get<1>(item) >= 0.005)
                {
                    painter.drawText(/* rect  */ textRect,
                                     /* flags */ toLeft ? Qt::AlignRight : Qt::AlignLeft,
                                     /* text  */ text);
                }

                startAngle += angle;
                ++i;
            }


            //Hollow the center of the pie chart for a "fancy look" ;-)
            painter.setPen(noPen);
            painter.setBrush(Qt::white);
            painter.drawEllipse(pieWidth, pieWidth, pieSize-(2*pieWidth), pieSize-(2*pieWidth));

            //Draw the name in the center
            if (selected != -1)
            {
                QFont captionFont;
                captionFont.setPointSize(9);

                painter.setFont(captionFont);
                painter.setPen(foreground);
                painter.drawText(QRect(QPoint(pieWidth, pieSize/2), QPoint(pieSize-pieWidth, pieSize/2+50)),
                                 Qt::AlignCenter,
                                 std::get<0>(m_items[selected]));

                QFont percFont;
                percFont.setPointSize(20);

                painter.setPen(CHART_COLORS[selected]);
                painter.setFont(percFont);
                painter.drawText(QRect(pieWidth, pieWidth, pieSize-(2*pieWidth), pieSize-(2*pieWidth)),
                                 Qt::AlignCenter,
                                 locale.toString(std::get<1>(m_items[selected])*100., 'f', 1) + " %");
            }
            break;
        }

        case ChartStyle::HorizontalBar:
        {
            // Viewport rectangles
            int chartSizeH = FULL_WIDTH - totalSizeH-(2*marginBarH)-(2*marginLegendH);
            int chartSizeV = totalSizeV-(2*marginBarV);

            QRect chartRect = QRect(marginBarH, marginBarV, chartSizeH, chartSizeV);


            painter.translate(chartRect.x() - horizontalScrollBar()->value(),
                              chartRect.y() - verticalScrollBar()->value());

            //Paint the lines
            QPoint topPoint(0,0);
            QPoint originPoint(0,chartSizeV);
            QPoint rightPoint(chartSizeH, chartSizeV);

            QPoint allPoints[3] = {topPoint, originPoint, rightPoint};

            painter.setPen(linePen);
            painter.drawPolyline(allPoints, 3);

            //Find the max value
            double max = 0;

            for (const ItemTuple& item : m_items)
            {
                if (std::get<1>(item) > max)
                {
                    max = std::get<1>(item);
                }
            }

            //Get the max in 10's of percent
            max *= 10;
            int maxLevel = int(ceil(max));
            double levelSize = 10;

            if (maxLevel < 2)
            {
                maxLevel = 4;
                levelSize = 2.5;
            }
            else if (maxLevel < 3)
            {
                maxLevel = 4;
                levelSize = 5;
            }
            else if (maxLevel < 4)
            {
                maxLevel = 6;
                levelSize = 5;
            }

            int spacingH = chartSizeH / maxLevel;

            //Write the bottom legend
            QLine hSep(spacingH, chartSizeV-3, spacingH, chartSizeV+3);

            painter.setFont(captionFont);

            for (int i = 0; i < maxLevel; ++i)
            {
                //Lines
                painter.setPen(linePen);
                painter.drawLine(hSep);

                //Label
                painter.setPen(foreground);
                painter.drawText(QRect(hSep.x1()-20, hSep.y2()+3, 40, 30),
                                 Qt::AlignHCenter | Qt::AlignTop,
                                 locale.toString(levelSize*(i+1), 'f', 1) + " %");

                hSep.translate(spacingH, 0);

            }

            //Draw the bars
            int spacingV = chartSizeV / m_items.count();
            int barThickness = int(0.6*spacingV);

            int curY = (spacingV-barThickness)/2;


            for (int i = 0; i < m_items.count(); ++i)
            {
                const ItemTuple& item = m_items[i];

                double length = ((double) spacingH / levelSize) * 100.0 * std::get<1>(item);
                QRect barRect(0, curY, length, barThickness);

                //Bar
                painter.setPen(noPen);
                painter.setBrush(QColor(CHART_COLORS[i % CHART_COLORS.size()]));
                painter.drawRoundedRect(barRect, 2.0, 2.0);


                //Caption
                painter.setFont(captionFont);
                painter.setPen(foreground);
                painter.drawText(QRect(-marginBarH, i*spacingV, marginBarH-4, spacingV),
                                 Qt::AlignVCenter | Qt::AlignRight,
                                 std::get<0>(item));

                //Value
                painter.drawText(QRect(length+4, i*spacingV, marginBarH-4, spacingV),
                                 Qt::AlignVCenter | Qt::AlignLeft,
                                 locale.toString(std::get<3>(item), 'f', 2));

                //Percent
                if (true) //std::get<2>(item) == currentHoverRow)
                {
                    painter.setFont(barPercFont);

                    if (length >= 40) //Big - show it inside
                    {
                        painter.setPen(Qt::white);
                        painter.drawText(barRect,
                                         Qt::AlignCenter,
                                         locale.toString(std::get<1>(item)*100., 'f', 1) + " %");
                    }
                    else //Small - show it next to it
                    {
                        painter.setPen(QColor(CHART_COLORS[i % CHART_COLORS.size()]));
                        painter.drawText(QRect(QPoint(length+8+spacingV, i*spacingV),
                                               QPoint(chartSizeH, (i+1)*spacingV)),
                                         Qt::AlignVCenter | Qt::AlignLeft,
                                         locale.toString(std::get<1>(item)*100., 'f', 1) + " %");


                    }
                }

                //painter.drawLine(0, i*spacingV, 100, i*spacingV);
                curY += spacingV;
            }



            break;
        }
        }




        painter.restore();

        //============================================== Paint the legend ==============================================

        QFont legendFont;
        legendFont.setPointSize(9);
        QFontMetrics legendMetrics(legendFont);


        const int rowHeight = legendMetrics.height()*2+5; //(totalSizeV-(2*marginLegendV))/(CHART_COLORS.size()+2);//;
        const int colSep = 5;
        const int colorSize = legendMetrics.height()-2;
        QLine rowLine(QPoint(0, rowHeight), QPoint(totalSizeH-marginLegendH, rowHeight));

        int allColWidth = (totalSizeH-marginLegendH) - (colSep*5); //4 rows + end
        int columnWidths[4] = {int(0.1 * allColWidth), int(0.4 * allColWidth), int(0.25 * allColWidth), int(0.25 * allColWidth) };

        painter.save();

        painter.translate(FULL_WIDTH-(marginLegendH+totalSizeH), marginLegendV);
        //painter.setRenderHint(QPainter::Antialiasing, false);

        //Draw row labels
        int curX = colSep*2+columnWidths[0];

        legendFont.setBold(true);
        painter.setFont(legendFont);
        painter.setPen(foreground);

        //Second row (caption)
        painter.drawText(QRect(curX, 0, columnWidths[1], rowHeight),
                         Qt::AlignVCenter | Qt::AlignLeft,
                         model()->headerData(m_legendCol, Qt::Horizontal).toString());

        curX += colSep + columnWidths[1];

        painter.drawText(QRect(curX, 0, columnWidths[2], rowHeight),
                         Qt::AlignVCenter | Qt::AlignRight,
                         tr("Allocation (%)"));

        curX += colSep + columnWidths[2];

        painter.drawText(QRect(curX, 0, columnWidths[3], rowHeight),
                         Qt::AlignVCenter | Qt::AlignRight,
                         model()->headerData(m_dataCol, Qt::Horizontal).toString());

        painter.setPen(lineColor);
        painter.drawLine(rowLine);

        //Draw the rows themselves
        legendFont.setBold(false);
        painter.setFont(legendFont);

        for (int i = 0; i < m_items.count(); ++i)
        {
            const ItemTuple& item = m_items[i];

            curX = colSep;

            //First column (color)
            painter.setPen(noPen);
            painter.setBrush(QColor(CHART_COLORS[i % CHART_COLORS.size()]));
            painter.drawRoundedRect(QRect(curX, rowLine.y1()+(rowHeight-colorSize)/2, colorSize, colorSize), 2.0, 2.0);

            curX += colSep + columnWidths[0];

            //Second column (caption)
            QString caption;
            if (lastIsOther && i == m_items.count()-1)
            {
                caption = std::get<0>(item);
            }
            else
            {
                caption = model()->data(model()->index(std::get<2>(item), m_legendCol)).toString();
            }

            painter.setPen(foreground);
            painter.drawText(QRect(curX, rowLine.y1(), columnWidths[1]+columnWidths[2], rowHeight),
                             Qt::AlignVCenter | Qt::AlignLeft,
                             caption);

            curX += colSep + columnWidths[1];

            //Third column (%)
            painter.drawText(QRect(curX, rowLine.y1(), columnWidths[2], rowHeight),
                             Qt::AlignVCenter | Qt::AlignRight,
                             locale.toString(std::get<1>(item)*100.0, 'f', 1));

            curX += colSep + columnWidths[2];

            //Fourth column (value)
            painter.drawText(QRect(curX, rowLine.y1(), columnWidths[2], rowHeight),
                             Qt::AlignVCenter | Qt::AlignRight,
                             locale.toString(std::get<3>(item), 'f', 2));

            rowLine.translate(0, rowHeight);
            painter.setPen(linePen);
            painter.drawLine(rowLine);

        }

        //Draw totals
        if (totalsShown())
        {
            legendFont.setBold(true);
            painter.setFont(legendFont);
            painter.setPen(foreground);

            curX = 3*colSep + columnWidths[0] + columnWidths[1];

            //Percent
            painter.drawText(QRect(curX, rowLine.y1(), columnWidths[2], rowHeight),
                             Qt::AlignVCenter | Qt::AlignRight,
                             "100.0");

            curX += colSep + columnWidths[2];

            painter.drawText(QRect(curX, rowLine.y1(), columnWidths[2], rowHeight),
                             Qt::AlignVCenter | Qt::AlignRight,
                             locale.toString(totalValue, 'f', 2));

        }

        painter.restore();
    }
}

void PercChart::resizeEvent(QResizeEvent*)
{
    updateGeometries();
}

int PercChart::rows(const QModelIndex &index) const
{
    return model()->rowCount(model()->parent(index));
}

void PercChart::rowsInserted(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row <= end; ++row) {

        QModelIndex index = model()->index(row, m_dataCol, rootIndex());
        double value = model()->data(index, m_dataRole).toDouble();

        if (value > 0.0) {
            totalValue += value;
            validItems++;
        }
    }

    updateItems();

    QAbstractItemView::rowsInserted(parent, start, end);
}

void PercChart::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    for (int row = start; row <= end; ++row) {

        QModelIndex index = model()->index(row, m_dataCol, rootIndex());
        double value = model()->data(index, m_dataRole).toDouble();
        if (value > 0.0) {
            totalValue -= value;
            validItems--;
        }
    }

    updateItems();

    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

void PercChart::scrollContentsBy(int dx, int dy)
{
    viewport()->scroll(dx, dy);
}

void PercChart::scrollTo(const QModelIndex &index, ScrollHint)
{
    QRect area = viewport()->rect();
    QRect rect = visualRect(index);

    if (rect.left() < area.left())
        horizontalScrollBar()->setValue(
                    horizontalScrollBar()->value() + rect.left() - area.left());
    else if (rect.right() > area.right())
        horizontalScrollBar()->setValue(
                    horizontalScrollBar()->value() + qMin(
                        rect.right() - area.right(), rect.left() - area.left()));

    if (rect.top() < area.top())
        verticalScrollBar()->setValue(
                    verticalScrollBar()->value() + rect.top() - area.top());
    else if (rect.bottom() > area.bottom())
        verticalScrollBar()->setValue(
                    verticalScrollBar()->value() + qMin(
                        rect.bottom() - area.bottom(), rect.top() - area.top()));

    update();
}

/*
     Find the indices corresponding to the extent of the selection.
 */

//void Chart::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
//{
//    // Use content widget coordinates because we will use the itemRegion()
//    // function to check for intersections.

//    QRect contentsRect = rect.translated(
//                             horizontalScrollBar()->value(),
//                             verticalScrollBar()->value()).normalized();

//    int rows = model()->rowCount(rootIndex());
//    int columns = model()->columnCount(rootIndex());
//    QModelIndexList indexes;

//    for (int row = 0; row < rows; ++row) {
//        for (int column = 0; column < columns; ++column) {
//            QModelIndex index = model()->index(row, column, rootIndex());
//            QRegion region = itemRegion(index);
//            if (region.intersects(contentsRect))
//                indexes.append(index);
//        }
//    }

//    if (indexes.size() > 0) {
//        int firstRow = indexes[0].row();
//        int lastRow = indexes[0].row();
//        int firstColumn = indexes[0].column();
//        int lastColumn = indexes[0].column();

//        for (int i = 1; i < indexes.size(); ++i) {
//            firstRow = qMin(firstRow, indexes[i].row());
//            lastRow = qMax(lastRow, indexes[i].row());
//            firstColumn = qMin(firstColumn, indexes[i].column());
//            lastColumn = qMax(lastColumn, indexes[i].column());
//        }

//        QItemSelection selection(
//                    model()->index(firstRow, firstColumn, rootIndex()),
//                    model()->index(lastRow, lastColumn, rootIndex()));
//        selectionModel()->select(selection, command);
//    } else {
//        QModelIndex noIndex;
//        QItemSelection selection(noIndex, noIndex);
//        selectionModel()->select(selection, command);
//    }

//    update();
//}

void PercChart::updateGeometries()
{
    horizontalScrollBar()->setPageStep(viewport()->width());
    horizontalScrollBar()->setRange(0, qMax(0, 2*totalSizeH - viewport()->width()));
    verticalScrollBar()->setPageStep(viewport()->height());
    verticalScrollBar()->setRange(0, qMax(0, totalSizeV - viewport()->height()));
}

int PercChart::verticalOffset() const
{
    return verticalScrollBar()->value();
}

/*
     Returns the position of the item in viewport coordinates.
 */

QRect PercChart::visualRect(const QModelIndex &index) const
{
    QRect rect = itemRect(index);
    if (rect.isValid())
        return QRect(rect.left() - horizontalScrollBar()->value(),
                     rect.top() - verticalScrollBar()->value(),
                     rect.width(), rect.height());
    else
        return rect;
}

/*
     Returns a region corresponding to the selection in viewport coordinates.
 */

QRegion PercChart::visualRegionForSelection(const QItemSelection &selection) const
{
    int ranges = selection.count();

    if (ranges == 0)
        return QRect();

    QRegion region;
    for (int i = 0; i < ranges; ++i) {
        QItemSelectionRange range = selection.at(i);
        for (int row = range.top(); row <= range.bottom(); ++row) {
            for (int col = range.left(); col <= range.right(); ++col) {
                QModelIndex index = model()->index(row, col, rootIndex());
                region += visualRect(index);
            }
        }
    }
    return region;
}

} //Namespace

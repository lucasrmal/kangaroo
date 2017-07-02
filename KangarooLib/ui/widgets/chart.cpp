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

#include "chart.h"

#include <QScrollBar>
#include <QPainter>
#include <QAbstractItemModel>
#include <QFontMetrics>
#include <math.h>

namespace KLib
{

    Chart::Chart(QWidget* _parent) :
        QScrollArea(_parent),
        m_model(NULL),
        m_view(new ChartView(this)),
        m_maxValue(0),
        m_minValue(0),
        m_averageValue(0),
        m_keyVisible(false),
        m_grid(GridFlag::H),
        m_showLabels(true),
        m_showAverage(false),
        m_type(ChartType::VBar)
    {
    }

    void Chart::setModel(QAbstractItemModel* _model, Qt::Orientation _orientation, int _section)
    {
        m_model = _model;
        m_orientation = _orientation;
        m_section = _section;

        connect(m_model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(onDataModified()));
        connect(m_model, SIGNAL(rowsInserted(QModelIndex,int,int)),
                this, SLOT(onColumnRowCountChanged()));
        connect(m_model, SIGNAL(rowsRemoved(QModelIndex,int,int)),
                this, SLOT(onColumnRowCountChanged()));
        connect(m_model, SIGNAL(columnsInserted(QModelIndex,int,int)),
                this, SLOT(onColumnRowCountChanged()));
        connect(m_model, SIGNAL(columnsRemoved(QModelIndex,int,int)),
                this, SLOT(onColumnRowCountChanged()));

        updateIntervals();
        m_view->update();
    }

    void Chart::onDataModified()
    {
        updateIntervals();
        m_view->update();
    }

    void Chart::onColumnRowCountChanged()
    {
        updateIntervals();
        m_view->update();
    }

    void Chart::updateIntervals()
    {
        //Min and max
        m_maxValue = -qInf();
        m_minValue = qInf();
        m_averageValue = 0;

        if (orientation() == Qt::Horizontal) //Columns
        {
            for (int i = 0; i < model()->columnCount(); ++i)
            {
                double val = model()->data(model()->index(section(), i), Qt::EditRole).toDouble();
                m_averageValue += val;

                if (val > m_maxValue)
                {
                    m_maxValue = val;
                }
                else if (val < m_minValue)
                {
                    m_minValue = val;
                }
            }

            m_averageValue /= (double) model()->columnCount();
        }
        else // (orientation() == Qt::Vertical) //Rows
        {
            for (int i = 0; i < model()->rowCount(); ++i)
            {
                double val = model()->data(model()->index(i, section()), Qt::EditRole).toDouble();
                m_averageValue += val;

                if (val > m_maxValue)
                {
                    m_maxValue = val;
                }
                else if (val < m_minValue)
                {
                    m_minValue = val;
                }
            }

            m_averageValue /= (double) model()->rowCount();
        }

        //Ceiling and interval
        QString str = QString::number((int) floor(m_maxValue));
        m_ceiling = (str.left(1).toInt() + 1);
        m_ceiling *= pow(10, str.length()-1); //ceiling is at least 1

        int twoFirst = QString::number(m_ceiling).left(2).toInt();

        while (twoFirst < 10)
            twoFirst *= 10; //twoFirst will be between 10 and 99

        m_intervalCount = 9;

        while (twoFirst % m_intervalCount != 0)
        {
            --m_intervalCount;
        }

        //ceiling is divisible by start, use that as the number of intervals.
        m_intervalSize = ((double)m_ceiling) / (double)m_intervalCount;

        if (m_minValue < 0)
        {
            m_floor = floor(m_minValue / m_intervalSize) * m_intervalSize;
            m_intervalCount += (int) round(abs(m_floor) / m_intervalSize);
        }
        else
        {
            m_floor = 0;
        }
    }


    //////////////////////////////////////////////////////


    ChartView::ChartView(Chart* _parent) :
        QWidget(_parent),
        m_chart(_parent)
    {

    }

    quint32 pixelsForPointSize(const quint32 p_pointSize, const QPaintDevice* p_device)
    {
        return (((double) p_pointSize) / 72.0) * (double) p_device->logicalDpiX();
    }

    QSize ChartView::minimumSizeHint() const
    {
        return QSize(300, 400);
    }

    QSize ChartView::sizeHint() const
    {
        return m_chart->viewport()->size();
    }

    void ChartView::paintEvent(QPaintEvent *event)
    {
        Q_UNUSED(event)

        if (!m_chart->model()) return;

        QPainter painter(this);
        painter.setRenderHint(QPainter::TextAntialiasing, true);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setPen(Qt::SolidLine);

        //Initialisations
        const QRect deviceRect = rect();
        const int deviceWidth = width();
        const int deviceHeight = height();
        const QColor colorBorder = Qt::black;
        const QColor colorGrid = QColor(200, 200, 200);
        const QColor colorAverage = Qt::red;

        const int widthBorder = 2;
        const int widthGrid = 2;
        const int widthAverage = 2;
        const int widthLine = 2;
        const int widthPoint = 4;

        const QRect internalRect = QRect(MARGIN_SIZE, MARGIN_SIZE, deviceWidth - (2 * MARGIN_SIZE), deviceHeight - (2 * MARGIN_SIZE));
        QFont currentFont = painter.font();

        quint32 distTop = 0;
        quint32 distRight = 0;
        quint32 distBottom = 0;
        quint32 distLeft = 0;

        QStringList labels;
        QList<double> values;
        QList<QColor> colors;

        //Fill the values and colors
        if (m_chart->orientation() == Qt::Horizontal) //Columns
        {
            for (int i = 0; i < m_chart->model()->columnCount(); ++i)
            {
                labels << m_chart->model()->headerData(i, Qt::Horizontal).toString();
                values << m_chart->model()->data(m_chart->model()->index(m_chart->section(), i), Qt::EditRole).toDouble();
                colors << m_chart->defaultColor();
            }
        }
        else // (m_chart->orientation() == Qt::Vertical) //Rows
        {
            for (int i = 0; i < m_chart->model()->rowCount(); ++i)
            {
                labels << m_chart->model()->headerData(i, Qt::Vertical).toString();
                values << m_chart->model()->data(m_chart->model()->index(i, m_chart->section()), Qt::EditRole).toDouble();
                colors << m_chart->defaultColor();
            }
        }

        //---------------------Background fill----------------------
        painter.fillRect(deviceRect, m_chart->palette().color(QPalette::Base));

        if (labels.count() == 0)
            return;

        //-------------------------Legend---------------------------
        if (m_chart->keyVisible())
        {
            //Find the largest width of the Y labels
            quint32 maxFontWidth = 0;
            currentFont.setPointSize(FONT_SIZE);
            currentFont.setBold(false);
            QFontMetrics metrics(currentFont, this);

            for (int i = 0; i < labels.count(); ++i)
            {
                quint32 newSize = metrics.width(labels[i]);

                if (newSize > maxFontWidth)
                    maxFontWidth = newSize;
            }

            if (maxFontWidth > 200)
                maxFontWidth = 200;

            //Const Values
            const quint32 MARGIN_BETWEEN_ROWS = 3;
            const quint32 FONT_HEIGHT = metrics.height() + 2;
            const quint32 FONT_WIDTH = maxFontWidth + 2;
            const quint32 MARGIN_RIGHT_LEGEND = 15;

            const quint32 LEGEND_HEIGHT = ((FONT_HEIGHT + MARGIN_BETWEEN_ROWS) * labels.count()) + MARGIN_BETWEEN_ROWS;
            const quint32 LEGEND_WIDTH = (MARGIN_BETWEEN_ROWS * 3) + FONT_WIDTH + FONT_HEIGHT;

            painter.setFont(currentFont);

            //Draw the rect
            int xPos = ((internalRect.height() - distTop) - LEGEND_HEIGHT) / 2;
            QRect legendRect(internalRect.right() - (LEGEND_WIDTH + MARGIN_RIGHT_LEGEND), internalRect.top() + distTop + xPos, LEGEND_WIDTH, LEGEND_HEIGHT);

            painter.setPen(QPen(QBrush(Qt::black), 1));

            painter.drawRect(legendRect);

            //Draw the rows

            QRect squareRect(legendRect.left() + MARGIN_BETWEEN_ROWS, legendRect.top() + MARGIN_BETWEEN_ROWS, FONT_HEIGHT, FONT_HEIGHT);
            QRect labelRect(legendRect.left() + ( 2 * MARGIN_BETWEEN_ROWS) + FONT_HEIGHT, legendRect.top() + MARGIN_BETWEEN_ROWS, FONT_WIDTH, FONT_HEIGHT);

            for (int i = 0; i < labels.count(); ++i)
            {
                painter.fillRect(squareRect, colors[i]);
                painter.drawRect(squareRect);

                painter.drawText(labelRect, Qt::AlignLeft | Qt::AlignTop, labels[i]);

                squareRect.translate(0, FONT_HEIGHT + MARGIN_BETWEEN_ROWS);
                labelRect.translate(0, FONT_HEIGHT + MARGIN_BETWEEN_ROWS);
            }

            distRight += MARGIN_RIGHT_LEGEND + LEGEND_WIDTH + 2;

        }

        QRect diagramRect(QPoint(internalRect.left() + distLeft, internalRect.top() + distTop), QPoint(internalRect.right() - distRight, internalRect.bottom() - distBottom));

        if (m_chart->chartType() != ChartType::Pie)
        {
            currentFont.setPointSize(FONT_SIZE);
            currentFont.setBold(false);
            QFontMetrics metrics(currentFont, this);

            const quint32 SPACE_TOP = 4;
            const quint32 SPACE_LEFT = metrics.width(QString::number(m_chart->ceilVal())) + 40;
            const quint32 SPACE_BOTTOM = 30;
            const quint32 SPACE_RIGHT = 8;

            const quint32 HLENGHT_X_LINES = 4;
            const quint32 VLENGHT_Y_LINES = 4;

            QRect mainDiagram(QPoint(diagramRect.left() + SPACE_LEFT, diagramRect.top() + SPACE_TOP), QPoint(diagramRect.right() - SPACE_RIGHT, diagramRect.bottom() - SPACE_BOTTOM));

            //Draw diagram borders
            painter.setPen(QPen(QBrush(colorGrid), widthGrid));
            painter.drawLine(mainDiagram.topLeft(), mainDiagram.bottomLeft());
            painter.drawLine(QPoint(mainDiagram.left() - HLENGHT_X_LINES, mainDiagram.bottom()), mainDiagram.bottomRight());
            painter.drawLine(mainDiagram.topRight(), QPoint(mainDiagram.right(), mainDiagram.bottom() + VLENGHT_Y_LINES));

            quint32 distBetweenYValues = mainDiagram.height() / m_chart->intervalCount();
            quint32 distBetweenXValues = mainDiagram.width() / values.count();

            //Lines & Labels
            //X Lines
            QLine xline(mainDiagram.left() - HLENGHT_X_LINES, mainDiagram.top(), mainDiagram.left() + HLENGHT_X_LINES,  mainDiagram.top());

            if (m_chart->grid() & GridFlag::H)
            {
                xline.setP2(QPoint(mainDiagram.right(), mainDiagram.top()));
            }

            for (int i = 0; i < m_chart->intervalCount(); ++i)
            {
                painter.drawLine(xline);
                xline.translate(0, distBetweenYValues);
            }

            //Y Lines
            QLine yline(mainDiagram.left(), mainDiagram.bottom(), mainDiagram.left(),  mainDiagram.bottom() + VLENGHT_Y_LINES);

            if (m_chart->grid() & GridFlag::V)
            {
                yline.setP1(QPoint(mainDiagram.left(), mainDiagram.top()));
            }

            for (int i = 0; i < values.count(); ++i)
            {
                painter.drawLine(yline);
                yline.translate(distBetweenXValues, 0);
            }

            int heightLabel = pixelsForPointSize(FONT_SIZE, this);
            currentFont.setPointSize(FONT_SIZE);
            currentFont.setBold(false);
            painter.setFont(currentFont);
            painter.setPen(m_chart->palette().color(QPalette::Foreground));

            //X Labels
            QRect rectXLabel(QPoint(diagramRect.left(), mainDiagram.top() - (heightLabel / 2)), QPoint(mainDiagram.left() - (HLENGHT_X_LINES + 5), mainDiagram.top() + 1 + (heightLabel / 2)));

            for (int i = m_chart->intervalCount(); i >= 0; i--)
            {
                painter.drawText(rectXLabel,
                                 Qt::AlignRight,
                                 QString::number(m_chart->intervalSize() * ((double) i) + m_chart->floorVal(), 'f', 2));

                //Chart->floor is negative or zero
                rectXLabel.translate(0, distBetweenYValues);
            }

            //Y Labels
            QRect rectYLabel(QPoint(mainDiagram.left(), mainDiagram.bottom() + VLENGHT_Y_LINES), QPoint(mainDiagram.left() + distBetweenXValues, diagramRect.bottom()));

            for (int i = 0; i < values.count(); ++i)
            {
                painter.drawText(rectYLabel, Qt::AlignCenter, labels[i]);
                rectYLabel.translate(distBetweenXValues, 0);
            }

            currentFont.setPointSize(FONT_SIZE);
            currentFont.setBold(false);
            painter.setFont(currentFont);

            double heightZero = m_chart->floorVal() == 0 ? 0
                                                         : abs((m_chart->floorVal() * mainDiagram.height())
                                                               / (m_chart->intervalCount() * m_chart->intervalSize()));

            double heightPositive = mainDiagram.height() - heightZero;

            switch (m_chart->chartType())
            {
            case ChartType::VBar:
            {
                //Data columns
                QRect rectData(QPoint(mainDiagram.left() + (distBetweenXValues / 4),
                                      mainDiagram.bottom() - 200),
                               QPoint(mainDiagram.left() + (3 * (distBetweenXValues / 4)),
                                      mainDiagram.bottom() - 2));

                for (int i = 0; i < values.count(); ++i)
                {
                    int rowHeight = ((values[i] / m_chart->ceilVal()) * heightPositive) - 1;

                    //rectData.setTop(mainDiagram.bottom() + heightZero - rowHeight);

                    if (values[i] > 0)
                    {
                        rectData.setTop(mainDiagram.bottom() - heightZero - rowHeight);
                        rectData.setBottom(mainDiagram.bottom() - heightZero);
                    }
                    else if (values[i] < 0)
                    {
                        rectData.setTop(mainDiagram.bottom() - heightZero);
                        rectData.setBottom(mainDiagram.bottom() - heightZero - rowHeight); //row height will be neg
                    }
                    else
                    {
                        rectData.translate(distBetweenXValues, 0);
                        continue;
                    }

                    painter.setPen(QPen(QBrush(colorBorder), widthBorder));

                    painter.fillRect(rectData, colors[i]);
                    painter.drawRect(rectData);

                    //Data Labels
                    if (m_chart->showLabels())
                    {
                        QRect dataLabelRect(QPoint(rectData.left(), rectData.top() - (heightLabel + 5)), QSize(rectData.width(), heightLabel + 5));

                        painter.setPen(m_chart->palette().color(QPalette::Foreground));
                        painter.drawText(dataLabelRect, Qt::AlignCenter, QString::number(values[i], 'f', 2));
                    }

                    rectData.translate(distBetweenXValues, 0);
                }
                break;
            }
            case ChartType::Line:
            case ChartType::Area:
            case ChartType::Dots:
            {
                QPoint currentPoint(mainDiagram.left() + (distBetweenXValues / 2),
                                    0);

                QVector<QPoint> points(values.count());

                for (int i = 0; i < values.count(); ++i)
                {
                    int rowHeight = ((values[i] / m_chart->ceilVal()) * heightPositive) - 1;

                    currentPoint.setY(mainDiagram.bottom() - heightZero - rowHeight);
                    points[i] = currentPoint;

                    //Data Labels
                    if (m_chart->showLabels())
                    {
                        int distFromPoint = values[i] > 0 ? -heightLabel - 5
                                                          :  heightLabel + 5;
                        QRect dataLabelRect(QPoint(currentPoint.x() - distBetweenXValues/2,
                                                   currentPoint.y() + distFromPoint),
                                            QSize(distBetweenXValues, heightLabel + 5));

                        painter.setPen(m_chart->palette().color(QPalette::Foreground));
                        painter.drawText(dataLabelRect, Qt::AlignCenter, QString::number(values[i], 'f', 2));
                    }

                    currentPoint.setX(currentPoint.x() + distBetweenXValues);
                }

                switch (m_chart->chartType())
                {
                case ChartType::Area:
                {
                    QPolygon polygon(points);
                    polygon << QPoint(currentPoint.x(),
                                      mainDiagram.bottom())
                            << QPoint(mainDiagram.left() + (distBetweenXValues / 2),
                                      mainDiagram.bottom());
                    painter.setBrush(QBrush(m_chart->defaultColor(), Qt::SolidPattern));
                    painter.drawPolygon(polygon);

                    painter.setPen(QPen(QBrush(colorBorder), widthBorder));
                    painter.drawPolyline(QPolygon(points));
                    break;
                }
                case ChartType::Line:
                    painter.setPen(QPen(QBrush(m_chart->defaultColor()), widthLine));
                    painter.drawPolyline(QPolygon(points));
                    break;

                case ChartType::Dots:
                {
                    painter.setPen(QPen(QBrush(m_chart->defaultColor()), widthPoint));
                    painter.drawPoints(QPolygon(points));
                    break;
                }
                default:
                    break;
                }
            }
            default:
                break;
            } // End switch


            //Average
            if (m_chart->showAverage())
            {
                painter.setPen(QPen(QBrush(colorAverage), widthAverage));

                quint32 rowHeight = (m_chart->averageValue() / m_chart->ceilVal()) * mainDiagram.height();

                QLine averageLine(QPoint(mainDiagram.left(), mainDiagram.bottom() - rowHeight),
                                  QPoint(mainDiagram.right(), mainDiagram.bottom() - rowHeight));

                painter.drawLine(averageLine);
            }
        }



    }

}

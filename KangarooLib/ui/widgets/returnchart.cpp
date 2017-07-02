#include "returnchart.h"

#include <QRect>
#include <QSize>
#include <QPoint>
#include <QPen>
#include <QPainter>
#include <QPaintEvent>
#include <QFontMetrics>
#include <cmath>

namespace KLib
{

    const QStringList ReturnChart::CHART_COLORS = {"#59B5CA", "#179443", "#71B62F", "#FABF00", "#E98248",
                                                   "#D0444D", "#B92049", "#B156B8", "#7066D2", "#1358A7"};

    ReturnChart::ReturnChart(ReturnChartStyle _style, ValueType _valueType, Qt::ItemDataRole _dataRole, QWidget* _parent) :
        QAbstractItemView(_parent),
        m_style(_style),
        m_valueType(_valueType),
        m_role(_dataRole),
        currentHoverRow(-1),
        m_legendVisible(true),
        m_averageVisible(false)
    {
        horizontalScrollBar()->setRange(0, 0);
        verticalScrollBar()->setRange(0, 0);

        lineColor = QColor("#E6E6E6");

        m_yAxisIndicator = QPair<QString, bool>(_valueType == ValueType::Percent ? "%" : "", false);

        captionFont.setPointSize(7);
        legendFont.setPointSize(9);
        titleFont.setBold(true);
        titleFont.setPointSize(12);

        QFontMetrics titleMetrics(legendFont);
        QFontMetrics legendMetrics(legendFont);

        titleHeight = titleMetrics.height()+10;

        minChartHeight = 300;
        maxChartHeight = 450;

        minLevelHeight = 40;
        maxLevelHeight = 80;

        legendWidth = 300;
        legendRowH = legendMetrics.height()*2+5;

        marginH = 70;
        marginV = 40;

        minBarWidth = 15;
        inBarSpacing = 10;
        outBarSpacing = 15;

        minPointSpacing = 10;


        setMouseTracking(true);
    }

    double ReturnChart::valueAt(int _row, int _col) const
    {
        double value = model()->data(model()->index(_row, _col), m_role).toDouble();

        if (m_valueType == ValueType::Percent)
        {
            value *= 100.0;
        }

        return value;
    }

    QSize ReturnChart::getChartSize() const
    {
        int chartWidth = 0, chartHeight = 0;

        switch (m_style)
        {
        case ReturnChartStyle::Line:
            chartWidth = (model()->rowCount()*2)*minPointSpacing;
            break;

        case ReturnChartStyle::VerticalBar:

            chartWidth = (model()->columnCount()*minBarWidth + (model()->columnCount()-1)*inBarSpacing)*model()->rowCount()
                         + outBarSpacing*model()->rowCount()*2;
            break;
        }

        chartWidth = std::max(viewport()->width()-marginH*2, chartWidth);

        if (m_legendVisible)
        {
            chartHeight = std::max(viewport()->height()-marginV*3
                                   - legendRowH*model()->columnCount()
                                   - (m_title.isEmpty() ? 0 : titleHeight),
                                   minChartHeight);
        }
        else
        {
            chartHeight = std::max(viewport()->height()
                                   -marginV*2
                                   - (m_title.isEmpty() ? 0 : titleHeight),
                                   minChartHeight);
        }

        chartHeight = std::min(chartHeight, maxChartHeight);

        return QSize(chartWidth, chartHeight);
    }

    void ReturnChart::setLegendVisible(bool _visible)
    {
        m_legendVisible = _visible;
        update();
    }

    void ReturnChart::setAverageVisible(bool _visible)
    {
        m_averageVisible = _visible;
        update();
    }

    void ReturnChart::setTitle(const QString& _title)
    {
        bool updateGeo =     (_title.isEmpty() && !m_title.isEmpty())
                          || (!_title.isEmpty() && m_title.isEmpty());

        m_title = _title;

        if (updateGeo)
        {
            updateGeometries();
        }

        update();
    }

    void ReturnChart::setYAxisIndicator(const QString& _indicator, bool _before)
    {
        m_yAxisIndicator = QPair<QString, bool>(_indicator, _before);
        update();
    }

    void ReturnChart::paintEvent(QPaintEvent *event)
    {
        QStyleOptionViewItem option = viewOptions();

        QBrush background = option.palette.base();
        QPen foreground(option.palette.color(QPalette::WindowText));

        QPen noPen;
        noPen.setStyle(Qt::NoPen);

        QPen mainLinePen;
        mainLinePen.setColor(Qt::black);
        mainLinePen.setWidth(1);

        QPen linePen;
        linePen.setColor(lineColor);
        linePen.setWidth(1);

        QFont captionFont;
        captionFont.setPointSize(7);
        QFontMetrics captionMetrics(captionFont);

        QFont barPercFont;
        barPercFont.setBold(true);
        barPercFont.setPointSize(10);

        QLocale locale;

        QPainter painter(viewport());
        painter.setRenderHint(QPainter::Antialiasing);

        painter.fillRect(event->rect(), background);

        if (!model()->rowCount() || !model()->columnCount())
            return;

        QSize chartSize = getChartSize();

        //============================================ DRAW THE TITLE ============================================

        if (!m_title.isEmpty())
        {
            QRect titleRect = QRect(marginH, marginV/2, chartSize.width(), titleHeight);
            painter.setFont(titleFont);
            painter.setPen(foreground);
            painter.drawText(titleRect, Qt::AlignCenter, m_title);
        }


        //============================================ DRAW THE CHART ============================================

        painter.save();

        //Go to topleft point of chart

        QRect chartRect = QRect(QPoint(marginH, marginV + (m_title.isEmpty() ? 0 : titleHeight)),
                                chartSize);

        painter.translate(chartRect.x() - horizontalScrollBar()->value(),
                          chartRect.y() - verticalScrollBar()->value());

        //Draw the chart borders
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setPen(mainLinePen);
        painter.drawLine(QPoint(0,0), QPoint(0, chartSize.height()));
        painter.drawLine(QPoint(0, chartSize.height()), QPoint(chartSize.width(), chartSize.height()));
        painter.setRenderHint(QPainter::Antialiasing);

        //Draw the y axis labels and h-lines

        //Find the min and max values
        double min = 0, max = 0;

        for (int row = 0; row < model()->rowCount(); ++row)
        {
            for (int col = 0; col < model()->columnCount(); ++col)
            {
                double value = valueAt(row, col);

                max = std::max(value, max);
                min = std::min(value, min);
            }
        }

        //Compute the number of levels
        double interval = pow(10.0, std::max(floor(log10(std::abs(max))),   //interval is ALWAYS positive
                                             floor(log10(std::abs(min)))));

        int numLevelsUp   = max > 0 ? std::ceil(std::abs(max)/interval) : 0.0;
        int numLevelsDown = min < 0 ? std::ceil(std::abs(min)/interval) : 0.0;

        if (numLevelsUp == 0 && numLevelsDown == 0)
        {
            numLevelsUp = 1;
            interval = 1;
        }

        int numLevels = numLevelsUp + numLevelsDown;
        int levelHeight = chartSize.height() / numLevels;

        while (levelHeight < minLevelHeight)
        {
            numLevelsUp = ceil(numLevelsUp / 2.0);
            numLevelsDown = ceil(numLevelsDown / 2.0);

            interval *= 2.0;

            numLevels = numLevelsUp + numLevelsDown;
            levelHeight = chartSize.height() / numLevels;
        }

        while (levelHeight > maxLevelHeight)
        {
            numLevelsUp *= 2.0;
            numLevelsDown *= 2.0;

            interval = ceil(interval / 2.0);

            numLevels = numLevelsUp + numLevelsDown;
            levelHeight = chartSize.height() / numLevels;
        }

        int captionPrec = -std::min(int(floor(log10(interval))), 0);

        //Draw lines
        int curY = chartSize.height();
        int zeroHeight = curY-numLevelsDown*levelHeight;
        double curLevel = -numLevelsDown*interval;

        painter.setFont(captionFont);

        QString yIndicator = (m_yAxisIndicator.second ? QString("%1 %2")
                                                      : QString("%2 %1")).arg(m_yAxisIndicator.first);

        for (int l = 0; l <= numLevels; ++l)
        {
            //Line
            painter.setPen(curY == zeroHeight ? mainLinePen : linePen);
            painter.setRenderHint(QPainter::Antialiasing, false);
            painter.drawLine(1, curY, chartSize.width()-1, curY);
            painter.setRenderHint(QPainter::Antialiasing, true);

            //Caption
            QString caption = locale.toString(curLevel, 'f', captionPrec);
            painter.setPen(foreground);
            painter.drawText(QRect(-marginH, curY-20, marginH-5, 40),
                             Qt::AlignVCenter | Qt::AlignRight,
                             l == numLevels ? yIndicator.arg(caption)
                                            : caption);

            curY -= levelHeight;
            curLevel += interval;
        }

        QVector<double> average(model()->columnCount(), 0);

        auto drawHorizontalLine = [&painter, &chartSize, &zeroHeight] (int x)
        {
            painter.drawLine(QPoint(x, 0), QPoint(x, zeroHeight-1));
            painter.drawLine(QPoint(x, zeroHeight+1), QPoint(x, chartSize.height()));
        };


        switch (m_style)
        {
        case ReturnChartStyle::VerticalBar:
        {
            int curX = outBarSpacing;

            int rowWidth = (chartSize.width()
                            - outBarSpacing*model()->rowCount()*2
                            - model()->rowCount()*(model()->columnCount()-1)*inBarSpacing)
                           / (model()->rowCount()*model()->columnCount());

            QRect bottomCaption(/* x   */ 0,
                                /* y   */ chartSize.height(),
                                /* wid */ outBarSpacing*2
                                          + model()->columnCount()*rowWidth
                                          + (model()->columnCount()-1)*inBarSpacing,
                                /* hei */ marginV);

            for (int row = 0; row < model()->rowCount(); ++row)
            {

                for (int col = 0; col < model()->columnCount(); ++col)
                {
                    double value = valueAt(row, col);
                    double height = (double(levelHeight) / interval) * value;

                    average[col] += value;

                    //Bar
                    painter.setPen(noPen);
                    painter.setBrush(QColor(CHART_COLORS[col % CHART_COLORS.size()]));

                    painter.drawRoundedRect(QRect(curX,     height > 0 ? zeroHeight-height+1 : zeroHeight+1,
                                                  rowWidth, height > 0 ? height : -height), 2.0, 2.0);

                    //Caption
                    painter.setPen(foreground);
                    QString caption = locale.toString(std::abs(value), 'f', captionPrec+1);

                    if (value < 0)
                        caption = "(" + caption + ")";

                    painter.drawText(QRect(curX-(inBarSpacing/2), height >= 0 ? zeroHeight-height-35 : zeroHeight-height+5,
                                           rowWidth+inBarSpacing, 30),
                                     Qt::AlignHCenter | (height >= 0 ? Qt::AlignBottom : Qt::AlignTop),
                                     caption);

                    curX += rowWidth;

                    if (col != model()->columnCount()-1)
                    {
                        curX += inBarSpacing;
                    }
                }

                //Caption
                painter.setPen(foreground);
                painter.drawText(bottomCaption.translated(0, 10),
                                 Qt::AlignHCenter | Qt::AlignTop,
                                 model()->headerData(row, Qt::Vertical).toString());

                //Lines
                painter.setPen(linePen);

                //Center line
                painter.setRenderHint(QPainter::Antialiasing, false);
                painter.drawLine(QPoint(bottomCaption.center().x(), chartSize.height()),
                                 QPoint(bottomCaption.center().x(), chartSize.height()+6));

                //Vertical line
                if (row != model()->rowCount()-1)
                {
                    drawHorizontalLine(bottomCaption.right());
                    //painter.drawLine(bottomCaption.topRight(), QPoint(bottomCaption.right(), 0));
                    bottomCaption.translate(bottomCaption.width(), 0);
                }

                painter.setRenderHint(QPainter::Antialiasing, true);

                curX += 2*outBarSpacing;
            }


            break;
        }
        case ReturnChartStyle::Line:
        {
            int pointSep = chartSize.width() / (model()->rowCount()*2);
            int rowWidth = pointSep*2;

            int curX = pointSep;

            QRect bottomCaption(/* x   */ 0,
                                /* y   */ chartSize.height(),
                                /* wid */ rowWidth,
                                /* hei */ marginV);

            QPen pointPen;
            pointPen.setWidth(6);
            pointPen.setCapStyle(Qt::RoundCap);

            QPen chartLinePen;
            chartLinePen.setWidth(2);
            chartLinePen.setStyle(Qt::DashLine);

            QVector<QPoint> previous(model()->columnCount());

            int pointLegendHeight = 1.5*captionMetrics.height();
            int sizeColorLabel = 6;
            int minDistBetweenLabels = 3;

            auto heightFor = [zeroHeight, levelHeight, interval] (double value) {
                return zeroHeight- ((double(levelHeight) / interval) * value);
            };

            for (int row = 0; row < model()->rowCount(); ++row)
            {
                QHash<int, int> labelLocation; //Col, x-top

                if (row == currentHoverRow)
                {
                    pointPen.setWidth(12);

                    //Find location of labels

                    QVector<QPair<double, int> > labels(model()->columnCount()); //Value, col

                    for (int col = 0; col < model()->columnCount(); ++col)
                    {
                        labels[col] = QPair<double, int>(valueAt(row, col), col);
                    }

                    //Sort by value
                    std::sort(labels.begin(), labels.end()); //Smallest first

                    auto preferredLocation = [&heightFor, pointLegendHeight] (double value)
                    {
                        return heightFor(value)-(pointLegendHeight/2);
                    };

                    int middle = labelLocation.count()/2;

                    labelLocation[labels[middle].second] = preferredLocation(labels[middle].first);

                    //Down first
                    for (int i = middle-1; i >= 0; --i)
                    {
                        int curX = std::max(preferredLocation(labels[i].first),
                                            preferredLocation(labels[i+1].first)+pointLegendHeight+minDistBetweenLabels);

                        labelLocation[labels[i].second] = curX;
                    }

                    //Up then
                    for (int i = middle+1; i < labels.count(); ++i)
                    {
                        int curX = std::min(preferredLocation(labels[i].first),
                                            preferredLocation(labels[i-1].first)-pointLegendHeight-minDistBetweenLabels);

                        labelLocation[labels[i].second] = curX;
                    }
                }
                else
                {
                    pointPen.setWidth(6);
                }


                for (int col = 0; col < model()->columnCount(); ++col)
                {
                    double value = valueAt(row, col);

                    average[col] += value;

                    pointPen.setColor(QColor(CHART_COLORS[col % CHART_COLORS.size()]));
                    chartLinePen.setColor(QColor(CHART_COLORS[col % CHART_COLORS.size()]));


                    QPoint curPoint(curX, heightFor(value));

                    painter.setPen(pointPen);
                    painter.drawPoint(curPoint);

                    //Check if paint lines
                    if (row > 0)
                    {
                        painter.setPen(chartLinePen);
                        painter.drawLine(previous[col], curPoint);
                    }

                    //Check if current row and need to draw labels
                    if (row == currentHoverRow)
                    {                        
                        QString caption = locale.toString(std::abs(value), 'f', captionPrec+1);
                        int pointLegendWidth = sizeColorLabel + 10 + captionMetrics.width(caption);

                        QRect legendRect(curPoint.x()-10-pointLegendWidth, labelLocation[col],
                                         pointLegendWidth, pointLegendHeight);

                        painter.setBrush(QColor("#FFFEB0"));
                        painter.setPen(foreground);
                        painter.drawRoundedRect(legendRect, 2.0, 2.0);

                        //Color
                        painter.setBrush(QColor(CHART_COLORS[col % CHART_COLORS.size()]));
                        painter.drawRoundedRect(legendRect.x()+3, legendRect.y()+(legendRect.height()-sizeColorLabel)/2,
                                                sizeColorLabel, sizeColorLabel,
                                                2.0, 2.0);

                        legendRect.setLeft(legendRect.left()+3+sizeColorLabel);

                        if (value < 0)
                            caption = "(" + caption + ")";

                        //legendRect.setRight(legendRect.right()-5);

                        //painter.setPen();
                        painter.drawText(legendRect, Qt::AlignCenter, caption);
                    }

                    //Set previous point
                    previous[col] = curPoint;
                }

                //Caption
                painter.setPen(foreground);
                painter.drawText(bottomCaption.translated(0, 10),
                                 Qt::AlignHCenter | Qt::AlignTop,
                                 model()->headerData(row, Qt::Vertical).toString());

                //Lines
                painter.setPen(linePen);

                //Center line
                painter.setRenderHint(QPainter::Antialiasing, false);
                painter.drawLine(QPoint(bottomCaption.center().x(), chartSize.height()),
                                 QPoint(bottomCaption.center().x(), chartSize.height()+6));

                //Vertical line
                if (row != model()->rowCount()-1)
                {
                    drawHorizontalLine(bottomCaption.right());
                    //painter.drawLine(bottomCaption.topRight(), QPoint(bottomCaption.right(), 0));
                    bottomCaption.translate(bottomCaption.width(), 0);
                }

                painter.setRenderHint(QPainter::Antialiasing, true);

                curX += 2*pointSep;
            }


            break;
        }

        }

        //Paint average if required
        if (m_averageVisible)
        {
            for (int col = 0; col < model()->columnCount(); ++col)
            {
                QPen pen(model()->columnCount() == 1 ? Qt::red : QColor(CHART_COLORS[col % CHART_COLORS.size()]));
                pen.setStyle(Qt::DashLine);

                painter.setPen(pen);
                double height = zeroHeight-(double(levelHeight) / interval) * (average[col] / double(model()->rowCount()));

                painter.drawLine(QPoint(0, height), QPoint(chartRect.width(), height));
            }
        }



        painter.restore();


        //=========================================== DRAW THE LEGEND ============================================

        if (m_legendVisible)
        {
            const int colSep = 5;
            const int colorSize = (legendRowH-5)/2;

            QLine rowLine(QPoint(0, 0), QPoint(legendWidth, 0));

            painter.save();

            painter.translate(marginH + (chartSize.width()-legendWidth)/2  - horizontalScrollBar()->value(),
                              chartSize.height()
                                 + marginV*2
                                 + (m_title.isEmpty() ? 0 : titleHeight)
                                 - verticalScrollBar()->value());

            painter.setFont(legendFont);

            for (int col = 0; col < model()->columnCount(); ++col)
            {
                //Color
                painter.setPen(noPen);
                painter.setBrush(QColor(CHART_COLORS[col % CHART_COLORS.size()]));
                painter.drawRoundedRect(QRect(colSep, rowLine.y1()+(legendRowH-colorSize)/2, colorSize, colorSize), 2.0, 2.0);

                //Text
                painter.setPen(foreground);
                painter.drawText(QRect(QPoint(colSep*2+colorSize, rowLine.y1()), QPoint(legendWidth, rowLine.y1()+legendRowH)),
                                 Qt::AlignVCenter | Qt::AlignLeft,
                                 model()->headerData(col, Qt::Horizontal).toString());


                rowLine.translate(0, legendRowH);

                if (col+1 < model()->columnCount())
                {
                    painter.setPen(linePen);
                    painter.drawLine(rowLine);
                }
            }

            painter.restore();
        }

    }

    QRect ReturnChart::visualRect(const QModelIndex &index) const
    {
        QRect rect = itemRect(index);
        if (rect.isValid())
            return QRect(rect.left() - horizontalScrollBar()->value(),
                         rect.top() - verticalScrollBar()->value(),
                         rect.width(), rect.height());
        else
            return rect;
    }

    void ReturnChart::scrollTo(const QModelIndex &index, ScrollHint)
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

    QModelIndex ReturnChart::indexAt(const QPoint &point) const
    {
        if (model()->rowCount() == 0 || model()->columnCount() == 0)
            return QModelIndex();

        // Transform the view coordinates into contents widget coordinates.
        int wx = point.x() + horizontalScrollBar()->value();
        int wy = point.y() + verticalScrollBar()->value();

        QRect chartRect = QRect(QPoint(marginH, marginV + m_title.isEmpty() ? 0 : titleHeight),
                                getChartSize());

        if (!chartRect.contains(wx, wy))
            return QModelIndex();

        int rowWidth = chartRect.width() / model()->rowCount();

        switch (m_style)
        {
        case ReturnChartStyle::Line:
            return model()->index((wx-marginH) / rowWidth, 0);

        case ReturnChartStyle::VerticalBar:
            return QModelIndex();
        }

        return QModelIndex();
    }

    void ReturnChart::mouseMoveEvent(QMouseEvent * event)
    {
        QModelIndex indexUnderMouse = indexAt(viewport()->mapFromParent(event->pos()));

        if (currentHoverRow != indexUnderMouse.row())
        {
            currentHoverRow = indexUnderMouse.row();
            viewport()->update();
        }
    }

    void ReturnChart::rowsInserted(const QModelIndex &parent, int start, int end)
    {
        updateGeometries();
        QAbstractItemView::rowsInserted(parent, start, end);
    }

    void ReturnChart::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
    {
        updateGeometries();
        QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
    }

    void ReturnChart::updateGeometries()
    {
        int widthReq = 0;

        switch (m_style)
        {
        case ReturnChartStyle::Line:
            widthReq = (model()->rowCount()*2)*minPointSpacing
                       + marginH*2;
            break;

        case ReturnChartStyle::VerticalBar:

            widthReq = (model()->columnCount()*minBarWidth + (model()->columnCount()-1)*inBarSpacing)*model()->rowCount()
                        + outBarSpacing*model()->rowCount()*2
                        + marginH*2;
            break;
        }

        int heightReq = minChartHeight + 3*marginV + legendRowH*model()->columnCount() + (m_title.isEmpty() ? 0 : titleHeight);

        horizontalScrollBar()->setPageStep(viewport()->width());
        horizontalScrollBar()->setRange(0, qMax(0, widthReq - viewport()->width()));
        verticalScrollBar()->setPageStep(viewport()->height());
        verticalScrollBar()->setRange(0, qMax(0, heightReq - viewport()->height()));
    }

    QRegion ReturnChart::visualRegionForSelection(const QItemSelection &selection) const
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

}


#ifndef RETURNCHART_H
#define RETURNCHART_H

#include <QAbstractItemView>
#include <QFont>
#include <QItemSelection>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QScrollBar>

namespace KLib
{
    enum class ReturnChartStyle
    {
        VerticalBar,
        Line
    };

    enum class ValueType
    {
        Percent,
        Amount
    };

    class ReturnChart : public QAbstractItemView
    {
        Q_OBJECT

        public:
            ReturnChart(ReturnChartStyle _style, ValueType _valueType, Qt::ItemDataRole _dataRole, QWidget* _parent = nullptr);

            QRect       visualRect(const QModelIndex &index) const override;
            void        scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible) override;
            QModelIndex indexAt(const QPoint &point) const override;

            bool legendIsVisible() const { return m_legendVisible; }
            bool averageIsVisible() const { return m_averageVisible; }

            QString title() const { return m_title; }
            void setTitle(const QString& _title);

            /**
             * @brief Will be indicated next to the highest y-axis label (ex: 20 %)
             * @param _before Will be before the value (% 20) if true, after the value otherwise (20 %)
             */
            void setYAxisIndicator(const QString& _indicator, bool _before);

        public slots:
            void setLegendVisible(bool _visible);
            void setAverageVisible(bool _visible);


        protected slots:
            void rowsInserted(const QModelIndex &parent, int start, int end) override;
            void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) override;

            bool edit(const QModelIndex&, EditTrigger, QEvent*) override { return false; }
            QModelIndex moveCursor(QAbstractItemView::CursorAction, Qt::KeyboardModifiers) override { return QModelIndex(); }

            int horizontalOffset() const override   { return horizontalScrollBar()->value(); }
            int verticalOffset() const override     { return verticalScrollBar()->value(); }

            bool isIndexHidden(const QModelIndex&) const override { return false; }
            void setSelection(const QRect&, QItemSelectionModel::SelectionFlags) override {}


            void paintEvent(QPaintEvent *event) override;
            void resizeEvent(QResizeEvent*) override        { updateGeometries(); }
            void scrollContentsBy(int dx, int dy) override   { viewport()->scroll(dx, dy); }

            void mouseMoveEvent(QMouseEvent * event) override;

            QRegion visualRegionForSelection(const QItemSelection &selection) const override;

        private:
            QRect itemRect(const QModelIndex&) const { return QRect(); }
            void updateGeometries() override;

            QSize getChartSize() const;

            double valueAt(int _row, int _col) const;

            const ReturnChartStyle m_style;
            const ValueType        m_valueType;
            Qt::ItemDataRole       m_role;

            int currentHoverRow;
            bool m_legendVisible;
            bool m_averageVisible;
            QString m_title;

            QPair<QString, bool> m_yAxisIndicator;

            QColor lineColor;
            QFont titleFont;
            QFont captionFont;
            QFont legendFont;

            int minChartHeight;
            int maxChartHeight;
            int minLevelHeight;
            int maxLevelHeight;

            int legendWidth;
            int legendRowH;

            int titleHeight;
            int marginH;
            int marginV;

            //Bar chart
            int minBarWidth;
            int inBarSpacing;
            int outBarSpacing;

            //Line chart
            int minPointSpacing;

            static const QStringList CHART_COLORS;
    };

}

#endif // RETURNCHART_H

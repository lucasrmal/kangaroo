#include "ihomewidget.h"
#include <QPainter>
#include <QFontMetrics>
#include <QVBoxLayout>

const QFont  IHomeWidget::TITLEBAR_FONT      = ([]() { QFont f; f.setBold(true); return f; })();
const int    IHomeWidget::TITLEBAR_HEIGHT    = QFontMetrics(IHomeWidget::TITLEBAR_FONT).height() + 10;
const QColor IHomeWidget::TITLEBAR_BACKCOLOR = QColor("#ADE494");
const QColor IHomeWidget::WIDGET_BACKCOLOR   = QColor("#EFF0F1");

IHomeWidget::IHomeWidget(QWidget* _centralWidget, QWidget* _parent) :
    QWidget(_parent),
    m_widget(_centralWidget)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addSpacing(TITLEBAR_HEIGHT + 2);
    layout->addWidget(m_widget);
}

void IHomeWidget::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    QRect titleRect(0, 0, width(), TITLEBAR_HEIGHT);
    QRect contentRect(0, 0, width() - 1, height() - 1);

    //Fill the background
    painter.fillRect(contentRect, WIDGET_BACKCOLOR);

    //Paint the label
    painter.fillRect(titleRect, TITLEBAR_BACKCOLOR);

    //Draw the title
    titleRect.setX(titleRect.x() + 10);
    painter.setPen(palette().color(QPalette::Foreground));
    painter.setFont(TITLEBAR_FONT);
    painter.drawText(titleRect, Qt::AlignLeft | Qt::AlignVCenter, title());

    //Draw the border
    painter.setPen(Qt::black);
    painter.drawRect(contentRect);

    painter.end();
}

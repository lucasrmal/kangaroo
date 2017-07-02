#include "standardtab.h"
#include "../models/portfolio.h"
#include <KangarooLib/ui/core.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

StandardTab::StandardTab(Portfolio* _portfolio, const QString& _title, QWidget *parent) :
    QWidget(parent),
    m_portfolio(_portfolio),
    m_title(_title)
{
    m_lblTitle = new QLabel(this);
    QLabel* lblPicture = new QLabel(this);
    lblPicture->setPixmap(KLib::Core::pixmap("dividend"));

    QPushButton* btnReload = new QPushButton(KLib::Core::icon("view-refresh"), "", this);
    connect(btnReload, &QPushButton::clicked, this, &StandardTab::refreshRequested);

    m_mainLayout = new QVBoxLayout(this);
    m_layoutTop = new QHBoxLayout();
    m_layoutBottom = new QHBoxLayout();

    m_stack = new QStackedWidget(this);

    m_mainLayout->setContentsMargins(0,0,0,0);
    m_layoutTop->setContentsMargins(6,6,6,0);

    m_layoutButton = new QHBoxLayout();
    m_layoutButton->setSpacing(0);
    m_layoutButton->addSpacing(10);
    m_layoutButton->addWidget(btnReload);

    m_layoutTop->addWidget(lblPicture);
    m_layoutTop->addWidget(m_lblTitle);
    m_layoutTop->addStretch(2);
    m_layoutTop->addLayout(m_layoutButton);

    m_mainLayout->addLayout(m_layoutTop);
    m_mainLayout->addWidget(m_stack);
    m_mainLayout->addLayout(m_layoutBottom);

    QFont f;
    f.setPointSize(14);
    f.setBold(true);
    m_lblTitle->setFont(f);

    onPortfolioNameChanged();
}

void StandardTab::addTab(const QString& _title, QWidget* _widget)
{
    QPushButton* button = new QPushButton(_title, this);
    m_layoutButton->insertWidget(m_buttons.size(), button);
    button->setCheckable(true);
    button->setAutoExclusive(true);

    m_buttons << button;
    m_stack->addWidget(_widget);

    connect(button, &QPushButton::toggled, this, &StandardTab::buttonToggled);
}

void StandardTab::onPortfolioNameChanged()
{
    m_lblTitle->setText(tr("%1 - %2").arg(m_title).arg(m_portfolio->portfolioName()));
}

void StandardTab::buttonToggled(bool _toggled)
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());

    if (button && _toggled)
    {
        int idx = m_buttons.indexOf(button);

        if (idx != -1)
        {
            m_stack->setCurrentIndex(idx);
        }
    }
}



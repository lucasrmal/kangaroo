#include "investingpane.h"
#include "models/portfolio.h"

#include "../TabInterface/tabinterface.h"

#include <KangarooLib/model/account.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/ui/core.h>

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <functional>

using namespace KLib;

QMultiMap<int, InvestingPane::InvestingView> InvestingPane::m_views = QMultiMap<int, InvestingPane::InvestingView>();

InvestingPane::InvestingPane(QWidget *parent) : QWidget(parent)
{
    //Create widgets
    m_cboAccounts = new QComboBox(this);

    for (int i = 0; i < NUM_INFOS; ++i)
    {
        m_lblInfos[i] = new QLabel(this);
    }

    //Layout
    m_layout = new QFormLayout(this);
    m_layout->addRow(tr("&Account:"), m_cboAccounts);

    m_layout->addItem(new QSpacerItem(10, 20));
    m_layout->addRow(new QLabel(QString("<b>%1</b>").arg(tr("Overview"))));

    for (int i = 0; i < NUM_INFOS; ++i)
    {
        m_layout->addRow(acccountInfoLabel(i), m_lblInfos[i]);
    }

    m_layout->addItem(new QSpacerItem(10, 20));
    m_layout->addRow(new QLabel(QString("<b>%1</b>").arg(tr("Views"))));

    for (QPushButton* btn : m_btnViews)
    {
        m_layout->addRow(btn);
    }

    loadAccounts();
    onAccountChanged();
    loadViews();



    connect(m_cboAccounts, SIGNAL(currentIndexChanged(int)), this, SLOT(onAccountChanged()));
}

void InvestingPane::registerView(const QString& _identifier,
                                 int _weight,
                                 const QString& _btnText,
                                 const QString& _btnIcon,
                                 fn_buildInvTab _buildTab)
{
    m_views.insert(_weight, InvestingView{_identifier, _btnText, _btnIcon, _buildTab});
}

void InvestingPane::loadViews()
{
    for (const InvestingView& view : m_views)
    {
        //Create the button
        QPushButton* btn = new QPushButton(Core::icon(view.buttonIcon), view.buttonText, this);
        btn->setFlat(true);
        btn->setStyleSheet("text-align: left");

        //Add it to the layout
        m_layout->addRow(btn);

        //Connect the button
        connect(btn, &QPushButton::clicked, [view, this] ()
        {
            if (m_cboAccounts->currentIndex() < 0)
                return;

            int idx = m_cboAccounts->currentIndex();
            QString id = QString("%1%2").arg(view.id).arg(idx);

            if (!TabInterface::instance()->containsTab(id))
            {
                TabInterface::instance()->addRegisteredTab(view.buildTab(m_portfolios[idx], this),
                                                           QString("%1 - %2").arg(view.buttonText).arg(m_cboAccounts->currentText()),
                                                           id,
                                                           true,
                                                           Core::icon(view.buttonIcon));
            }
            else
            {
                TabInterface::instance()->setFocus(id);
            }

        });
    }
}

void InvestingPane::onAccountChanged()
{
    if (m_cboAccounts->currentIndex() < 0)
        return;

    //Change the infos on the tab
    Portfolio* current = m_portfolios[m_cboAccounts->currentIndex()];

    try
    {
        m_lblInfos[MarketValue]->setText(current->currency()->formatAmount(current->totalMarketValue()));
        m_lblInfos[CostBasis]->setText(current->currency()->formatAmount(current->totalCostBasis()));
        m_lblInfos[Gain]->setText(current->formatGainLoss(current->totalProfitLoss()));
        m_lblInfos[GainPerc]->setText(Portfolio::formatPercChange(current->totalPercProfitLoss()));
        m_lblInfos[NumPositions]->setText(QString::number(current->positionCount()));


        //Compute the largest positions
        QList<Position> pos = current->positions();

        std::sort(pos.begin(), pos.end(), [](const Position& a, const Position& b) {
            return a.marketValue() > b.marketValue();
        });

        QString largest;

        for (int i = 0; i < std::min(3, pos.size()); ++i)
        {
            if (!largest.isEmpty())
                largest.append(", ");

            largest.append(pos[i].security->symbol());
        }

        if (pos.size() > 3)
            largest.append(", ...");


        m_lblInfos[LargestPositions]->setText(largest);
    }
    catch (...) {}
}

void InvestingPane::onPortfolioChanged()
{
    Portfolio* p = qobject_cast<Portfolio*>(sender());

    if (p && m_cboAccounts->currentIndex() >= 0 && p == m_portfolios[m_cboAccounts->currentIndex()])
    {
        onAccountChanged();
    }
}

void InvestingPane::loadAccounts()
{
    std::function<void(Account*)> load_models_rec;

    load_models_rec = [this, &load_models_rec] (Account* _a)
    {
        //Account must have type Asset/Bank Account/Investment/Cash
        if (Account::generalType(_a->type()) != AccountType::ASSET
            && _a != Account::getTopLevel())
            return;

        //Add the account if at least 1 of its children are investment accounts
        bool add = false;

        for (Account* child : _a->getChildren())
        {
            add = add || child->type() == AccountType::INVESTMENT;

            load_models_rec(child);
        }

        if (add)
        {
            m_cboAccounts->addItem(_a->name(), _a->id());

            Portfolio* p = new Portfolio(_a, this);

            connect(p, SIGNAL(parentCurrencyPriceModified()), this, SLOT(onPortfolioChanged()));
            connect(p, SIGNAL(positionAdded(KLib::Security*)), this, SLOT(onPortfolioChanged()));
            connect(p, SIGNAL(positionDataChanged(KLib::Security*)), this, SLOT(onPortfolioChanged()));
            connect(p, SIGNAL(positionRemoved(KLib::Security*,int)), this, SLOT(onPortfolioChanged()));
            connect(p, SIGNAL(portfolioSecurityModified(KLib::Security*)), this, SLOT(onPortfolioChanged()));
            connect(p, SIGNAL(securityPriceModified(KLib::Security*)), this, SLOT(onPortfolioChanged()));

            m_portfolios << p;
        }
    };

    m_portfolios.clear(); //No! Fix this!!!
    m_cboAccounts->clear();

    m_cboAccounts->addItem(tr("All Accounts"), -1);
    m_portfolios << new Portfolio(Account::getTopLevel(), this);

    load_models_rec(Account::getTopLevel());
}

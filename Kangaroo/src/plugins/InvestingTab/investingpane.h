#ifndef INVESTINGPANE_H
#define INVESTINGPANE_H

#include <QWidget>
#include <QList>
#include <functional>
#include <QMultiMap>
#include "../TabInterface/centralwidget.h"

class Portfolio;
//class PositionsDetailModel;
class QComboBox;
class QLabel;
class QPushButton;
class QFormLayout;

namespace KLib
{
    class Account;
}

class InvestingPane : public QWidget
{
    Q_OBJECT

    public:
        typedef std::function<QWidget*(Portfolio*, QWidget*)> fn_buildInvTab;

    private:

        enum AccountInfos
        {
            MarketValue,
            CostBasis,
            Gain,
            GainPerc,
            NumPositions,
            LargestPositions,

            NUM_INFOS
        };

        static QString acccountInfoLabel(int i)
        {
            switch (i)
            {
            case MarketValue:
                return tr("Market Value:");

            case CostBasis:
                return tr("Cost Basis:");

            case Gain:
                return tr("Gain:");

            case GainPerc:
                return tr("Gain %:");

            case NumPositions:
                return tr("Num. positions:");

            case LargestPositions:
                return tr("Largest positions:");

            default:
                return QString();
            }
        }

        struct InvestingView
        {
                QString id;
                QString buttonText;
                QString buttonIcon;
                fn_buildInvTab buildTab;
        };

    public:
        explicit InvestingPane(QWidget *parent = nullptr);

        static void registerView(const QString& _identifier,
                                 int            _weight,
                                 const QString& _btnText,
                                 const QString& _btnIcon,
                                 fn_buildInvTab _buildTab);

    private slots:
        void onAccountChanged();
        void onPortfolioChanged();

        //We need some signals if accounts are added, modified, etc.

    private:
        void loadAccounts();
        void loadViews();


        QFormLayout* m_layout;
        QComboBox* m_cboAccounts;
        QLabel* m_lblInfos[NUM_INFOS];

        QList<QPushButton*> m_btnViews;

        QList<Portfolio*> m_portfolios;

        static QMultiMap<int, InvestingView> m_views;
};

class InvestingPaneBuilder : public ISidePane
{
    public:
        QString title() const override
        {
            return QObject::tr("Investing");
        }

        int weight() const override
        {
            return 20;
        }

        QWidget* buildPane(QWidget* _parent) const override
        {
            return new InvestingPane(_parent);
        }
};

#endif // INVESTINGPANE_H

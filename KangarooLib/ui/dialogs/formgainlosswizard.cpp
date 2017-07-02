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

#include "formgainlosswizard.h"
#include "../core.h"
#include "../widgets/amountedit.h"
#include "../../model/investmentlotsmanager.h"
#include "../../model/investmenttransaction.h"
#include "../../model/ledger.h"
#include "../../model/security.h"
#include "../../model/account.h"
#include "../../model/currency.h"
#include "../../model/modelexception.h"

#include <QRadioButton>
#include <QStackedWidget>
#include <QLabel>
#include <QGridLayout>
#include <QTextEdit>
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>
#include <QDebug>

namespace KLib
{

    FormGainLossWizard::FormGainLossWizard(const GainLossData& _data, QWidget* _parent) :
        CAMSEGDialog(DialogWithPicture, OkCancelButtons, _parent),
        m_data(_data)
    {
        setPicture(Core::pixmap("office-chart-line"));
        setBothTitles(tr("Gain/Loss Wizard"));

        m_security = SecurityManager::instance()->get(Account::getTopLevel()->account(m_data.idAccount)->idSecurity());

        /* Widgets */

        m_optAverageCost = new QRadioButton(tr("&Average Cost"), this);
        m_optFIFO        = new QRadioButton(tr("&First In First Out"), this);
        m_optSelectLots  = new QRadioButton(tr("Select &Lots"), this);

        m_lblCostBasis = new QLabel(this);
        m_lblGainLoss = new QLabel(this);

        m_optAverageCost->setChecked(true);

        m_stack = new QStackedWidget(this);
        //m_stack->setMinimumSize(450, 200);

        QFrame* bottomLine = new QFrame(this);
        bottomLine->setFrameShape(QFrame::HLine);

        //QScrollArea* scroll = new QScrollArea(this);
        //scroll->setWidget(m_stack);
        //scroll->setSizeAdjustPolicy(QAbstractScrollArea::AdjustToContents);

        QGridLayout* mainLayout = new QGridLayout(centralWidget());
        mainLayout->addWidget(new QLabel(tr("Shares sold: %1").arg(_data.numShares.toString()),
                                         this), 0, 0);
        mainLayout->addWidget(new QLabel(tr("Price per share: %1").arg(_data.pricePerShare.toString()),
                                         this), 1, 0);
        mainLayout->addWidget(new QLabel(tr("Fee: %1").arg(_data.fee.toString()),
                                         this), 2, 0);
        mainLayout->addWidget(new QLabel(tr("Net Proceeds: %1").arg(_data.netProceeds().toString()),
                                         this), 3, 0);

        mainLayout->addWidget(m_optAverageCost, 0, 1);
        mainLayout->addWidget(m_optFIFO,        1, 1);
        mainLayout->addWidget(m_optSelectLots,  2, 1);

        mainLayout->addWidget(bottomLine, 4, 0, 1, 2);
        //mainLayout->addWidget(scroll,     5, 0, 1, 2);
        mainLayout->addWidget(m_stack,     5, 0, 1, 2);

        mainLayout->addWidget(m_lblCostBasis, 6, 0);
        mainLayout->addWidget(m_lblGainLoss,  6, 1);


        /* Average Cost Widget */

        m_txtDetailsAverageCost = new QTextEdit(this);
        m_txtDetailsAverageCost->setReadOnly(true);
        m_stack->addWidget(m_txtDetailsAverageCost);
        loadAverageCostData();

        /* FIFO Widget */
        m_FIFOWidget = new QWidget(this);
        m_stack->addWidget(m_FIFOWidget);
        loadLots();

        connect(m_optAverageCost, SIGNAL(toggled(bool)), this, SLOT(onMethodChanged()));
        connect(m_optFIFO, SIGNAL(toggled(bool)), this, SLOT(onMethodChanged()));
        connect(m_optSelectLots, SIGNAL(toggled(bool)), this, SLOT(onMethodChanged()));

        computeGain();
        loadCurrent();
    }

    void FormGainLossWizard::accept()
    {
        m_lots.clear();

        //Check if lots are all in range and sum up to the correct value
        if (!m_optAverageCost->isChecked())
        {
            Amount total(0, m_security->precision());

            for (auto i = m_lotUsages.begin(); i != m_lotUsages.end(); ++i)
            {
                if (i.value().editor->amount() > i.value().max)
                {
                    QMessageBox::information(this,
                                             tr("Select Lots"),
                                             tr("The value for lot <b>%1</b> exceeds its availability.")
                                                .arg(i.key().toString()));
                    return;
                }
                else
                {
                    m_lots[i.value().lotNumber] = i.value().editor->amount();
                }

                total += i.value().editor->amount();
            }

            if (total != m_data.numShares.abs())
            {
                QMessageBox::information(this,
                                         tr("Select Lots"),
                                         tr("The number of shares/units selected (%1) does not match "
                                            "the number of shares/unit required (%2).")
                                            .arg(total.toString())
                                            .arg(m_data.numShares.abs().toString()));
                return;
            }
        }

        done(QDialog::Accepted);
    }

    void FormGainLossWizard::onMethodChanged()
    {
        m_stack->setCurrentIndex(m_optAverageCost->isChecked() ? 0 : 1);

        //Enable or disable lot selectors if FIFO
        Amount remaining = m_data.numShares.abs();
        if (!m_optAverageCost->isChecked())
        {
            for (LotUsage& l : m_lotUsages)
            {
                l.editor->setEnabled(m_optSelectLots->isChecked());

                if (m_optFIFO->isChecked() && remaining > 0)
                {
                    l.editor->setAmount(std::min(remaining, l.max));
                    remaining -= l.max;
                }
                else if (m_optFIFO->isChecked())
                {
                    l.editor->setAmount(0);
                }
            }
        }

        computeGain();
    }

    void FormGainLossWizard::computeGain()
    {
        //Compute cost basis
        Amount costBasis;

        if (m_optAverageCost->isChecked())
        {
            costBasis = m_avgCostBase;
        }
        else
        {
            Amount total(0, m_security->precision());
            for (const LotUsage& l : m_lotUsages)
            {
                costBasis += l.netPricePerShare * l.editor->amount();
                total += l.editor->amount();
            }

            QString text = "<b>" + total.toString() + "</b>";

            if (total != m_data.numShares.abs())
            {
                m_lblTotal->setText("<font color='red'>" + text + "</font>");
            }
            else
            {
                m_lblTotal->setText(text);
            }
        }

        //Compute gain
        m_gainLoss = m_data.netProceeds()-costBasis;

        //Show new gain/loss
        m_lblCostBasis->setText(tr("Cost Basis: <b>%1</b>").arg(costBasis.toString()));
        m_lblGainLoss->setText(tr("Gain/Loss: <b>%1</b>").arg(m_gainLoss.toString()));
    }

    void FormGainLossWizard::loadCurrent()
    {
        if (!m_data.lots.isEmpty())
        {
            m_optSelectLots->setChecked(true);

            for (const LotUsage& us : m_lotUsages)
            {
                if (m_data.lots.contains(us.lotNumber))
                {
                    us.editor->setAmount(std::min(us.max, m_data.lots[us.lotNumber]));
                }
                else
                {
                    us.editor->setAmount(0);
                }
            }
        }
    }

    void FormGainLossWizard::loadLots()
    {
        try
        {
            Lots l = InvestmentLotsManager::instance()
                     ->lotsAvailableExcluding(InvestmentLotsManager::investmentActionClass(m_data.action),
                                              m_data.idAccount,
                                              m_data.transactionDate,
                                              m_data.idTransaction);

            QGridLayout* lotsLayout = new QGridLayout(m_FIFOWidget);
            lotsLayout->setSizeConstraint(QLayout::SetFixedSize);

            //Fixme: add lots used by this transaction if saved already...


//            QString text = m_data.action == InvestmentAction::Sell ? tr("%1, bought %2 shares at %3 (net):")
//                                                                   : tr("%1, short sold %2 shares at %3 (net):");


            for (auto i = l.begin(); i != l.end(); ++i)
            {
                const InvestmentTransaction* tr = InvestmentLotsManager::instance()->transactionForLot(i.key());

                AmountEdit* lotEdit = new AmountEdit(this);
                lotEdit->setPrecision(m_security->precision());
                lotEdit->setMaximum(i.value());
                connect(lotEdit, SIGNAL(valueChanged(double)), this, SLOT(computeGain()));

                m_lotUsages.insert(tr->date(), LotUsage{i.key(), lotEdit, tr->netPricePerShare(), i.value()});
            }

            QString text = tr("%1, %2 shares available at %3 (net)");
            int row = 0;

            //Now insert them in the grid layout (they will be in order)
            for (auto i = m_lotUsages.begin(); i != m_lotUsages.end(); ++i, ++row)
            {
                QLabel* label = new QLabel(text.arg(i.key().toString())
                                               .arg(i.value().max.toString())
                                               .arg(i.value().netPricePerShare.toString()));


                lotsLayout->addWidget(label, row, 0);
                lotsLayout->addWidget(i.value().editor, row, 1);
            }


            QLabel* lblTotalLabel = new QLabel(tr("Total (must be %1):").arg(m_data.numShares.abs().toString()), this);
            lblTotalLabel->setAlignment(Qt::AlignHCenter | Qt::AlignRight);

            m_lblTotal = new QLabel(this);
            m_lblTotal->setAlignment(Qt::AlignHCenter | Qt::AlignRight);

            lotsLayout->addWidget(lblTotalLabel, row, 0);
            lotsLayout->addWidget(m_lblTotal,    row, 1);
            lotsLayout->setRowStretch(row+1, 10);
        }
        catch (ModelException e)
        {
            QMessageBox::warning(this,
                                 tr("Unexpected Error"),
                                 e.description());
        }
    }

    void FormGainLossWizard::loadAverageCostData()
    {
        Amount cost, balance, curcost;
        Account* a = Account::getTopLevel()->account(m_data.idAccount);


        if (!a)
        {
            m_txtDetailsAverageCost->setText(tr("Error! Account is invalid."));
            return;
        }

        Currency* cur = CurrencyManager::instance()->get(m_security->currency());

        auto formatCur = [cur] (const Amount& _a) { return cur->formatAmount(_a); };


        auto transactions = a->ledger()->transactions();
        QString text;

        text.append("<table border='0' width='100%'>");

        for (auto i = transactions->begin(); i != transactions->end() && i.key() <= m_data.transactionDate; ++i)
        {
            const InvestmentTransaction* trans = qobject_cast<InvestmentTransaction*>(i.value());

            if (!trans || trans->id() == m_data.idTransaction)
                continue;

            QString curLine = "<tr>";

            curLine.append("<td>"+trans->date().toString("MMM d yyyy")+"</td>");

            switch (trans->action())
            {
            case InvestmentAction::Buy:
            //case InvestmentAction::ShortCover:
            case InvestmentAction::ReinvestDiv:
            case InvestmentAction::ReinvestDistrib:
                curcost = trans->pricePerShare()*trans->shareCount() + trans->fee();
                cost += curcost;

                curLine.append(tr("<td>%5: %1 at %2/share, %3 fee</td><td align='right'>%4</td>")
                                     .arg(trans->shareCount().toString())
                                     .arg(formatCur(trans->pricePerShare()))
                                     .arg(formatCur(trans->fee()))
                                     .arg(formatCur(curcost))
                                     .arg(InvestmentTransaction::actionToString(trans->action())));
                break;

            case InvestmentAction::Sell:
            //case InvestmentAction::ShortSell:
                curcost = -(trans->pricePerShare()*trans->shareCount().abs() - trans->fee() + trans->gainLoss());
                cost += curcost;

                curLine.append(tr("<td>%5: %1 at %2/share, %3 fee</td><td align='right'>%4</td>")
                                     .arg(trans->shareCount().abs().toString())
                                     .arg(formatCur(trans->pricePerShare()))
                                     .arg(formatCur(trans->fee()))
                                     .arg(formatCur(curcost))
                                     .arg(InvestmentTransaction::actionToString(trans->action())));
                break;

            case InvestmentAction::Transfer:
            case InvestmentAction::Swap:
                if (trans->idInvestmentAccount() != m_data.idAccount) //Take cost basis from other account
                {
                    int otherAccount = trans->idInvestmentAccount();
                    Amount balanceBef = LedgerManager::instance()->ledger(otherAccount)->balanceBefore(i.value());
                    double frac = balanceBef == trans->shareCount()
                                  ? 1.0
                                  : trans->shareCount().toDouble() / balanceBef.toDouble();

                    curcost = LedgerManager::instance()->ledger(otherAccount)->costBasisBefore(i.value())
                            * frac;
                    cost += curcost;

                    curLine.append(tr("<td>Transfer in %1 shares</td><td align='right'>%2</td>")
                                         .arg(trans->shareCount().toString())
                                         .arg(formatCur(curcost)));
                }
                else //Take cost basis from this account.
                {
                    double frac = balance == trans->shareCount().abs()
                                  ? 1.0
                                  : trans->shareCount().toDouble() / balance.toDouble();

                    curcost = -(cost*frac);
                    cost += curcost;

                    curLine.append(tr("<td>Transfer out %1 shares</td><td align='right'>%2</td>")
                                         .arg(trans->shareCount().abs().toString())
                                         .arg(formatCur(curcost)));
                }
                break;

            case InvestmentAction::Distribution:
                if (trans->distribComposition().contains(DistribType::ReturnOfCapital)
                    && trans->distribComposition().value(DistribType::ReturnOfCapital) > 0)
                {
                    curcost = trans->splitFor(InvestmentSplitType::DistributionSource).amount //Will be negative - we want that.
                              * (trans->distribComposition().value(DistribType::ReturnOfCapital, 0)/100.0);
                    cost += curcost;

                    curLine.append(tr("<td>Return of Capital in Distribution:</td><td align='right'>%1</td>")
                                         .arg(formatCur(curcost)));
                }
                else
                {
                    curLine.append(tr("<td>Distribution: No Return of Capital</td><td align='right'>&nbsp;</td>")
                                         .arg(formatCur(curcost)));
                }
                break;

            case InvestmentAction::StockSplit:
                balance = InvestmentTransaction::balanceAfterSplit(balance, trans->splitFraction());
                break;


            case InvestmentAction::UndistributedCapitalGain:
            case InvestmentAction::CostBasisAdjustment:
                curcost = trans->basisAdjustment();
                cost += curcost;

                curLine.append(QString("<td>%1</td><td align='right'>%2</td>")
                                .arg(InvestmentTransaction::actionToString(trans->action()))
                                .arg(formatCur(curcost)));
                break;

            default:
                curLine.clear();
                break;
            }

            if (!curLine.isEmpty())
                text.append(curLine + "</tr>");

            //Check if distrib or reinvested dist and ReturnOfCapital
            if (trans->action() == InvestmentAction::ReinvestDistrib
                && trans->distribComposition().contains(DistribType::ReturnOfCapital)
                && trans->distribComposition().value(DistribType::ReturnOfCapital) > 0)
            {
                curcost = trans->splitFor(InvestmentSplitType::DistributionSource).amount //Will be negative - we want that.
                          * (trans->distribComposition().value(DistribType::ReturnOfCapital)/100.0);
                cost += curcost;

                text.append(tr("<tr><td>&nbsp;</td><td>Return of Capital in Distribution:</td><td align='right'>%1</td></tr>")
                                     .arg(formatCur(curcost)));
            }

            //Update the balance
            for (Transaction::Split s : i.value()->splits())
            {
                if (s.idAccount == m_data.idAccount)
                {
                    balance += s.amount;
                }
            }
        }

        m_avgCostBase = cost;
        Amount curTotal = m_data.netProceeds();

        text.append(tr("<tr><td colspan='2'><b>Total cost basis: </b></td><td align='right'><b>%1</b></td></tr>")
                             .arg(formatCur(cost)));

        text.append("<tr></tr>"); //Add empty row to separate

        text.append(tr("<tr><td colspan='2'><b>Sell %1 at %2/share, %3 fee</b></td><td align='right'><b>%4</b></td></tr>")
                             .arg(m_data.numShares.toString())
                             .arg(formatCur(m_data.pricePerShare))
                             .arg(formatCur(m_data.fee))
                             .arg(formatCur(curTotal)));

        if (balance != m_data.numShares) //Fraction
        {
            double frac = m_data.numShares.toDouble() / balance.toDouble();

            m_avgCostBase = cost * frac;
        }

        text.append(tr("<tr><td colspan='2'><b>Cost basis for current fraction: </b></td><td align='right'><b>-%1</b></td></tr>")
                             .arg(m_avgCostBase.toString()));

        Amount gainLoss = curTotal - m_avgCostBase;

        text.append(tr("<tr><td colspan='2'><b>Computed Gain/Loss: </b></td><td align='right'><b>%1</b></td></tr>")
                             .arg(formatCur(gainLoss)));

        text.append("</table>");

        m_txtDetailsAverageCost->setHtml(text);
    }

}


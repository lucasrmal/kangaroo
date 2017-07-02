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

#ifndef FORMGAINLOSSWIZARD_H
#define FORMGAINLOSSWIZARD_H

#include "../../model/investmenttransaction.h"
#include "camsegdialog.h"
#include <QMultiMap>

class QRadioButton;
class QStackedWidget;
class QTextEdit;

namespace KLib
{
    class AmountEdit;
    class Security;

    struct GainLossData
    {
        Amount numShares;
        Amount pricePerShare;
        Amount fee;
        int    idAccount;
        int    idTransaction;   ///< Should be Constants::NO_ID if this is a new transaction
        QDate  transactionDate;
        InvestmentAction action;

        //Current stuff
        Lots lots;

        Amount netProceeds() const { return numShares.abs()*pricePerShare-fee; }
    };

    class FormGainLossWizard : public CAMSEGDialog
    {
        Q_OBJECT

        struct LotUsage
        {
            int lotNumber;
            AmountEdit* editor;
            Amount netPricePerShare;
            Amount max;
        };

        public:
            explicit FormGainLossWizard(const GainLossData& _data, QWidget* _parent = nullptr);

            void accept();

            const Lots& lots() const        { return m_lots; }
            const Amount& gainLoss() const  { return m_gainLoss; }

            QSize sizeHint() const { return QSize(600, 600); }

        private slots:
            void onMethodChanged();
            void computeGain();

        private:
            void loadAverageCostData();
            void loadLots();
            void loadCurrent();

            const GainLossData m_data;
            Lots m_lots;
            Amount m_gainLoss;
            Amount m_avgCostBase;

            QRadioButton* m_optAverageCost;
            QRadioButton* m_optFIFO;
            QRadioButton* m_optSelectLots;

            QStackedWidget* m_stack;

            //Average
            QTextEdit* m_txtDetailsAverageCost;

            //FIFO/Lots
            QWidget* m_FIFOWidget;
            QMultiMap<QDate, LotUsage> m_lotUsages;
            QLabel* m_lblTotal;

            //Total
            QLabel* m_lblCostBasis;
            QLabel* m_lblGainLoss;

            Security* m_security;

    };

}

#endif // FORMGAINLOSSWIZARD_H

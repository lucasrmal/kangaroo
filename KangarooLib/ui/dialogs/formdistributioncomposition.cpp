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

#include "formdistributioncomposition.h"

#include <QLabel>
#include <QGridLayout>
#include <QMessageBox>
#include "../core.h"
#include "../widgets/amountedit.h"

namespace KLib
{

    FormDistributionComposition::FormDistributionComposition(DistribComposition& _comp, QWidget* _parent) :
        CAMSEGDialog(DialogWithPicture, OkCancelButtons, _parent),
        m_comp(_comp)
    {
        setPicture(Core::pixmap("office-chart-pie"));
        setBothTitles(tr("Distribution Composition"));


        QGridLayout* grid = new QGridLayout(centralWidget());

        for (int i = 0; i < DISTR_TYPES_COUNT; ++i)
        {
            m_composition[i] = new AmountEdit(this);
            m_composition[i]->setMinimum(0);
            m_composition[i]->setMaximum(100);
            m_composition[i]->setPrecision(3);

            if (_comp.contains((DistribType) i))
            {
                m_composition[i]->setAmount(_comp[(DistribType) i]);
            }

            connect(m_composition[i], SIGNAL(valueChanged(double)), this, SLOT(updateTotal()));

            QLabel* lblComp = new QLabel(tr("&%1: ").arg(InvestmentTransaction::distribTypeToString((DistribType) i)),
                                         this);

            lblComp->setBuddy(m_composition[i]);

            grid->addWidget(lblComp, i, 0, 2, 1);
            grid->addWidget(m_composition[i], i, 2);
        }

        QLabel* lblTotalLabel = new QLabel(tr("Total (must be 100%):"), this);
        lblTotalLabel->setAlignment(Qt::AlignHCenter | Qt::AlignRight);

        m_lblTotal = new QLabel(this);
        m_lblTotal->setAlignment(Qt::AlignHCenter | Qt::AlignRight);

        grid->addWidget(lblTotalLabel, DISTR_TYPES_COUNT, 1);
        grid->addWidget(m_lblTotal,    DISTR_TYPES_COUNT, 2);

        updateTotal();
    }

    void FormDistributionComposition::accept()
    {
        Amount total; computeTotal(total);

        if (total != 100)
        {
            QMessageBox::information(this,
                                     tr("Distribution Composition"),
                                     tr("The sum of each component of the distribution must equal 100%."));
        }
        else
        {
            m_comp.clear();

            for (int i = 0; i < DISTR_TYPES_COUNT; ++i)
            {
                m_comp[(DistribType) i] = m_composition[i]->amount();
            }

            done(QDialog::Accepted);
        }
    }

    void FormDistributionComposition::updateTotal()
    {
        Amount total; computeTotal(total);
        QString text = "<b>" + total.toString() + " %</b>";

        if (total != 100)
        {
            m_lblTotal->setText("<font color='red'>" + text + "</font>");
        }
        else
        {
            m_lblTotal->setText(text);
        }
    }

    void FormDistributionComposition::computeTotal(Amount& _total) const
    {
        for (int i = 0; i < DISTR_TYPES_COUNT; ++i)
        {
            _total += m_composition[i]->amount();
        }
    }

}


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

#include "formcurrencyexchange.h"
#include "../core.h"
#include "../widgets/amountedit.h"
#include "../../model/currency.h"
#include "../../model/pricemanager.h"

#include <QLabel>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QRadioButton>
#include <QLabel>
#include <QMessageBox>
#include <QDoubleSpinBox>
#include <QCheckBox>

namespace KLib
{

    FormCurrencyExchange::FormCurrencyExchange(const QString& _from,
                                               const QString& _to,
                                               const Amount& _amount,
                                               const QDate& _date,
                                               QWidget *parent) :
        CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
        m_curFrom(CurrencyManager::instance()->get(_from)),
        m_curTo(CurrencyManager::instance()->get(_to)),
        m_amount(_amount),
        m_date(_date),
        m_absAmount(_amount.abs())
    {
        setPicture(Core::pixmap("exchange-currency"));
        setTitle(tr("Currency Exchange"));
        setWindowTitle(title());

        QLabel* lblFrom = new QLabel(m_curFrom->formatAmount(m_absAmount), this);
        QLabel* lblDetails = new QLabel(tr("Transfer from %1 to %2").arg(m_curFrom->name()).arg(m_curTo->name()), this);

        QHBoxLayout* layAmount = new QHBoxLayout();
        QHBoxLayout* layRate = new QHBoxLayout();

        m_chkSaveRate = new QCheckBox(tr("&Save Rate"), this);
        addButton(m_chkSaveRate, 0, AtLeft);

        m_optRate = new QRadioButton(tr("Specify &Rate:"), this);
        m_txtRate = new QDoubleSpinBox(this);

        m_optAmount = new QRadioButton(tr("To &Amount:"), this);
        m_txtAmount = new AmountEdit(m_curTo->precision(), this);

        m_lblRateFromTo = new QLabel(this);
        m_lblRateToFrom = new QLabel(this);

        m_txtRate->setMinimum(0);
        m_txtRate->setMaximum(qInf());
        m_txtRate->setDecimals(PriceManager::DECIMALS_RATE);

        m_txtRate->setFixedWidth(300);
        m_txtAmount->setFixedWidth(300);


        layRate->addWidget(m_txtRate);
        layRate->addWidget(m_lblRateFromTo);
        layAmount->addWidget(m_txtAmount);
        layAmount->addWidget(m_lblRateToFrom);

        QFormLayout* layout = new QFormLayout(centralWidget());

        layout->addRow(lblDetails);
        layout->addRow(tr("From:"), lblFrom);
        layout->addRow(m_optRate, layRate);
        layout->addRow(m_optAmount, layAmount);

        connect(m_optRate, SIGNAL(toggled(bool)), this, SLOT(useRate(bool)));
        connect(m_txtAmount, SIGNAL(textChanged(QString)), this, SLOT(amountChanged()));
        connect(m_txtRate, SIGNAL(valueChanged(double)), this, SLOT(rateChanged()));

        m_optRate->setChecked(true);
        m_txtRate->setValue(PriceManager::instance()->rate(m_curFrom->code(),
                                                           m_curTo->code(),
                                                           _date));
        useRate(true);
        amountChanged();
    }

    void FormCurrencyExchange::setToAmount(const Amount& _amount)
    {
        m_optAmount->setChecked(true);
        m_txtAmount->setAmount(_amount);
    }

    Amount FormCurrencyExchange::exchangedAmount() const
    {
        if (m_optAmount->isChecked())
        {
            return m_amount < 0 ? -m_txtAmount->amount() : m_txtAmount->amount();
        }
        else
        {
            return m_amount * m_txtRate->value();
        }
    }

    void FormCurrencyExchange::accept()
    {
        if ((m_txtRate->isEnabled() && m_txtRate->value() == 0)
            || (m_txtAmount->isEnabled() && m_txtAmount->amount() == 0))
        {
            QMessageBox::information(this,
                                     this->windowTitle(),
                                     tr("Enter a non-zero rate or amount."));

            if (m_txtRate->isEnabled())
            {
                m_txtRate->setFocus();
            }
            else
            {
                m_txtAmount->setFocus();
            }

            return;
        }

        if (m_chkSaveRate->isChecked())
        {
            //Need to save the rate
            PriceManager::instance()->getOrAdd(m_curFrom->code(),
                                               m_curTo->code())->set(m_date.isValid() ? m_date : QDate::currentDate(),
                                                                     m_txtRate->value());
        }

        done(QDialog::Accepted);
    }

    void FormCurrencyExchange::useRate(bool _use)
    {
        m_txtRate->setEnabled(_use);
        m_txtAmount->setEnabled(!_use);

        if (_use)
        {
            m_txtRate->setFocus();
        }
        else
        {
            m_txtAmount->setFocus();
        }
    }

    void FormCurrencyExchange::amountChanged()
    {
        if (m_optAmount->isChecked())
            m_txtRate->setValue(m_txtAmount->doubleValue()/m_absAmount.toDouble());

        m_lblRateFromTo->setText(tr("1 %1 = %2 %3").arg(m_curFrom->code())
                                                   .arg(m_txtRate->value())
                                                   .arg(m_curTo->code()));
        m_lblRateToFrom->setText(tr("1 %1 = %2 %3").arg(m_curTo->code())
                                                   .arg(1.0/m_txtRate->value())
                                                   .arg(m_curFrom->code()));
    }

    void FormCurrencyExchange::rateChanged()
    {
        if (m_optRate->isChecked())
            m_txtAmount->setAmount(m_absAmount*m_txtRate->value());

        m_lblRateFromTo->setText(tr("1 %1 = %2 %3").arg(m_curFrom->code())
                                                   .arg(m_txtRate->value())
                                                   .arg(m_curTo->code()));
        m_lblRateToFrom->setText(tr("1 %1 = %2 %3").arg(m_curTo->code())
                                                   .arg(1.0/m_txtRate->value())
                                                   .arg(m_curFrom->code()));
    }

}

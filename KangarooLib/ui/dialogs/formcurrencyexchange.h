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

#ifndef FORMCURRENCYEXCHANGE_H
#define FORMCURRENCYEXCHANGE_H

#include "camsegdialog.h"
#include "../../amount.h"
#include <QDate>

class QRadioButton;
class QDoubleSpinBox;
class QLabel;
class QCheckBox;

namespace KLib
{
    class AmountEdit;
    class Currency;

    class FormCurrencyExchange : public CAMSEGDialog
    {
        Q_OBJECT

        public:
            explicit FormCurrencyExchange(const QString& _from,
                                          const QString& _to,
                                          const Amount& _amount,
                                          const QDate& _date,
                                          QWidget *parent = 0);

            Amount exchangedAmount() const;

            void setToAmount(const Amount& _amount);

        public slots:
            void accept();

        private slots:
            void useRate(bool _use);
            void amountChanged();
            void rateChanged();

        private:
            const Currency* m_curFrom;
            const Currency* m_curTo;
            const Amount m_amount;
            const QDate m_date;
            const Amount m_absAmount;

            QDoubleSpinBox* m_txtRate;
            AmountEdit* m_txtAmount;
            QRadioButton* m_optRate;
            QRadioButton* m_optAmount;
            QLabel* m_lblRateFromTo;
            QLabel* m_lblRateToFrom;
            QCheckBox* m_chkSaveRate;


    };

}

#endif // FORMCURRENCYEXCHANGE_H

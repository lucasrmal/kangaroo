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

#ifndef FORMDISTRIBUTIONCOMPOSITION_H
#define FORMDISTRIBUTIONCOMPOSITION_H

#include "../../model/investmenttransaction.h"
#include "camsegdialog.h"

class QLabel;

namespace KLib
{
    class AmountEdit;

    class FormDistributionComposition : public CAMSEGDialog
    {
        Q_OBJECT

        public:
            FormDistributionComposition(DistribComposition& _comp, QWidget* _parent = nullptr);

            void accept();

            static const int DISTR_TYPES_COUNT = 3;

        private slots:
            void updateTotal();

        private:
            DistribComposition& m_comp;

            AmountEdit* m_composition[DISTR_TYPES_COUNT];
            QLabel* m_lblTotal;

            void computeTotal(Amount& _total) const;


    };

}

#endif // FORMDISTRIBUTIONCOMPOSITION_H

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

#ifndef SPLITFRACTIONWIDGET_H
#define SPLITFRACTIONWIDGET_H

#include <QWidget>

class QSpinBox;

namespace KLib
{
    class SplitFractionWidget : public QWidget
    {
        Q_OBJECT

        public:
            explicit SplitFractionWidget(QWidget *parent = nullptr);

            QPair<int, int> splitFraction() const;

            void setSplitFraction(const QPair<int, int>& _frac);

            static const int MIN_FRAC = 1;
            static const int MAX_FRAC = 100;

        private:
            QSpinBox* m_spinFirst;
            QSpinBox* m_spinSecond;
    };

}

#endif // SPLITFRACTIONWIDGET_H

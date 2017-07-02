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

#include "splitfractionwidget.h"

#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>

namespace KLib
{

    SplitFractionWidget::SplitFractionWidget(QWidget *parent) : QWidget(parent)
    {
        m_spinFirst = new QSpinBox(this);
        m_spinSecond = new QSpinBox(this);

        QHBoxLayout* layout = new QHBoxLayout(this);
        QLabel* sep = new QLabel("for", this);
        sep->setFixedWidth(16);
        sep->setAlignment(Qt::AlignCenter);

        layout->addWidget(m_spinFirst);
        layout->addWidget(sep);
        layout->addWidget(m_spinSecond);
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);

        m_spinFirst->setMinimum(MIN_FRAC);
        m_spinSecond->setMinimum(MIN_FRAC);
        m_spinFirst->setMaximum(MAX_FRAC);
        m_spinSecond->setMaximum(MAX_FRAC);
    }

    QPair<int, int> SplitFractionWidget::splitFraction() const
    {
        return QPair<int, int>(m_spinFirst->value(), m_spinSecond->value());
    }

    void SplitFractionWidget::setSplitFraction(const QPair<int, int>& _frac)
    {
        auto checkBounds = [] (int _val) {
            return _val >= SplitFractionWidget::MIN_FRAC && _val <= SplitFractionWidget::MAX_FRAC;
        };

        if (checkBounds(_frac.first) && checkBounds(_frac.second))
        {
            m_spinFirst->setValue(_frac.first);
            m_spinSecond->setValue(_frac.second);
        }
    }

}


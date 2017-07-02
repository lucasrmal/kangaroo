/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2008-2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
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

/**
  @file  frmdaterange.cpp
  This file contains the method definitions for the frmDateRange dialog
  
  @author   Lucas Rioux Maldague
  @date     June 11th 2010
  @version  3.0

*/

#include "frmdaterange.h"
#include "../core.h"

#include <QVBoxLayout>
#include <QMessageBox>
#include <QGroupBox>
#include <QDateEdit>

namespace KLib {

    frmDateRange::frmDateRange(const QDate& p_startDate, const QDate& p_endDate, QWidget* parent) :
            CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
            m_startDate(p_startDate),
            m_endDate(p_endDate)
    {
        loadUI();
    }
    
    bool frmDateRange::selectDateRange(QWidget* parent, QDate* p_start, QDate* p_end, const QDate p_minDate, const QDate p_maxDate)
    {
        bool returnValue = false;
        frmDateRange* dlDateRange = new frmDateRange(*p_start, *p_end, parent);
        
        if (p_minDate.isValid())
            dlDateRange->setMinDate(p_minDate);
            
        if (p_maxDate.isValid())
            dlDateRange->setMaxDate(p_maxDate);
            
        if (dlDateRange->exec() == QDialog::Accepted)
        {
            returnValue = true;
            
            *p_start = dlDateRange->startDate();
            *p_end = dlDateRange->endDate();
        }

        delete dlDateRange;
        return returnValue;
    }

    void frmDateRange::loadUI()
    {
        setBothTitles(tr("Select Date Range"));
        setPicture(Core::pixmap("daterange"));

        grbStart = new QGroupBox(tr("&Start Date"), this);
        grbEnd   = new QGroupBox(tr("&End Date"), this);

        dateStart = new QDateEdit(m_startDate, this);
        dateEnd   = new QDateEdit(m_endDate, this);

        QVBoxLayout* mainLayout = new QVBoxLayout();
        QVBoxLayout* layoutStart = new QVBoxLayout(grbStart);
        QVBoxLayout* layoutEnd = new QVBoxLayout(grbEnd);

        grbStart->setCheckable(true);
        grbEnd->setCheckable(true);

        grbStart->setChecked(m_startDate.isValid());
        grbEnd->setChecked(m_endDate.isValid());

        dateStart->setCalendarPopup(true);
        dateEnd->setCalendarPopup(true);        

        if ( ! m_startDate.isValid())
            dateStart->setDate(QDate::currentDate());
            
        if ( ! m_endDate.isValid())
            dateEnd->setDate(QDate::currentDate());

        layoutStart->addWidget(dateStart);
        layoutEnd->addWidget(dateEnd);

        mainLayout->addWidget(grbStart);
        mainLayout->addWidget(grbEnd);

        centralWidget()->setLayout(mainLayout);
    }

    void frmDateRange::accept()
    {
        //Start date > min date
        if (grbStart->isChecked() && m_minDate.isValid() && dateStart->date() < m_minDate)
        {
            QMessageBox::information(this, this->windowTitle(), tr("The start date must be after %1.").arg(m_minDate.toString(Qt::DefaultLocaleShortDate)));
            dateStart->setFocus();
            return;
        }
        
        //End date < max date
        if (grbEnd->isChecked() && m_maxDate.isValid() && dateEnd->date() > m_maxDate)
        {
            QMessageBox::information(this, this->windowTitle(), tr("The end date must be before %1.").arg(m_maxDate.toString(Qt::DefaultLocaleShortDate)));
            dateEnd->setFocus();
            return;
        }
        
        //End date > start date
        if (grbStart->isChecked() && grbEnd->isChecked() && dateStart->date() > dateEnd->date())
        {
            QMessageBox::information(this, this->windowTitle(), tr("The end date must be after the start date."));
            dateEnd->setFocus();
            return;
        }
               
        //Set dates
        m_startDate = grbStart->isChecked() ? dateStart->date() : QDate();
        m_endDate   = grbEnd->isChecked()   ? dateEnd->date()   : QDate();

        //Close the dialog
        done(QDialog::Accepted);
    }
    
} //Namespace




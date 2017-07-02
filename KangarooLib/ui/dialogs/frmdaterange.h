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
  @file  frmdaterange.h
  This file contains the method declarations for the frmDateRange dialog
  
  @author   Lucas Rioux Maldague
  @date     June 11th 2010
  @version  3.0

*/

#ifndef FRMDATERANGE_H
#define FRMDATERANGE_H

#include "camsegdialog.h"
#include <QDate>

class QDateEdit;
class QGroupBox;

namespace KLib {

    /**
      The frmDateRange dialog provides a dialog to get a range of dates. The 
      range may have a minimum or a maximum date, and it can also be empty. If empty, 
      startDate() and endDate() will return an invalid date. If one of these methods returns an 
      invalid date, it means there is no start or end date, depending on which function you 
      call. If a minimum or a maximum is set, the start and end dates will respectively have to 
      be set.
      
      A convenience static method is also provided, selectDateRange(). It's the recommended way
      of selecting a date range.
      
      @brief Date range selector
      @see selectDateRange()
    */
    class frmDateRange : public CAMSEGDialog
    {
        Q_OBJECT

        public:
            /**
              Creates a new date range dialog
              
              @param[in] p_startDate The default start date
              @param[in] p_endDate The default end date
              @param[in] parent The dialog's parent
            */
            frmDateRange(const QDate& p_startDate, const QDate& p_endDate, QWidget* parent = NULL);
            
            /**
              @return The start date, or an invalid date if no start date is selected.
            */
            QDate   startDate() const               { return m_startDate; }
            
            /**
              @return If a start date has been selected.
            */
            bool    hasStartDate() const            { return m_startDate.isValid(); }
            
            /**
              @return The end date, or an invalid date if no end date is selected.
            */
            QDate   endDate() const                 { return m_startDate; }
            
            /**
              @return If an end date has been selected.
            */
            bool    hasEndDate() const              { return m_endDate.isValid(); }
            
            /**
              Sets the minimum start date. If a minimum date is defined, the start date will have
              to be selected. If p_date is invalid, the minimum date is removed.
              
              @param[in] p_date The minimum start date
            */
            void    setMinDate(const QDate& p_date) { m_minDate = p_date; }
            
            /**
              @return The minimum start date, or an invalid date if no minimum date is defined.
            */
            QDate   minDate() const                 { return m_minDate; }
           
            /**
              @return If a minimum start date is defined.
            */
            bool    hasMinDate() const              { return m_minDate.isValid(); }
            
            /**
              Sets the maximum end date. If a maximum date is defined, the end date will have
              to be selected. If p_date is invalid, the maximum date is removed.
              
              @param[in] p_date The maximum end date
            */
            void    setMaxDate(const QDate& p_date) { m_maxDate = p_date; }
            
            /**
              @return The maximum end date, or an invalid date if no maximum date is defined.
            */
            QDate   maxDate() const                 { return m_maxDate; }
            
            /**
              @return If a maximum end date is defined.
            */
            bool    hasMaxDate() const              { return m_maxDate.isValid(); }            
            
            /**
              Convenience function that acts as a wrapper over the class. It's the recommended way
              of selecting a date range.
              
              @param[in] parent The dialog's parent
              @param[in] p_start The default start date
              @param[in] p_end The default end date
              @param[in] p_minDate The minimum start date or an invalid date if no minimum date is desired.
              @param[in] p_maxDate The maximum end date or an invalid date if no maximum date is desired.
              
              @return True if the user accepted the dialog, or false if he rejected it.
            */
            static bool selectDateRange(QWidget* parent, QDate* p_start, QDate* p_end, const QDate p_minDate = QDate(), const QDate p_maxDate = QDate());
                       
        public slots:
            void accept();

        private:
            void loadUI();
            
            QGroupBox*  grbStart;
            QGroupBox*  grbEnd;

            QDateEdit*  dateStart;
            QDateEdit*  dateEnd;

            QDate       m_startDate;
            QDate       m_endDate;
            
            QDate       m_minDate;
            QDate       m_maxDate;
    };
    
} //Namespace

#endif // FRMDATERANGE_H

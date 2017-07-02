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
  @file  amountedit.h
  This file contains the method declarations for the AmountEdit class

  @author   Lucas Rioux Maldague
  @date     November 24th 2009
  @version  3.0
*/

#ifndef AMOUNTEDIT_H
#define AMOUNTEDIT_H

#include <QWidget>
#include "../../amount.h"

class QPushButton;
class QDoubleValidator;
class QLineEdit;
class QFrame;

namespace KLib {

    class Calculator;

    /**
      AmountEdit is a widget designed to easily edit amounts and quantities. It provides a textbox and a button which
      pops up a calculator. The calculator can also be opened with F12.
    */
    class AmountEdit : public QWidget
    {
        Q_OBJECT

        public:
        
            /**
              Creates a new AmountEdit with 2 decimals.
              
              @param[in] parent The widget's parent
            */
            explicit            AmountEdit(QWidget* parent = NULL);
            
            /**
              Creates a new AmountEdit
              
              @param[in] p_nbDecimals The number of decimals
              @param[in] parent The widget's parent
            */
            explicit            AmountEdit(quint8 p_nbDecimals, QWidget* parent = NULL);

            /**
              @return The minimum value of the amount (default is 0).
              @see setMinimum()
            */
            double              minimum() const                                 { return m_minValue; }
            
            /**
              Sets the minimum value of the amount.
              @see minimum()
            */
            void                setMinimum(const double p_amount);
            void                setMinimum(const Amount p_amount) { setMinimum(p_amount.toDouble()); }
            
            /**
              @return The maximum value of the amount (default is 1 000 000 000).
              @see setMaximum()
            */
            double              maximum() const                                 { return m_maxValue; }
            
            /**
              Sets the maximum value of the amount.
              @see maximum()
            */
            void                setMaximum(const double p_amount);
            void                setMaximum(const Amount& p_amount) { setMaximum(p_amount.toDouble()); }

            /**
              Sets the minimum and maximum values of the amount.
              @see setMinimum()
              @see setMaximum()
            */
            void                setRange(const double p_min, const double p_max);
            
            /**
              @return The precision (number of decimals) of the amount (default is 2).
              @see setPrecision()
            */
            quint8              precision()                                     { return m_prec; }
            
            /**
              Sets the precision of the amount (number of decimals)
              @see precision()
            */
            void                setPrecision(const quint8 p_precision);
            
            /**
              @return If the widget is in read-only mode (default is false).
              @see setReadOnly()
            */
            bool                isReadOnly() const;
            
            /**
              Sets if the widget is in read-only mode.
              @see isReadOnly()
            */
            void                setReadOnly(const bool p_readOnly);

            /**
              @return If the calculator button is visible (default is false).
              @see setCalculatorButtonVisible()
            */
            bool                isCalculatorButtonVisible() const;
            
            /**
              Sets the visibility of the calculator button.
              @see isCalculatorButtonVisible()
            */
            void                setCalculatorButtonVisible(const bool p_show);
            
            /**
              @return If an empty value ("") is allowed (default is false). If you call amount(), quantity(), doubleValue()
              or intValue(), they will return 0. If empty is not allowed and the user erases the text in the widget,
              it will be replaced by 0.
              @see setAllowEmpty()
            */
            bool                isEmptyAllowed() const                          { return m_allowEmpty; }
            
            /**
              Sets if an empty value is allowed.
              @see isEmptyAllowed()
            */
            void                setAllowEmpty(bool p_allowed)                   { m_allowEmpty = p_allowed; }
            
            /**
              @return The current prefix (displayed before the amount)
              @see setPrefix()
            */
            QString             prefix() const                                  { return m_prefix; }
            
            /**
              Sets the current prefix.
              @see prefix()
            */
            void                setPrefix(const QString & p_prefix)             { m_prefix = p_prefix; }
            
            /**
              @return The current suffix (displayed after the amount)
              @see setSuffix()
            */
            QString             suffix() const                                  { return m_suffix; }
            
            /**
              Sets the current suffix.
              @see suffix()
            */
            void                setSuffix(const QString & p_suffix)             { m_suffix = p_suffix; }
            
            QString             cleanText() const;

            /**
              @return The current amount
            */
            Amount     	 		amount() const;
            
            /**
              @return The current amount as a double
            */
            double              doubleValue() const;
            
            /**
              @return The current amount rounded to the nearest int.
            */
            int                 intValue() const;

            /**
              @return If the curent text is valid (in the range, not empty).
            */
            bool                isValid() const;
            
            QWidget*            focusWidget() const;
//            void                setFocus();

            /**
              @return The lineedit
            */
            QLineEdit*          lineedit() const                                { return m_lineEdit; }

    virtual bool                eventFilter(QObject*, QEvent*);

        protected:
            void                keyPressEvent(QKeyEvent* p_event);
            void                calculatorOpen(QKeyEvent* p_keyEvent);
            void                ensureFractionalPart();

        public slots:
            /**
              Loads the text p_text in the lineedit. The text is validated before it's added.
            */
            void                loadText(const QString& p_text);
            
            /**
              Clears the widget
            */
            void                clearText();

             /**
              Loads the text p_text in the lineedit. The text is validated before it's added.
            */
            void                setText(const QString& p_text) { setDoubleValue(p_text.toDouble()); }
            
            /**
              Sets the amount to p_value
            */
            void                setAmount(const Amount & p_value);
            
            /**
              Sets the amount to p_value
            */
            void                setDoubleValue(const double p_value);
            
            /**
              Sets the amount to p_value
            */
            void                setIntValue(const int p_value);

        protected slots:
            void                theTextChanged(const QString & p_text);
            void                slotCalculatorResult();
            void                slotCalculatorOpen();

        signals:
            /**
              Emitted when the value is changed
            */
            void                valueChanged(double p_newValue);
            
            /**
              Emitted when the text is changed
            */
            void                textChanged(const QString& text);

        private:
            void                loadUI();
        
            bool                cursorIsBeforeDecimalSeparator() const;
            void                ensureFractionalPart(QString& txt) const;

            double              m_minValue;
            double              m_maxValue;

            QString             m_prefix;
            QString             m_suffix;

            QLineEdit*          m_lineEdit;
            QPushButton*        m_btnShowCalculator;
            Calculator*         m_calculator;
            QFrame*             m_calculatorFrame;

            QDoubleValidator*   m_validator;

            bool                m_allowEmpty;
            quint8              m_prec;

            QString             previousText; // keep track of what has been typed
            QString             m_text;       // keep track of what was the original value
    };

} //Namespace

#endif // AMOUNTEDIT_H

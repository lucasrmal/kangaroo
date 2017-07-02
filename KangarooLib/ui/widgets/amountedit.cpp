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
  @file  amountedit.cpp
  This file contains the method definitions for the AmountEdit class

  @author   Lucas Rioux Maldague
  @date     November 24th 2009
  @version  3.0
*/

#include "amountedit.h"
#include "../core.h"

#include <QKeyEvent>
#include <QDoubleValidator>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLocale>
#include <QHBoxLayout>
#include <QApplication>

#include "../mainwindow.h"
#include "calculator.h"

namespace KLib {

    AmountEdit::AmountEdit(QWidget* parent) :
        QWidget(parent),
        m_minValue(0),
        m_maxValue(qInf()),
        m_allowEmpty(false),
        m_prec(2)
    {
        loadUI();
    }
    
    AmountEdit::AmountEdit(quint8 p_nbDecimals, QWidget* parent) :
        QWidget(parent),
        m_minValue(0),
        m_maxValue(qInf()),
        m_allowEmpty(false),
        m_prec(p_nbDecimals)
    {
        loadUI();
    }
    
    void AmountEdit::loadUI()
    {
        m_lineEdit = new QLineEdit(this);

        m_calculatorFrame = new QFrame(this);
        m_calculatorFrame->setWindowFlags(Qt::Popup);
        m_calculatorFrame->setFrameStyle(QFrame::Panel);
        m_calculatorFrame->setLineWidth(3);

        m_calculator = new Calculator(m_calculatorFrame);
        m_calculator->setVisible(false);
        m_calculatorFrame->setFixedSize(m_calculator->sizeHint().width()+3, m_calculator->sizeHint().height()+3);
        m_calculatorFrame->hide();

        m_btnShowCalculator = new QPushButton(Core::pixmap("calc"), "", this);
        m_btnShowCalculator->setFocusProxy(m_lineEdit);

        m_validator = new QDoubleValidator(this);
        m_validator->setBottom(m_minValue);
        m_validator->setTop(m_maxValue);
        m_lineEdit->setValidator(m_validator);

        m_validator->setDecimals(m_prec);
        m_validator->setNotation(QDoubleValidator::StandardNotation);
        
        m_lineEdit->setAlignment(Qt::AlignRight);
        m_lineEdit->installEventFilter(this);

        m_btnShowCalculator->setFixedWidth(m_btnShowCalculator->sizeHint().height());
        m_btnShowCalculator->setFixedHeight(m_lineEdit->sizeHint().height());

        QString deftext = "0.";

        for (quint8 i = 0; i < m_prec; ++i)
            deftext += "0";

        if (m_prec == 0)
            deftext = "0";

        m_text = deftext;
        m_lineEdit->setText(deftext);

        //Layout
        QHBoxLayout* mainLayout = new QHBoxLayout(this);
        mainLayout->setMargin(0);
        mainLayout->setSpacing(0);
        mainLayout->addWidget(m_lineEdit);
        mainLayout->addWidget(m_btnShowCalculator);

        //Connexions
        connect(m_lineEdit, SIGNAL(textChanged(const QString&)), this, SLOT(theTextChanged(const QString&)));
        connect(m_btnShowCalculator, SIGNAL(clicked()), this, SLOT(slotCalculatorOpen()));
        connect(m_calculator, SIGNAL(resultAvailable()), this, SLOT(slotCalculatorResult()));
        connect(m_calculator, SIGNAL(pleaseClose()), m_calculatorFrame, SLOT(hide()));

        setFocusPolicy(Qt::StrongFocus);
        setFocusProxy(m_lineEdit);
    }

    void AmountEdit::slotCalculatorResult()
    {
        if (m_calculator)
        {
            m_calculator->setVisible(false);
            m_calculatorFrame->hide();
            m_lineEdit->setText(QString::number(m_calculator->result().toDouble(), 'f', m_prec));
            ensureFractionalPart();
            emit valueChanged(m_lineEdit->text().toDouble());
            m_text = m_lineEdit->text();
        }
    }

    void AmountEdit::slotCalculatorOpen()
    {
        calculatorOpen(NULL);
    }

    void AmountEdit::calculatorOpen(QKeyEvent* k)
    {
        Q_UNUSED(k)

        if (m_lineEdit->isReadOnly())
            return;

        m_calculator->setInitialValue(QString::number(locale().toDouble(m_lineEdit->text())));

        int h = m_calculatorFrame->height();
        int w = m_calculatorFrame->width();

        // usually, the calculator widget is shown underneath the MoneyEdit widget
        // if it does not fit on the screen, we show it above this widget
        QPoint p = mapToGlobal(QPoint(0,0));
        if(p.y() + height() + h > QApplication::desktop()->height())
            p.setY(p.y() - h);
        else
            p.setY(p.y() + height());

        // usually, it is shown left aligned. If it does not fit, we align it
        // to the right edge of the widget
        if(p.x() + w > QApplication::desktop()->width())
            p.setX(p.x() + width() - w);

        QRect r = m_calculator->geometry();
        r.moveTopLeft(p);
        m_calculatorFrame->setGeometry(r);
        m_calculatorFrame->show();
        m_calculator->setVisible(true);
        m_calculator->setFocus();

        if (k && k->type() == QEvent::KeyPress)
        {
            m_calculator->pressKey(static_cast<QKeyEvent*>(k));
        }
    }

    QString AmountEdit::cleanText() const
    {
        return m_lineEdit->text();
    }

    Amount AmountEdit::amount() const
    {
        return Amount::fromUserLocale(m_lineEdit->text(), m_prec);
    }

    double AmountEdit::doubleValue() const
    {
        return m_lineEdit->text().toDouble();
    }

    int AmountEdit::intValue() const
    {
        return Amount(m_lineEdit->text()).roundToInt();
    }

    void AmountEdit::setMinimum(const double p_amount)
    {
        m_minValue = p_amount;
        m_validator->setBottom(p_amount);
    }

    void AmountEdit::setMaximum(const double p_amount)
    {
        m_maxValue = p_amount;
        m_validator->setTop(p_amount);
    }

    void AmountEdit::setRange(const double p_min, const double p_max)
    {
        setMinimum(p_min);
        setMaximum(p_max);
    }

    void AmountEdit::keyPressEvent(QKeyEvent* event)
    {
        if (event->key() == Qt::Key_F12)
        {
            slotCalculatorOpen();
        }
        else
        {
            //event->ignore();
            QWidget::keyPressEvent(event);
        }
    }

    void AmountEdit::theTextChanged(const QString & theText)
    {
        QLocale l;
        QString d = l.decimalPoint();
        QString l_text = theText;
        QString nsign, psign;

        nsign = l.negativeSign();
        psign = l.positiveSign();

        int i = 0;

        if (isEnabled())
        {
            QValidator::State state =  m_lineEdit->validator()->validate( l_text, i);

            if (state == QValidator::Intermediate)
            {
                if (l_text.length() == 1)
                {
                    if(l_text != d && l_text != nsign && l_text != psign)
                        state = QValidator::Invalid;
                    else
                        emit textChanged(m_lineEdit->text());
                }
            }
            if (state==QValidator::Invalid)
            {
                m_lineEdit->setText(previousText);
            }
            else
            {
                previousText = l_text;
                emit textChanged(m_lineEdit->text());
            }
        }
    }

    void AmountEdit::setAmount(const Amount & p_value)
    {
        loadText(p_value.toString());
        emit valueChanged(p_value.toDouble());
    }

    void AmountEdit::setDoubleValue(const double p_value)
    {
        loadText(QString::number(p_value, 'f', m_prec));
        emit valueChanged(p_value);
    }

    void AmountEdit::setIntValue(const int p_value)
    {
        loadText(QString::number(p_value));
        emit valueChanged((double) p_value);
    }

    void AmountEdit::loadText(const QString& txt)
    {
      m_lineEdit->setText(txt);

      ensureFractionalPart();

      m_text = m_lineEdit->text();
    }

    void AmountEdit::clearText(void)
    {
      m_text.clear();
      m_lineEdit->setText(m_text);
    }

    QWidget* AmountEdit::focusWidget() const
    {
      QWidget* w = m_lineEdit;
      while(w->focusProxy())
        w = w->focusProxy();
      return w;
    }

//    void AmountEdit::setFocus()
//    {
//        m_lineEdit->selectAll();
//        m_lineEdit->setFocus();
//    }

    void AmountEdit::setCalculatorButtonVisible(const bool show)
    {
        m_btnShowCalculator->setVisible(show);
    }

    bool AmountEdit::isCalculatorButtonVisible() const
    {
        return m_btnShowCalculator->isVisible();
    }

    bool AmountEdit::isReadOnly() const
    {
        return m_lineEdit ? m_lineEdit->isReadOnly() : false;
    }

    void AmountEdit::setReadOnly(const bool readOnly)
    {
        if (m_lineEdit)
        {
            m_lineEdit->setReadOnly(readOnly);
            m_btnShowCalculator->setEnabled(false);
        }
    }

    void AmountEdit::setPrecision(const quint8 prec)
    {
        if (prec <= 20 && prec != m_prec)
        {
            m_validator->setDecimals(prec);
            m_prec = prec;

            // update current display
            setDoubleValue(doubleValue());
        }
    }

    bool AmountEdit::isValid() const
    {
        return !(m_lineEdit->text().isEmpty());
    }

    bool AmountEdit::eventFilter(QObject* _object, QEvent* _event)
    {
        // we want to catch some keys that are usually handled by
        // the base class (e.g. '+', '-', etc.)
        if(_event->type() == QEvent::KeyPress)
        {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *> (_event);

            switch (keyEvent->key())
            {
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                QApplication::postEvent(this, new QKeyEvent(keyEvent->type(), keyEvent->key(), keyEvent->modifiers()));
                return true;

            case Qt::Key_Plus:
            case Qt::Key_Minus:
            case Qt::Key_Slash:
            case Qt::Key_Asterisk:
            case Qt::Key_Percent:
                calculatorOpen(keyEvent);
                return true;
            }

        }
        else if(_event->type() == QEvent::FocusOut)
        {
            if(!m_lineEdit->text().isEmpty() || !m_allowEmpty)
            {
                ensureFractionalPart();
            }

            if(Amount(m_lineEdit->text().toDouble()) != Amount(m_text.toDouble())
                && !m_calculator->isVisible())
            {
                emit valueChanged(m_lineEdit->text().toDouble());
            }

            m_text = m_lineEdit->text();

            QFocusEvent* focusEvent = static_cast<QFocusEvent *>(_event);
            QApplication::postEvent(this, new QFocusEvent(focusEvent->type(), focusEvent->reason()));
            return false;
        }

        return QWidget::eventFilter(_object, _event);
    }

    bool AmountEdit::cursorIsBeforeDecimalSeparator() const
    {
        //Find the current decimal separator position
        int position = m_lineEdit->text().indexOf('.');

        if (position >= 0)
        {
            return m_lineEdit->cursorPosition() + 1 < position;
        }

        return false;
    }

    void AmountEdit::ensureFractionalPart()
    {
        QString s(m_lineEdit->text());
        ensureFractionalPart(s);
        m_lineEdit->setText(s);
    }

    void AmountEdit::ensureFractionalPart(QString& s) const
    {
        QLocale locale;
        QString decimalSymbol = locale.decimalPoint();

        if (decimalSymbol.isEmpty())
            decimalSymbol = '.';

        // If text contains no 'monetaryDecimalSymbol' then add it
        // followed by the required number of 0s
        if (!s.isEmpty())
        {
            if (m_prec > 0)
            {
                if (!s.contains(decimalSymbol))
                {
                    s += decimalSymbol;

                    for (int i=0; i < m_prec; i++)
                        s += '0';
                }
                else
                {
                    //Check the required amount of decimals
                    int decimals = s.count() - s.indexOf(decimalSymbol) - 1;

                    for (int i = decimals; i < m_prec; ++i)
                        s += '0';

                }
            }
            else if (m_prec == 0)
            {
                while (s.contains(decimalSymbol))
                {
                    int pos = s.lastIndexOf(decimalSymbol);

                    if(pos != -1)
                    {
                        s.truncate(pos);
                    }
                }
            }
            else if (s.contains(decimalSymbol))
            {  // m_prec == -1 && fraction

                // no trailing zeroes
                while(s.endsWith('0'))
                {
                    s.truncate(s.length()-1);
                }

                // no trailing decimalSymbol
                if(s.endsWith(decimalSymbol))
                    s.truncate(s.length()-1);
            }
        }
    }
} //Namespace

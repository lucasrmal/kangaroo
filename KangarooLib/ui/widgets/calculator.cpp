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

#include "calculator.h"
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QKeyEvent>
#include <QSignalMapper>
#include <QMessageBox>

#include <muParser.h>

#define CONST_PI 3.141592653589793238462643383279502884

namespace KLib
{
    Calculator::Calculator(QWidget *parent) :
        QFrame(parent),
        m_curPos(0),
        m_justEvaluated(false),
        m_lastWasMRC(false)
    {
        loadUI();
        installEventFilter(this);
    }

    void Calculator::onButtonPressed(int _button)
    {
        switch (_button)
        {
        case Second:
            break;

        case Left:
            left();
            break;

        case Right:
            right();
            break;

        case Del:
            del();
            break;

        case Clear:
            clear();
            break;

        case SquareRoot:
            addChar("√");
            break;

        case Square:
            addChar("²");
            break;

        case OpenPar:
            addChar("(");
            break;

        case ClosePar:
            addChar(")");
            break;

        case Divided:
            addChar("÷");
            break;


        case PlusMinus:
            addChar("-");
            break;

        case Dig_7:
            addChar("7");
            break;

        case Dig_8:
            addChar("8");
            break;

        case Dig_9:
            addChar("9");
            break;

        case Times:
            addChar("x");
            break;

        case Dig_4:
            addChar("4");
            break;

        case Dig_5:
            addChar("5");
            break;

        case Dig_6:
            addChar("6");
            break;

        case Minus:
            addChar("-");
            break;

        case Dig_1:
            addChar("1");
            break;

        case Dig_2:
            addChar("2");
            break;

        case Dig_3:
            addChar("3");
            break;

        case Plus:
            addChar("+");
            break;

        case MRC:
            if (m_lastWasMRC)
            {
                memClear();
            }
            else
            {
                memShow();
                m_lastWasMRC = true;
            }
            return;

        case MPlus:
            memPlus();
            break;

        case MMinus:
            memMinus();
            break;

        case Dig_0:
            addChar("0");
            break;

        case Dot:
            addChar(".");
            break;

        case Pi:
            addChar("π");
            break;

        case Enter:
            evaluate();
            break;

        default:
            break;
        }

        m_lastWasMRC = false;
    }

    bool isDigit(const QChar& _str)
    {
        return _str == '0'
                || _str == '1'
                || _str == '2'
                || _str == '3'
                || _str == '4'
                || _str == '5'
                || _str == '6'
                || _str == '7'
                || _str == '8'
                || _str == '9';
    }

    void Calculator::evaluate()
    {
        using namespace mu;

        try
        {
            Parser p;
            QString temp;
            temp = m_query;
            temp.replace("π", "_pi");
            temp.replace("÷", "/");
            temp.replace("x", "*");
            temp.replace("²", "^2");
            temp.replace("√", "S");

            for (int i = 0; i < temp.count(); ++i)
            {
                if (temp[i] == 'S' && i+1 < temp.count())
                {
                    if (temp[i+1] != '(')
                    {
                        temp.insert(i+1, '(');
                        i = i+2; //Go after the parenthesis

                        while (i < temp.count() && isDigit(temp[i])) //While it's a digit
                        {
                            ++i;
                        }

                        temp.insert(i, ")");
                    }
                }
            }

            temp.replace("S", "sqrt");

            if (temp.isEmpty())
            {
                temp = "0";
            }

            p.DefineConst("_pi", (double)CONST_PI);
            p.SetExpr(temp.toStdString());

            m_query = QString::number(p.Eval(), 'f', 8);

            //Remove trailing zeros (if contains a .)
            while (m_query.contains('.')
                   && m_query.count()
                   && (m_query[m_query.count()-1] == '0'
                       || m_query[m_query.count()-1] == '.'))
            {
                m_query.chop(1);
            }

            if (m_query.isEmpty())
            {
                m_query = "0";
            }

            m_curPos = m_query.count();
            updateDisplay();
            m_justEvaluated = true;
            emit resultAvailable();
        }
        catch (Parser::exception_type &e)
        {
            QMessageBox::warning(this,
                                 tr("Evaluate Expression"),
                                 tr("Syntax Error: %1").arg(QString::fromStdString(e.GetMsg())));
        }
    }

    void Calculator::updateDisplay()
    {
        QString temp = m_query.left(m_curPos);

        temp += "<u>";

        if (m_query.count() > m_curPos)
        {
            temp += m_query[m_curPos];
        }
        else
        {
            temp += "&nbsp;";
        }

        temp += "</u>";

        temp += m_query.mid(m_curPos+1);

        m_lblDisplay->setText(temp);
    }

    void Calculator::addChar(const QString& _c)
    {
        m_justEvaluated = false;

        if (m_curPos == m_query.count())
        {
            m_query.append(_c);
        }
        else
        {
            m_query.insert(m_curPos, _c);
        }

        ++m_curPos;
        updateDisplay();
    }

    void Calculator::left()
    {
        if (m_curPos > 0)
        {
            --m_curPos;
        }
        updateDisplay();
    }

    void Calculator::right()
    {
        if (m_curPos < m_query.count())
        {
            ++m_curPos;
        }
        updateDisplay();
    }

    void Calculator::del()
    {
        /*if (m_curPos > 0 && m_curPos < m_query.count())
        {
            m_query.remove(m_curPos-1,1);
        }
        else */if (m_curPos > 0)
        {
            m_query.remove(m_curPos-1,1);
            --m_curPos;
        }

        updateDisplay();
    }

    void Calculator::clear()
    {
        m_justEvaluated = true;
        m_query.clear();
        updateDisplay();
    }

    void Calculator::begin()
    {
        m_curPos = 0;
        updateDisplay();
    }

    void Calculator::end()
    {
        m_curPos = m_query.count();
        updateDisplay();
    }

    void Calculator::memPlus()
    {
        //Evaluate the current answer if not done already
        if (!m_justEvaluated)
        {
            evaluate();
        }

        if (m_justEvaluated)
        {
            if (m_query.isEmpty())
            {
                m_memory = m_query;
            }
            else
            {
                m_memory += "+" + m_query;
            }
        }
    }

    void Calculator::memMinus()
    {
        //Evaluate the current answer if not done already
        if (!m_justEvaluated)
        {
            evaluate();
        }

        if (m_justEvaluated)
        {
            m_memory += "-" + m_query;
        }
    }

    void Calculator::memShow()
    {
        m_query = m_memory;
        evaluate();
        m_memory = m_query;
    }

    void Calculator::memClear()
    {
        m_memory.clear();
    }

    QString Calculator::result()
    {
        //Evaluate the current answer if not done already
        if (!m_justEvaluated)
        {
            evaluate();
        }

        return m_query;
    }

    void Calculator::setInitialValue(const QString& _value)
    {
        m_query = _value;
        evaluate();

        if (m_query == "0")
        {
            m_query = "";
            updateDisplay();
        }
    }

    void Calculator::pressKey(QKeyEvent* _event)
    {
        eventFilter(NULL, _event);
    }

    bool Calculator::eventFilter(QObject* _object, QEvent* _event)
    {
        Q_UNUSED(_object)

        if (_event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent *>(_event);

            switch (keyEvent->key())
            {
            case Qt::Key_Escape:
                emit pleaseClose();
                break;

            case Qt::Key_Return:
            case Qt::Key_Enter:
                onButtonPressed(Enter);
                break;

            case Qt::Key_Plus:
                onButtonPressed(Plus);
                break;

            case Qt::Key_Minus:
                onButtonPressed(Minus);
                break;

            case Qt::Key_Asterisk:
                onButtonPressed(Times);
                break;

            case Qt::Key_Slash:
                onButtonPressed(Divided);
                break;

            case Qt::Key_ParenLeft:
                onButtonPressed(OpenPar);
                break;

            case Qt::Key_ParenRight:
                onButtonPressed(ClosePar);
                break;

            case Qt::Key_Left:
                onButtonPressed(Left);
                break;

            case Qt::Key_Right:
                onButtonPressed(Right);
                break;

            case Qt::Key_Home:
                begin();
                break;

            case Qt::Key_End:
                end();
                break;

            case Qt::Key_Backspace:
                onButtonPressed(Del);
                break;

            case Qt::Key_Delete:
                onButtonPressed(Clear);
                break;

            case Qt::Key_Period:
                onButtonPressed(Dot);
                break;

            case Qt::Key_0:
                onButtonPressed(Dig_0);
                break;

            case Qt::Key_1:
                onButtonPressed(Dig_1);
                break;

            case Qt::Key_2:
                onButtonPressed(Dig_2);
                break;

            case Qt::Key_3:
                onButtonPressed(Dig_3);
                break;

            case Qt::Key_4:
                onButtonPressed(Dig_4);
                break;

            case Qt::Key_5:
                onButtonPressed(Dig_5);
                break;

            case Qt::Key_6:
                onButtonPressed(Dig_6);
                break;

            case Qt::Key_7:
                onButtonPressed(Dig_7);
                break;

            case Qt::Key_8:
                onButtonPressed(Dig_8);
                break;

            case Qt::Key_9:
                onButtonPressed(Dig_9);
                break;
            default:
                return false;
            }

            return true;
        }

        return false;
    }

    void Calculator::loadUI()
    {
        const int numColumns = 5;
        int curRow = 0, curColumn = 0;

        QGridLayout* grid = new QGridLayout();
        grid->setSpacing(0);

        QSignalMapper* mapper = new QSignalMapper(this);

        for (int i = 0; i < NumButtons; ++i, ++curColumn)
        {
            if (curColumn == numColumns)
            {
                curColumn = 0;
                ++curRow;
            }

            m_buttons[i] = new QPushButton(this);
            m_buttons[i]->setMinimumSize(QSize(20, 30));
            grid->addWidget(m_buttons[i], curRow, curColumn);

            mapper->setMapping(m_buttons[i], i);
            connect(m_buttons[i], SIGNAL(clicked()),
                    mapper, SLOT(map()));

            m_buttons[i]->installEventFilter(this);
        }

        connect(mapper, SIGNAL(mapped(int)),
                this, SLOT(onButtonPressed(int)));

        m_buttons[Second]->setText(tr("2nd"));
        m_buttons[Left]->setText(tr("<"));
        m_buttons[Right]->setText(tr(">"));
        m_buttons[Del]->setText(tr("DEL"));
        m_buttons[Clear]->setText(tr("CLR"));
        m_buttons[SquareRoot]->setText(tr("√"));
        m_buttons[Square]->setText(tr("x²"));
        m_buttons[OpenPar]->setText(tr("("));
        m_buttons[ClosePar]->setText(tr(")"));
        m_buttons[Divided]->setText(tr("÷"));
        m_buttons[PlusMinus]->setText(tr("+/-"));
        m_buttons[Dig_7]->setText(tr("7"));
        m_buttons[Dig_8]->setText(tr("8"));
        m_buttons[Dig_9]->setText(tr("9"));
        m_buttons[Times]->setText(tr("x"));
        m_buttons[MRC]->setText(tr("MRC"));
        m_buttons[Dig_4]->setText(tr("4"));
        m_buttons[Dig_5]->setText(tr("5"));
        m_buttons[Dig_6]->setText(tr("6"));
        m_buttons[Minus]->setText(tr("-"));
        m_buttons[MPlus]->setText(tr("M+"));
        m_buttons[Dig_1]->setText(tr("1"));
        m_buttons[Dig_2]->setText(tr("2"));
        m_buttons[Dig_3]->setText(tr("3"));
        m_buttons[Plus]->setText(tr("+"));
        m_buttons[MMinus]->setText(tr("M-"));
        m_buttons[Dig_0]->setText(tr("0"));
        m_buttons[Dot]->setText(tr("."));
        m_buttons[Pi]->setText(tr("π"));
        m_buttons[Enter]->setText(tr("ENTER"));

        QPalette buttonColor;
        buttonColor.setColor(QPalette::Button, Qt::red);
        m_buttons[Second]->setPalette(buttonColor);

        buttonColor.setColor(QPalette::Button, Qt::white);
        m_buttons[Dig_0]->setPalette(buttonColor);
        m_buttons[Dig_1]->setPalette(buttonColor);
        m_buttons[Dig_2]->setPalette(buttonColor);
        m_buttons[Dig_3]->setPalette(buttonColor);
        m_buttons[Dig_4]->setPalette(buttonColor);
        m_buttons[Dig_5]->setPalette(buttonColor);
        m_buttons[Dig_6]->setPalette(buttonColor);
        m_buttons[Dig_7]->setPalette(buttonColor);
        m_buttons[Dig_8]->setPalette(buttonColor);
        m_buttons[Dig_9]->setPalette(buttonColor);
        m_buttons[ClosePar]->setPalette(buttonColor);
        m_buttons[OpenPar]->setPalette(buttonColor);
        m_buttons[Dot]->setPalette(buttonColor);
        m_buttons[Pi]->setPalette(buttonColor);

        buttonColor.setColor(QPalette::Button, Qt::blue);
        m_buttons[Left]->setPalette(buttonColor);
        m_buttons[Right]->setPalette(buttonColor);

        buttonColor.setColor(QPalette::Button, QColor("#FFA858"));
        m_buttons[Plus]->setPalette(buttonColor);
        m_buttons[Minus]->setPalette(buttonColor);
        m_buttons[Times]->setPalette(buttonColor);
        m_buttons[Divided]->setPalette(buttonColor);
        m_buttons[Square]->setPalette(buttonColor);
        m_buttons[SquareRoot]->setPalette(buttonColor);
        m_buttons[PlusMinus]->setPalette(buttonColor);
    //    m_buttons[Square]->setPalette(buttonColor);

        buttonColor.setColor(QPalette::Button, Qt::blue);
        m_buttons[Left]->setPalette(buttonColor);
        m_buttons[Right]->setPalette(buttonColor);
        m_buttons[Del]->setPalette(buttonColor);
        m_buttons[Clear]->setPalette(buttonColor);
        m_buttons[Enter]->setPalette(buttonColor);

        buttonColor.setColor(QPalette::Button, QColor("#F1F14D"));
        m_buttons[Del]->setPalette(buttonColor);
        m_buttons[Clear]->setPalette(buttonColor);
        m_buttons[Enter]->setPalette(buttonColor);

        m_buttons[MRC]->setPalette(buttonColor);
        m_buttons[MPlus]->setPalette(buttonColor);
        m_buttons[MMinus]->setPalette(buttonColor);



        m_lblDisplay = new QLabel(this);
        m_lblDisplay->setFrameStyle(QFrame::Panel);
        m_lblDisplay->setFrameShadow(QFrame::Sunken);
        m_lblDisplay->setAutoFillBackground(true);
        m_lblDisplay->setMinimumHeight(40);
        m_lblDisplay->setMaximumHeight(60);

        QFont displayFont;
        displayFont.setPointSize(16);
        m_lblDisplay->setFont(displayFont);

        QPalette p;
        p.setColor(QPalette::Window, QColor("#99FF8E"));
        m_lblDisplay->setPalette(p);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(m_lblDisplay);
        layout->addLayout(grid);
        //layout->addStretch(1);

        setMaximumSize(270, 230);
    }

    QSize Calculator::sizeHint() const
    {
        return QSize(270, 230);
    }

} //Namespace




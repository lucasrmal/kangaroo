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

#ifndef CALCULATOR_H
#define CALCULATOR_H

#include <QFrame>

class QPushButton;
class QLabel;

namespace KLib
{

    class Calculator : public QFrame
    {
        Q_OBJECT

            enum Buttons
            {
                Second,
                Left,
                Right,
                Del,
                Clear,

                SquareRoot,
                Square,
                OpenPar,
                ClosePar,
                Divided,

                PlusMinus,
                Dig_7,
                Dig_8,
                Dig_9,
                Times,

                MRC,
                Dig_4,
                Dig_5,
                Dig_6,
                Minus,

                MPlus,
                Dig_1,
                Dig_2,
                Dig_3,
                Plus,

                MMinus,
                Dig_0,
                Dot,
                Pi,
                Enter,

                NumButtons
            };

        public:
            explicit Calculator(QWidget *parent = 0);

            QSize sizeHint() const;

            QString result();
            void setInitialValue(const QString& _value);

            void pressKey(QKeyEvent* _event);

        signals:
            void pleaseClose();
            void resultAvailable();

        protected:
            bool eventFilter(QObject* _object, QEvent* _event);

        private slots:
            void onButtonPressed(int _button);
            void updateDisplay();

        private:
            void loadUI();

            void evaluate();
            void addChar(const QString& _c);
            void left();
            void right();
            void del();
            void clear();
            void begin();
            void end();

            void memPlus();
            void memMinus();
            void memShow();
            void memClear();


            QLabel* m_lblDisplay;
            QPushButton* m_buttons[NumButtons];
            QString m_query;
            int m_curPos;

            QString m_memory;

            bool m_justEvaluated;
            bool m_lastWasMRC;
    };
}

#endif // CALCULATOR_H

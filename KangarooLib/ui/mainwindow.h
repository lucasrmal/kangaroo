/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2008 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
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
  @file  mainwindow.h
  This file contains the method declarations for the MainWindow class

  @author   Lucas Rioux Maldague
  @date     May 24th 2010
  @version  3.0

*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "dialogs/isaveposition.h"

class QLabel;
class QMenu;

namespace KLib
{
    class MainWindow : public QMainWindow, ISavePosition
    {
        Q_OBJECT
        
        public:
            explicit    MainWindow(QWidget *parent = NULL);
                        ~MainWindow();
            
            void        showStatusBar();

        protected:
            void        closeEvent(QCloseEvent* event);
            
        private:            
            void        createStatusBar();
            
            QLabel*     lblFileName;
    };

} //Namespace

#endif // MAINWINDOW_H

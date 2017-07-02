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
  @file  mainwindow.cpp
  This file contains the method definitions for the MainWindow class

  @author   Lucas Rioux Maldague
  @date     May 24th 2010
  @version  3.0

*/

#include "mainwindow.h"
#include "core.h"
#include "actionmanager/actionmanager.h"
#include "actionmanager/qttoolbardialog.h"

#include <QDate>
#include <QSettings>
#include <QCloseEvent>
#include <QLabel>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QStatusBar>
#include <QDesktopWidget>

namespace KLib
{
    MainWindow::MainWindow(QWidget *parent) :
        QMainWindow(parent),
        ISavePosition("MainWindow", this, QApplication::desktop()->availableGeometry().size(), QPoint(0, 0)),
        lblFileName(NULL)
    {
        //WindowTitle
        setWindowTitle(Core::APP_NAME);

//        if (QDate::currentDate().month() == 12 && QDate::currentDate().day() >= 10 && QDate::currentDate().day() < 26)
//        {
//            setWindowIcon(Core::icon("camseg_christmas"));
//        }
//        else
//        {
            setWindowIcon(Core::icon("kangaroo"));
//        }
        
        //State and position
        loadPosition();
    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::closeEvent(QCloseEvent* event)
    {
        if (!Core::instance()->canClose())
        {
            event->ignore();
            return;
        }

        savePosition();
        
        QSettings settings;
        settings.setValue("ToolBarState/Manager", ActionManager::toolBarManager()->saveState());
        Core::instance()->shutdownPlugins();
        
        event->accept();
    }
    
    void MainWindow::showStatusBar()
    {
        if ( !lblFileName)
            createStatusBar();
        
        statusBar()->show();

        
//        if ( !campaign)
//            return;
        
//        lblCampaignName->setText(QString("<strong>%1</strong> %2 %3").arg(campaign->getCampaignName())
//                                                                    .arg(tr("by"))
//                                                                    .arg(campaign->getOrganization()->name));
    }
    
    void MainWindow::createStatusBar()
    {
        lblFileName = new QLabel(statusBar());
        statusBar()->addWidget(lblFileName);
    }

} //Namespace

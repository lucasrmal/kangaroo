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

#include "dockwidgetmanager.h"
#include "actionmanager/actionmanager.h"
#include "actionmanager/actioncontainer.h"
#include "core.h"
#include "mainwindow.h"

#include <QAction>
#include <QDockWidget>
#include <QMenu>
#include <QSettings>

namespace KLib
{

    DockWidgetManager* DockWidgetManager::m_instance = NULL;

    DockWidgetManager* DockWidgetManager::instance()
    {
        if ( !m_instance)
            m_instance = new DockWidgetManager();

        return m_instance;
    }

    DockWidgetManager::~DockWidgetManager()
    {
        for (DockWidgetFactorySuper* s : m_docks)
        {
            delete s;
        }
    }

    void DockWidgetManager::registerDockWidget(const QString& _dockId,
                                           const QString& _actionText,
                                           const QIcon& _actionIcon,
                                           Qt::DockWidgetArea _area,
                                           bool _requiresFile,
                                           DockWidgetFactorySuper* _factory)
    {
        if (!m_docks.contains(_dockId) && _factory)
        {
            _factory->m_id = _dockId;
            _factory->m_requiresFile = _requiresFile;
            _factory->m_area = _area;
            _factory->m_action = ActionManager::instance()->
                                     actionContainer(STD_MENUS::MENU_VIEW_DOCKS)->
                                         menu()->addAction(_actionIcon,
                                                           _actionText,
                                                           this, SLOT(openDock()));

            ActionManager::instance()->registerAction(_factory->m_action, QString("View.Docks.%1").arg(_dockId));

            m_docksByAction[_factory->m_action] = _factory;
            m_docks[_dockId] = _factory;

            if (_requiresFile)
            {
                _factory->m_action->setEnabled(false);
            }
        }
    }

    void DockWidgetManager::openDock()
    {
        QAction* a = static_cast<QAction*>(sender());
        DockWidgetFactorySuper* sup = m_docksByAction[a];

        if (!sup->currentDock())
        {
            sup->m_current = sup->createDock(Core::instance()->mainWindow());

            Core::instance()->mainWindow()->addDockWidget(sup->m_area,
                                                          sup->currentDock());
        }
        else
        {
            sup->currentDock()->setVisible(true);
        }

    }


    void DockWidgetManager::onLoad()
    {

        for (DockWidgetFactorySuper* dock : m_docks)
        {
            if (dock->m_requiresFile)
            {
                dock->m_action->setEnabled(true);
            }
        }

        //Load the last state
        QSettings settings;
        QStringList ids = settings.value("DockWidgetManager/Ids").toStringList();
        QStringList statuses = settings.value("DockWidgetManager/Statuses").toStringList();

        for (int i = 0; i < std::min(ids.count(), statuses.count()); ++i)
        {
            if (statuses[i] == "Y")
            {
                DockWidgetFactorySuper* sup = m_docks[ids[i]];

                if (!sup->m_current)
                {
                    sup->m_current = sup->createDock(Core::instance()->mainWindow());

                    Core::instance()->mainWindow()->addDockWidget(sup->m_area,
                                                                  sup->currentDock());
                }
                else
                {
                    sup->m_current->setVisible(true);
                }

            }

        }
    }

    void DockWidgetManager::onUnload()
    {
        QStringList ids;
        QStringList statuses;

        for (DockWidgetFactorySuper* dock : m_docks)
        {
            ids << dock->m_id;
            statuses << QString(dock->m_current && dock->m_current->isVisible() ? "Y" : "N");

            if (dock->m_requiresFile)
            {
                dock->m_action->setEnabled(false);

                if (dock->m_current)
                {
                    dock->m_current->deleteLater();
                    dock->m_current = NULL;
                }
            }
        }

        //Save the state
        QSettings settings;
        settings.setValue("DockWidgetManager/Ids", ids);
        settings.setValue("DockWidgetManager/Statuses", statuses);
    }

}



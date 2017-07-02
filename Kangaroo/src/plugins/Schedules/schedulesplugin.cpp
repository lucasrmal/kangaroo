/*
This file is part of Kangaroo.
Copyright (C) 2014 Lucas Rioux-Maldague

Kangaroo is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Kangaroo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Kangaroo. If not, see <http://www.gnu.org/licenses/>.
*/

#include "schedulesplugin.h"
#include "scheduleeditor.h"
#include "scheduleentryform.h"
#include "../TabInterface/tabinterface.h"

#include <QAction>
#include <QMenu>

#include <KangarooLib/model/schedule.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>

using namespace KLib;

SchedulesPlugin::SchedulesPlugin() : m_tab(NULL)
{
}

bool SchedulesPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    ActionContainer* mnuEdit = ActionManager::instance()->actionContainer(STD_MENUS::MENU_MANAGE);

    m_mnuScheduleEditor = mnuEdit->menu()->addAction(Core::icon("schedule"),
                                                     tr("Scheduled &Transactions"),
                                                     this,
                                                     SLOT(viewScheduleEditor()));

    ActionManager::instance()->registerAction(m_mnuScheduleEditor, "Manage.ScheduledTransactions");

    m_mnuScheduleEditor->setEnabled(false);

    return true;
}

void SchedulesPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void SchedulesPlugin::viewScheduleEditor()
{
    if (!m_tab)
    {
        m_tab = new ScheduleEditor();
    }

    TabInterface::instance()->addRegisteredTab(m_tab, tr("Schedule Editor"), "ScheduleEditor", true);
}

void SchedulesPlugin::onLoad()
{    
    m_mnuScheduleEditor->setEnabled(true);

    //Find the due schedules
//    QList<QDate> dates;
//    QList<Schedule*> schedules = ScheduleManager::instance()->dueSchedules(dates);

//    QList<QDate> altDates;
//    QList<Schedule*> altSchedules;

//    for (int i = 0; i < schedules.count(); ++i)
//    {
//        if (schedules[i]->autoEnter())
//        {
//            schedules[i]->enterNext();
//        }
//        else
//        {
//            altSchedules << schedules[i];
//            altDates << dates[i];
//        }
//    }

//    if (altSchedules.count())
//    {
//        ScheduleEntryForm* form = new ScheduleEntryForm(altSchedules, altDates);
//        form->setAttribute(Qt::WA_DeleteOnClose, true);
//        form->show();
//        form->raise();
//    }
}

void SchedulesPlugin::onUnload()
{
    m_mnuScheduleEditor->setEnabled(false);

    if (m_tab)
    {
        m_tab->deleteLater();
        m_tab = NULL;
    }
}

QString SchedulesPlugin::name() const
{
    return "Schedules";
}

QString SchedulesPlugin::version() const
{
    return "1.0";
}

QString SchedulesPlugin::description() const
{
    return tr("Provides scheduled transactions.");
}

QString SchedulesPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString SchedulesPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString SchedulesPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList SchedulesPlugin::requiredPlugins() const
{
    return QStringList("TabInterface");
}



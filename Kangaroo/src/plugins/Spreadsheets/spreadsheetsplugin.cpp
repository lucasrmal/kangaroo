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

#include "spreadsheetsplugin.h"
#include "spreadsheetviewer.h"
#include "../TabInterface/tabinterface.h"

#include <QAction>
#include <QMenu>

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>


using namespace KLib;

SpreadsheetsPlugin::SpreadsheetsPlugin() : m_spreadsheet(NULL)
{
}

bool SpreadsheetsPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    ActionContainer* mnuTools = ActionManager::instance()->actionContainer(STD_MENUS::MENU_TOOLS);

    m_mnuSpreadsheet = new QAction(Core::icon("spreadsheet"), tr("Spreadsheet"), mnuTools->menu());
    mnuTools->menu()->insertAction(mnuTools->menu()->actions().first(), m_mnuSpreadsheet);
    connect(m_mnuSpreadsheet, SIGNAL(triggered()), this, SLOT(viewSpreadsheet()));

    //ActionManager::instance()->registerAction(m_mnuSpreadsheet, "Tools.Spreadsheet");

    m_mnuSpreadsheet->setEnabled(false);

    return true;
}

void SpreadsheetsPlugin::viewSpreadsheet()
{
    if (!m_spreadsheet)
    {
        m_spreadsheet = new SpreadsheetViewer();
    }

    TabInterface::instance()->addRegisteredTab(m_spreadsheet, tr("Spreadsheet"), "Spreadheet", true);
}

void SpreadsheetsPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void SpreadsheetsPlugin::onLoad()
{
    m_mnuSpreadsheet->setEnabled(true);
}

void SpreadsheetsPlugin::onUnload()
{
    m_mnuSpreadsheet->setEnabled(false);
}

QString SpreadsheetsPlugin::name() const
{
    return "Spreadsheets";
}

QString SpreadsheetsPlugin::version() const
{
    return "1.0";
}

QString SpreadsheetsPlugin::description() const
{
    return tr("Provides spreadsheets to analyse data.");
}

QString SpreadsheetsPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString SpreadsheetsPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString SpreadsheetsPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList SpreadsheetsPlugin::requiredPlugins() const
{
    return QStringList("TabInterface");
}



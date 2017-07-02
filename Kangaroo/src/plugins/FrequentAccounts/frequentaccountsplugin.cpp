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

#include "frequentaccountsplugin.h"
#include "frequentaccounts.h"
#include "formeditfrequentaccounts.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/dockwidgetmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/mainwindow.h>

#include <QMenu>
#include <QAction>

using namespace KLib;

FrequentAccountsPlugin::FrequentAccountsPlugin()
{
}

bool FrequentAccountsPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    mnuEditFrequent = ActionManager::instance()->
                            actionContainer(STD_MENUS::MENU_MANAGE)->
                                menu()->addAction(Core::icon("favorites"),
                                                  tr("&Frequent Accounts"),
                                                  this, SLOT(editFrequentAccounts()));

    ActionManager::instance()->registerAction(mnuEditFrequent, "Manage.FrequentAccounts");
    mnuEditFrequent->setEnabled(false);

    m_factory = new DockWidgetFactory<FrequentAccounts>();

    DockWidgetManager::instance()->registerDockWidget("FrequentAccounts",
                                                      tr("&Frequent Accounts"),
                                                      Core::icon("favorites"),
                                                      Qt::LeftDockWidgetArea,
                                                      true,
                                                      m_factory);

    return true;
}

void FrequentAccountsPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void FrequentAccountsPlugin::onLoad()
{
    mnuEditFrequent->setEnabled(true);
}

void FrequentAccountsPlugin::onUnload()
{
    mnuEditFrequent->setEnabled(false);
}

void FrequentAccountsPlugin::editFrequentAccounts()
{
    FormEditFrequentAccounts f;

    if (f.exec() == QDialog::Accepted && m_factory->currentDock())
    {
        static_cast<FrequentAccounts*>(m_factory->currentDock())->reloadAccounts();
    }
}

QString FrequentAccountsPlugin::name() const
{
    return "FrequentAccounts";
}

QString FrequentAccountsPlugin::version() const
{
    return "1.0";
}

QString FrequentAccountsPlugin::description() const
{
    return tr("");
}

QString FrequentAccountsPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString FrequentAccountsPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString FrequentAccountsPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList FrequentAccountsPlugin::requiredPlugins() const
{
    return QStringList("TabInterface");
}



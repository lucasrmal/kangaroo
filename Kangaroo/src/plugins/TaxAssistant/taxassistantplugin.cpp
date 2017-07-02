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

#include "taxassistantplugin.h"
#include "taxassistant.h"
#include "taxstatustab.h"
#include "taxreporter.h"
#include "../AccountCreateEdit/formeditaccount.h"

#include <QAction>
#include <QMenu>

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/controller/scriptengine.h>
#include <KangarooLib/interfaces/scriptable.h>

using namespace KLib;

TaxAssistantPlugin::TaxAssistantPlugin()
{
}

bool TaxAssistantPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

//    ActionContainer* mnuTools = ActionManager::instance()->actionContainer(STD_MENUS::MENU_TOOLS);

//    m_mnuTaxAssistant = new QAction(Core::icon("taxes-finances"), tr("Ta&x Assistant"), mnuTools->menu());
//    mnuTools->menu()->insertAction(mnuTools->menu()->actions().first(), m_mnuTaxAssistant);
//    connect(m_mnuTaxAssistant, SIGNAL(triggered()), this, SLOT(viewTaxAssistant()));

//    ActionManager::instance()->registerAction(m_mnuTaxAssistant, "Tools.TaxAssistant");

//    m_mnuTaxAssistant->setEnabled(false);

    FormEditAccount::registerTab([](QWidget* parent) { return new TaxStatusTab(parent);});

    ScriptEngine::instance()->registerScriptable(new ScriptableInstance<TaxReporter>());
    ScriptEngine::instance()->registerScriptable(new TaxReporterScriptable());

    return true;
}

void TaxAssistantPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void TaxAssistantPlugin::onLoad()
{
//    m_mnuTaxAssistant->setEnabled(true);
}

void TaxAssistantPlugin::onUnload()
{
//    m_mnuTaxAssistant->setEnabled(false);

//    if (m_tab)
//    {
//        m_tab->deleteLater();
//        m_tab = NULL;
//    }
}

//void TaxAssistantPlugin::viewTaxAssistant()
//{
//    if (!m_tab)
//    {
//        m_tab = new TaxAssistant();
//    }

//    TabInterface::instance()->addRegisteredTab(m_tab, tr("Tax Assistant"), "TaxAssistant", true);
//}

QString TaxAssistantPlugin::name() const
{
    return "TaxAssistant";
}

QString TaxAssistantPlugin::version() const
{
    return "1.0";
}

QString TaxAssistantPlugin::description() const
{
    return tr("Helps with preparing tax returns.");
}

QString TaxAssistantPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString TaxAssistantPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString TaxAssistantPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList TaxAssistantPlugin::requiredPlugins() const
{
    return {"AccountCreateEdit"};
}



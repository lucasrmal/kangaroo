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

#include "investmentcalculatorplugin.h"

#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/command.h>
#include <QMenu>
#include <QAction>

#include "formcalculator.h"

using namespace KLib;

InvestmentCalculatorPlugin::InvestmentCalculatorPlugin() : m_calc(NULL)
{
}

bool InvestmentCalculatorPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    ActionContainer* mnuTools = ActionManager::instance()->actionContainer(STD_MENUS::MENU_TOOLS);

    mnuCalc = new QAction(Core::icon("investment-calculator"), tr("In&vestment Calculator"), mnuTools->menu());
    mnuTools->menu()->insertAction(mnuTools->menu()->actions().first(),
                                   mnuCalc);

    connect(mnuCalc, SIGNAL(triggered()), this, SLOT(showCalculator()));
    ActionManager::instance()->registerAction(mnuCalc, "Tools.InvestmentCalculator");

    return true;
}

void InvestmentCalculatorPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void InvestmentCalculatorPlugin::showCalculator()
{
    if (m_calc)
    {
        m_calc->raise();
        m_calc->activateWindow();
        m_calc->show();
    }
    else
    {
        m_calc = new FormCalculator(Core::instance()->mainWindow());
        m_calc->show();
    }
}

void InvestmentCalculatorPlugin::onLoad()
{
}

void InvestmentCalculatorPlugin::onUnload()
{
}

QString InvestmentCalculatorPlugin::name() const
{
    return "InvestmentCalculator";
}

QString InvestmentCalculatorPlugin::version() const
{
    return "1.0";
}

QString InvestmentCalculatorPlugin::description() const
{
    return tr("");
}

QString InvestmentCalculatorPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString InvestmentCalculatorPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString InvestmentCalculatorPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList InvestmentCalculatorPlugin::requiredPlugins() const
{
    return QStringList();
}



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

#include "investingtabplugin.h"
#include "investingpane.h"
#include "views/sectorstab.h"
#include "views/positionstab.h"
#include "views/returnstab.h"
#include "views/dividendstab.h"

#include <KangarooLib/ui/core.h>

using namespace KLib;

InvestingTabPlugin::InvestingTabPlugin()
{
}

bool InvestingTabPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    CentralWidget::instance()->registerSidePane(new InvestingPaneBuilder());

    //Load the views
    InvestingPane::registerView("INV_POSITIONS",
                                0,
                                tr("Positions"),
                                "view-form-table",
                                [](Portfolio* portfolio, QWidget* parent){ return new PositionsTab(portfolio, parent); });

    InvestingPane::registerView("INV_RETURNS",
                                10,
                                tr("Returns"),
                                "office-chart-line",
                                [](Portfolio* portfolio, QWidget* parent){ return new ReturnsTab(portfolio, parent); });

    InvestingPane::registerView("INV_SECTORS",
                                20,
                                tr("Sectors"),
                                "office-chart-ring",
                                [](Portfolio* portfolio, QWidget* parent){ return new SectorsTab(portfolio, parent); });

    InvestingPane::registerView("INV_DIVIDENDS",
                                30,
                                tr("Dividends"),
                                "dividend",
                                [](Portfolio* portfolio, QWidget* parent){ return new DividendsTab(portfolio, parent); });

    return true;
}

void InvestingTabPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void InvestingTabPlugin::onLoad()
{
}

void InvestingTabPlugin::onUnload()
{
}

QString InvestingTabPlugin::name() const
{
    return "InvestingTab";
}

QString InvestingTabPlugin::version() const
{
    return "1.0";
}

QString InvestingTabPlugin::description() const
{
    return tr("Provides a user interface to display information about investments.");
}

QString InvestingTabPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString InvestingTabPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString InvestingTabPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList InvestingTabPlugin::requiredPlugins() const
{
    return {"TabInterface"};
}



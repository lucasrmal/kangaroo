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

#include "rewardsplugin.h"
#include "accountrewardstab.h"
#include "rewardsledgercontroller.h"
#include "rewardsprogram.h"
#include "formmanageprograms.h"

#include <QAction>
#include <QMenu>

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/controller/io.h>
#include <KangarooLib/controller/ledger/ledgercontrollermanager.h>
#include "../AccountCreateEdit/formeditaccount.h"

using namespace KLib;


RewardsPlugin::RewardsPlugin()
{
}

bool RewardsPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    //Add AccountRewardsTab
    FormEditAccount::registerTab([](QWidget* _parent) { return new AccountRewardsTab(_parent); });

    //Add Reward Ledger
    LedgerControllerManager::instance()
            ->registerLedger(new LedgerControllerFactory<RewardsLedgerController>("rewards",
                             [](const Account* _a)
                             { return _a->properties()->contains(RewardsProgram::PROP_REWARDS_PROGRAM); },
                             30));

    //Add RewardProgram
    IO::instance()->registerStored(RewardsProgramManager::TAG,
                                   RewardsProgramManager::instance());

    //Manage Rewards Programs menu

    ActionContainer* mnuEdit = ActionManager::instance()->actionContainer(STD_MENUS::MENU_MANAGE);

    mnuPrograms = mnuEdit->menu()->addAction(Core::icon("favorites"), tr("Re&wards Programs"), this, SLOT(managePrograms()));
    ActionManager::instance()->registerAction(mnuPrograms, "Manage.RewardsPrograms");

    mnuPrograms->setEnabled(false);

    //Add Rewards account type
    CustomType* c = new CustomType();
    c->id = RewardsProgram::REWARDS_ACCOUNT_TYPE;
    c->name = tr("Rewards");
    c->negativeDebits = false;
    c->weight = -675;
    c->classification = AccountClassification::OTHER;
    c->generalType = AccountType::ASSET;
    c->parents = QSet<int>()  << c->id
                              << AccountType::ASSET
                              << AccountType::CHECKING
                              << AccountType::CHECKING
                              << AccountType::PREPAIDCARD
                              << AccountType::DEPOSIT;
    c->children = QSet<int>() << c->id
                              << AccountType::ASSET;
    c->icon = "favorites";
    c->customDebit = tr("Earn");
    c->customCredit = tr("Redeem");

    c->newValidation = [](const Account*,QString) { return true; };
    c->deleteValidation = [](const Account*,QString) { return true; };

    return Account::addCustomType(c, p_errorMessage);
}

void RewardsPlugin::managePrograms()
{
    FormManagePrograms form(Core::instance()->mainWindow());
    form.exec();
}

void RewardsPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void RewardsPlugin::onLoad()
{
    mnuPrograms->setEnabled(true);
}

void RewardsPlugin::onUnload()
{
    mnuPrograms->setEnabled(false);
}

QString RewardsPlugin::name() const
{
    return "Rewards";
}

QString RewardsPlugin::version() const
{
    return "1.0";
}

QString RewardsPlugin::description() const
{
    return tr("");
}

QString RewardsPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString RewardsPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString RewardsPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList RewardsPlugin::requiredPlugins() const
{
    return QStringList() << "AccountCreateEdit" << "ManageOthers";
}



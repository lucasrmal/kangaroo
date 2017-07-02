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

#include "propertyaccountsplugin.h"
#include "realestatehelpers.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/model/account.h>

using namespace KLib;

PropertyAccountsPlugin::PropertyAccountsPlugin()
{
}

bool PropertyAccountsPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    return addRealEstateType(p_errorMessage)
            && addMortgageType(p_errorMessage);
}

bool PropertyAccountsPlugin::addRealEstateType(QString& _error)
{
    CustomType* c = new CustomType();
    c->id = RealEstateHelpers::REAL_ESTATE_ACCOUNT_ID;
    c->name = tr("Real Estate");
    c->negativeDebits = false;
    c->weight = 500;
    c->classification = AccountClassification::PROPERTY_DEBT;
    c->generalType = AccountType::ASSET;
    c->parents = QSet<int>()  << c->id
                              << AccountType::ASSET;
    c->children = QSet<int>() << c->id
                              << AccountType::ASSET;
    c->icon = "home";
    c->customDebit = tr("Increase");
    c->customCredit = tr("Decrease");

    c->newValidation = [](const Account*,QString) { return true; };
    c->deleteValidation = [](const Account*,QString) { return true; };

    return Account::addCustomType(c, _error);
}

bool PropertyAccountsPlugin::addMortgageType(QString& _error)
{
    CustomType* c = new CustomType();
    c->id = RealEstateHelpers::MORTGAGE_ACCOUNT_ID;
    c->name = tr("Mortgage Loan");
    c->negativeDebits = true;
    c->weight = 500;
    c->classification = AccountClassification::PROPERTY_DEBT;
    c->generalType = AccountType::LIABILITY;
    c->parents = QSet<int>()  << c->id
                              << AccountType::LIABILITY;
    c->children = QSet<int>() << c->id
                              << AccountType::LIABILITY;
    c->icon = "mortgage";
    c->customCredit = tr("Decrease");
    c->customDebit = tr("Increase");

    c->newValidation = [](const Account*,QString) { return true; };
    c->deleteValidation = [](const Account*,QString) { return true; };

    return Account::addCustomType(c, _error);
}

void PropertyAccountsPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void PropertyAccountsPlugin::onLoad()
{
}

void PropertyAccountsPlugin::onUnload()
{
}

QString PropertyAccountsPlugin::name() const
{
    return "PropertyAccounts";
}

QString PropertyAccountsPlugin::version() const
{
    return "1.0";
}

QString PropertyAccountsPlugin::description() const
{
    return tr("Provides custom account types and assistants for home and mortgages.");
}

QString PropertyAccountsPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString PropertyAccountsPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString PropertyAccountsPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList PropertyAccountsPlugin::requiredPlugins() const
{
    return {};
}



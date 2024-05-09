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

#include "accountcreateeditplugin.h"

#include <KangarooLib/ui/core.h>

#include "formeditaccount.h"
#include "investmentsettings.h"
#include "secondarycurrencies.h"

using namespace KLib;

AccountCreateEditPlugin::AccountCreateEditPlugin() {}

bool AccountCreateEditPlugin::initialize(QString& p_errorMessage) {
  Q_UNUSED(p_errorMessage)

  FormEditAccount::registerTab(
      [](QWidget* _parent) { return new InvestmentSettings(_parent); });
  FormEditAccount::registerTab(
      [](QWidget* _parent) { return new SecondaryCurrencies(_parent); });

  return true;
}

void AccountCreateEditPlugin::checkSettings(QSettings& settings) const {
  Q_UNUSED(settings)
}

void AccountCreateEditPlugin::onLoad() {}

void AccountCreateEditPlugin::onUnload() {}

QString AccountCreateEditPlugin::name() const { return "AccountCreateEdit"; }

QString AccountCreateEditPlugin::version() const { return "1.0"; }

QString AccountCreateEditPlugin::description() const { return tr(""); }

QString AccountCreateEditPlugin::author() const { return Core::APP_AUTHOR; }

QString AccountCreateEditPlugin::copyright() const {
  return Core::APP_COPYRIGHT;
}

QString AccountCreateEditPlugin::url() const { return Core::APP_WEBSITE; }

QStringList AccountCreateEditPlugin::requiredPlugins() const {
  return QStringList();  //("ManageOthers");
}

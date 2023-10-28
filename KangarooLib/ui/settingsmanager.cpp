/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2008-2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
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

/**
  @file  settingsmanager.cpp
  This file contains the method definitions for the SettingsManager class

  @author   Lucas Rioux Maldague
  @date     May 24th 2010
  @version  3.0

*/

#include "settingsmanager.h"
#include "dialogs/settingsdialogs/generalsettings.h"
#include "dialogs/settingsdialogs/keyshortcutssettings.h"
#include "dialogs/settingsdialogs/ledgersettings.h"
#include "core.h"

#include <QLocale>
#include <QSettings>
#include <QDir>

namespace KLib
{
    SettingsManager* SettingsManager::m_instance = nullptr;

    //SettingsManager
    SettingsManager::SettingsManager() : QObject(nullptr)
    {
        //Add main categories
        addCategory("environment", tr("Environment"), "environment-settings", -100);
        addCategory("ledger", tr("Ledger"), "open-ledger", 0);

        //Add default pages
        addSettingsPage(tr("General"), "environment", -100,
                        [](QWidget* parent) { return new GeneralSettings(parent);});

        addSettingsPage(tr("Keyboard Shortcuts"), "environment", 200,
                        [](QWidget* parent) { return new KeyShortcutsSettings(parent);});


        addSettingsPage(tr("General"), "ledger", 100,
                        [](QWidget* parent) { return new LedgerSettings(parent);});

        addSettingsPage(tr("Schedules"), "ledger", 200,
                        [](QWidget* parent) { return new LedgerScheduleSettings(parent);});

        //Check default settings
        registerSetting("RecentFiles/MaxCount", 5);
        registerSetting("General/DateFormat", "yyyy/MM/dd");
        registerSetting("General/Language", "en_US");

        //UI
        registerSetting("UI/ShowPictureInstitution", true);
        registerSetting("UI/ShowPicturePayee", true);

        //Ledger
        registerSetting("Ledger/AccountHeightDisplayed", 2);
        registerSetting("Ledger/ScheduleDisplayPolicy", 0);
        registerSetting("Ledger/ScheduleDisplayDays", 30);
        registerSetting("Ledger/ScheduleDisplayInstances", 1);
        registerSetting("Ledger/AskBeforeAddingNewPayee", false);
        registerSetting("Ledger/FontSize", QFont().pointSize());
        registerSetting("Ledger/RowHeight", 26);
    }
	
    SettingsManager* SettingsManager::instance()
    {
        if ( !m_instance)
            m_instance = new SettingsManager();

        return m_instance;
    }

    bool SettingsManager::addCategory(const QString& _id, const QString& _title, const QString& _icon, int _weight)
    {
        if (!m_categories.contains(_id))
        {
            m_categories[_id] = SettingsCategory(_title, _icon, _weight);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool SettingsManager::addSettingsPage(const QString& _title, const QString& _categoryId, int _weight, fn_createpage _builder)
    {
        if (m_categories.contains(_categoryId))
        {
            m_categories[_categoryId].pages.insert(_weight, SettingsPage{_title, _builder});
            return true;
        }
        else
        {
            return false;
        }
    }

    void SettingsManager::registerSetting(const QString& _key, const QVariant& _defaultValue)
    {
        if (!m_settings.contains(_key))
        {
            m_settings.setValue(_key, _defaultValue);
        }
    }

    QVariant SettingsManager::value(const QString& _key) const
    {
        return m_settings.value(_key);
    }

    void SettingsManager::setValue(const QString& _key, const QVariant& _value)
    {
        if (!m_settings.contains(_key) || m_settings.value(_key) != _value)
        {
            m_settings.setValue(_key, _value);
            emit valueChanged(_key);
        }
    }

    bool SettingsManager::contains(const QString& _key) const
    {
        return m_settings.contains(_key);
    }

    void SettingsManager::remove(const QString& _key)
    {
        m_settings.remove(_key);
    }

    QList<const SettingsManager::SettingsCategory*> SettingsManager::categories() const
    {
        typedef const SettingsCategory* cptr;

        QList<const SettingsCategory*> list;

        for (const SettingsCategory& c : m_categories)
        {
            list << &c;
        }

        //Sort by weight
        std::sort(list.begin(), list.end(), [](cptr a, cptr b) { return a->weight < b->weight; });

        return list;
    }



} //Namespace

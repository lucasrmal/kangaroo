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
  @file  settingsmanager.h
  This file contains the method declarations for the SettingsManager class

  @author   Lucas Rioux Maldague
  @date     May 24th 2010
  @version  3.0

*/

#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QSettings>
#include <QWidget>
#include <QMultiMap>
#include <functional>

class QSettings;

namespace KLib
{
    class ISettingsPage : public QWidget
    {
        Q_OBJECT

        public:
            ISettingsPage(QWidget* _parent = nullptr) :
                QWidget(_parent) {}

            /**
             * @brief Validate the page
             * @return An empty list if all good, otherwise a list of errors.
             */
            virtual QStringList validate() = 0;

            virtual void save() = 0;
            virtual void load() = 0;
    };

	/**
      The SettingsManager class manages the application settings. It creates the settings
	  dialog and exposes an interface to register settings. When created, the settings
	  manager does not contains any page or category. The settings are organized in
	  categories and in pages.
	  
	  The SettingsManager class is a singleton, so it can be accessed with the instance()
	  method.
	  
      To add categories or pages, use the methods addCategory() and addSettingsPage().
	  
	  @brief Settings Manager
	*/
    class SettingsManager : public QObject
    {            
        Q_OBJECT

        SettingsManager();

        public:

            typedef std::function<ISettingsPage*(QWidget*)> fn_createpage;

            struct SettingsPage
            {
                QString       title;
                fn_createpage builder;
            };

            struct SettingsCategory
            {
                SettingsCategory() : weight(0) {}

                SettingsCategory(const QString& _title, const QString& _icon, int _weight) :
                    title(_title),
                    icon(_icon),
                    weight(_weight) {}

                QString title;
                QString icon;
                int     weight;

                QMultiMap<int, SettingsPage> pages;
            };


            /**
              @return An instance of the Settings Manager
             */
            static SettingsManager* instance();

            bool        addCategory(const QString& _id, const QString& _title, const QString& _icon, int _weight);

            bool        addSettingsPage(const QString& _title, const QString& _categoryId, int _weight, fn_createpage _builder);

            void        registerSetting(const QString& _key, const QVariant& _defaultValue);

            QVariant    value(const QString& _key) const;
            void        setValue(const QString& _key, const QVariant& _value);
            bool        contains(const QString& _key) const;

            void        remove(const QString& _key);

            QList<const SettingsCategory*> categories() const;

        signals:
            void        valueChanged(const QString& _key);

    private:
        static SettingsManager* m_instance;

        QHash<QString, SettingsCategory> m_categories;

        QSettings m_settings;
    };

} //Namespace

#endif // SETTINGSMANAGER_H

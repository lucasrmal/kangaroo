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
  @file  iplugin.h
  This file contains the method declarations for the IPlugin interface

  @author   Lucas Rioux Maldague
  @date     May 30th 2010
  @version  3.0

*/

#ifndef IPLUGIN_H
#define IPLUGIN_H

#include <QtPlugin>

class QSettings;

namespace KLib
{
    /**
      The IPlugin interface is the base interface of all CAMSEG SCM plugins. Plugins must inherit
      this interface and QObject in order to be valid plugins. See the Qt documentation for more details.
      
      @brief Plugin interface
    */
    class IPlugin
    {
        public:

            IPlugin() {}
            virtual ~IPlugin() {}

            /**
              This method initializes the plugin contents : forms, menus, toolbars, etc. It
              will be called right at the application start.

              @param[out] p_erorMessage The error message in case the load is not succesfull.

              @return If the plugin was succesfully loaded
            */
            virtual bool initialize(QString& p_errorMessage) = 0;

            /**
              This method should test the presence of all settings defined by the plugin. It
              will be called after all plugins are loaded.
              
              @param[in] settings The settings
            */
            virtual void checkSettings(QSettings& settings) const { Q_UNUSED(settings); }

            /**
              This method will be called when the application exits. It should unload the ressources
              used by the plugin
            */
            virtual void onShutdown() {}

            /**
              This method will be called when a file is loaded
            */
            virtual void onLoad() {}

            /**
              This method will be called when a file is unloaded
            */
            virtual void onUnload() {}

            /**
              @return The plugin name
            */
            virtual QString name() const = 0;

            /**
              @return The plugin version
            */
            virtual QString version() const = 0;

            /**
              @return The plugin description
            */
            virtual QString description() const = 0;

            /**
              @return The plugin author
            */
            virtual QString author() const = 0;

            /**
              @return The plugin copyright informations
            */
            virtual QString copyright() const = 0;

            /**
              @return The plugin's author's website URL
            */
            virtual QString url() const { return QString(); }

            /**
              @return The list of plugins required by this plugin. If the required plugins are not found,
              the plugin is not loaded. Also, the required plugins will be loaded before this plugin.

              Note: requiring a plugin that requires this plugin results in undefined behaviour.
            */
            virtual QStringList requiredPlugins() const = 0;
    };
}

Q_DECLARE_INTERFACE(KLib::IPlugin, "Kangaroo.IPlugin/1.0")

#endif // IPLUGIN_H

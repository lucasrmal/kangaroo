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
  @file  core.h
  This file contains the method declarations for the Core class

  @author   Lucas Rioux Maldague
  @date     May 24th 2010
  @version  3.0

*/

#ifndef CORE_H
#define CORE_H

/**
  Checks if a settings key is set in the settings. If it's not set, it sets it to defaultValue. This macro
  must be used inside a function's body.

  Example :

  @code

  QSettings settings;
  CHECK_SETTING_EMPTY(settings, "MyCategory/MyKey", 123)
  CHECK_SETTING_EMPTY(settings, "MyCategory/MyOtherKey", "Hello World!")

  @endcode
*/
#define CHECK_SETTING_EMPTY(settings, key, defaultValue) if ( !settings.contains(key)) settings.setValue(key, defaultValue);

#include <QList>
#include <QHash>
#include <QIcon>
#include <unordered_set>

class QObject;
class QComboBox;
class QListWidget;
class QDockWidget;
class QDateEdit;

//namespace std
//{
//    template<class T>class unordered_set;
//}

namespace KLib
{
    class IPlugin;
    class MainWindow;

    /**
      @enum This enumeration contains the different paths to access application data
    */
    enum Path
    {
        Path_Icons,          ///< The folder containing the icons
        Path_HelpFile,       ///< The folder containing the help documents
        Path_Translations,   ///< The folder containing the translations
        Path_Reports,        ///< The folder containing the report definitions
        Path_Plugins,        ///< The folder containing the plugins
        Path_Themes,         ///< The folder containing the themes
        Path_License,        ///< The license file
    };

    /**
      The class Core contains generic functions, manages the plugins and manages the current campaign
      @brief Application core
    */
    class Core : public QObject
    {
        Q_OBJECT

            Core();

        public:
            ~Core();

            /**
              @return The instance of the class
            */
            static Core*		instance()
            {
                if ( !m_instance)
                    m_instance = new Core();

                return m_instance;
            }

             /**
               @return The path for the type p_for
             */
            static QString    	path(const Path p_for) { return m_paths[p_for]; }

             /**
               @return The icon identified by p_name. The identification name
               is simply the name of the file located in Path_Icons, without the extension (.xyz)
             */
            static QIcon  		icon(const QString& p_name)
            {
                return QIcon(m_paths[Path_Icons] + p_name + ".png");
            }

             /**
               @return The pixmap identified by p_name. The identification name
               is simply the name of the file located in Path_Icons, without the extension (.xyz)
             */
            static QPixmap 		pixmap(const QString& p_name)
            {
                return QPixmap(m_paths[Path_Icons] + p_name + ".png");
            }

             /**
                Sets the path for type p_for. VERY IMPORTANT : you should call this method for ALL types,
                BEFORE instancing ANY class in KLib.

                @param[in] p_for The type of path
                @param[in] p_path The path (must end with a '/' if it's a directory path. Note : use slashes (/),
                not backslashes (\) in all cases, even in Windows.
            */
            static void			setPath(const Path p_for, const QString& p_path);

            /**
              This function loads the campaign p_campaign. If a campaign is currently loaded,
              it's unloaded before loading the new one. The campaign is tested for validity and the login is asked in this method.
              If the login is not succesfull, it returns false and p_campaign is <b>deleted</b>. The campaign is NOT saved in the recent
              campaigns list.
              
              If no user or vendor is logged in, the Login Dialog will popup to ask the user to provide it's username and password.

              @param[in] p_campaign The campaign to load.

              @return If the campaign was succesfully loaded.
            */
			
            /**
              @return The application's main window. Note : you should never use directly the
              methods of MainWindow. Instead, use the wrapper classes such as ActionManager.
            */
            MainWindow*         mainWindow() const          { return m_mainWindow; }
            
            /**
              Sets the current central widget. This is a wrapper function over MainWindow::setCentralWidget().
              
              @param[in] p_centralWidget The new central widget
            */
            void                setCentralWidget(QWidget* p_centralWidget);
            
            /**
              @return The current central widget
            */
            QWidget*            centralWidget() const;

            bool                fileIsLoaded() const    { return m_isLoaded; }

            bool                canClose() const;

            QString             dateFormat() const;

            /**
              Adds the plugin p_plugin to the list of plugins.

              @param[in] p_plugin The plugin to load
            */
            void                loadPlugin(QObject* p_plugin);
			
            /**
              Initializes all the plugins loaded with loadPlugin(). All plugins should be loaded
              before calling this method. Also, it's very important to call it only ONCE !!!

              @return If the initialization was succesfull
              @see loadPlugin()
            */
            bool                initializePlugins();

            void                shutdownPlugins();
			
            /**
              @return If a plugin with the name p_name exists
            */
            bool                hasPlugin(const QString& _name) const { return m_plugins.contains(_name); }

            /**
              @return The plugin with the name p_name or NULL if it does not exists

              @see hasPlugin()
            */
            IPlugin*            plugin(const QString& _name) const { return m_plugins[_name]; }

            const QHash<QString, IPlugin*>& plugins() const { return m_plugins; }

            /**
              @return The number of plugins loaded
            */
            quint32             pluginCount() const               { return m_plugins.count(); }

            /**
              @return The list of the most recent campaigns in the following format :
              <ul>
                <li>Path if the campaign is MySQL based and has been opened throught a DB infos file</li>
                <li>Local DB address (localdb:n) where n is the no of the database
              </ul>
            */
            QStringList         recentFileList() const;
			
            /**
              Adds the path p_newPath to the list of recent campaigns. If the list already contains
              the path, it's moved at the first position. Otherwise, it's added at the first position.
              If the list exceeds the maximum lenght allowed (can be modified in SCM Settings, the last
              path is removed.

              The format of the path should be the following :
              <ul>
                <li>Path if the campaign is MySQL based and has been opened throught a DB infos file</li>
                    <li>Local DB address (localdb:n) where n is the no of the database
              </ul>

              @param[in] p_newPath The new path
            */
            void                addRecentFile(const QString& _newPath) const;
			
            /**
              Removes the path p_path from the recent campaigns list
            */
            void                removeRecentFile(const QString& _path) const;

            static void         setupDateEdit(QDateEdit* _dte);

            /**
              The GUI version (X.X.X)
            */
            static const QString APP_VERSION;
            
            /**
              The GUI codename (such as Arctic Wolf)
            */
            static const QString APP_CODENAME;

            /**
              The application name
            */
            static const QString APP_NAME;

            /**
              The application name
            */
            static const QString APP_AUTHOR;

            /**
              The application name
            */
            static const QString APP_WEBSITE;

            /**
              The application name
            */
            static const QString APP_COPYRIGHT;

        signals:
            void recentFileListModified() const;

        public slots:

            /**
             * @brief Loads a file
             */
            void                selectOpenFile();
            void                openFile(const QString& _file);

            bool                loadNew();

            bool                beginLoadNew();
            void                endLoadNew();

            void                loadRecentFile();

            void                loadRecentFileList() const;

            /**
             * @brief Saves the current file
             */
            void                saveFile() const;

            void                saveFileAs() const;
            /**
              Unloads the current file.
            */
            bool                unloadFile();

            /**
             * @brief Shows the Image Manager
             */
            void                showPictureManager() const;
			
            /**
              Shows the toolbars configuration dialog. This function blocks until the dialog is closed.
            */
            void                showToolbarsConfiguration() const;
			
            /**
              Shows the Settings dialog. This function blocks until the dialog is closed.
            */
            void                showSettings() const;
            
            /**
              Opens Help Contents file with the system's default PDF viewer.
            */
            void                showHelpContents() const;
            
            /**
              Shows the About Plugins dialog. This function blocks until the dialog is closed.
            */
            void                showAboutPlugins() const;
            
            /**
              Shows the AboutKangaroo dialog. This function blocks until the dialog is closed.
            */
            void                showAbout() const;

            void                showXMLFile() const;

        private slots:
            void                onFileDirty();
            void                onFileClean();
            void                updateWindowTitle();

        private:
            void                priv_loadFile(const QString& _file = QString());
            void                priv_saveFile(const QString& _file = QString()) const;
            void                priv_notifyLoad();
            void                priv_notifyUnload();
            void                priv_updateRecentFileMenu(const QStringList& _list) const;

            void                priv_setLoaded(bool _isLoaded);

            void                priv_checkDefaultSettings();

            bool                initializePlugin(IPlugin* plugin,
                                                 QSet<QString>& pluginsToDelete,
                                                 QSet<QString>& loadedPlugins,
                                                 QSet<QString>& dependencies,
                                                 QHash<QString, QString>& errors);

            static Core*        m_instance;
            static QHash<Path, QString> m_paths;

            QHash<QString, IPlugin*>     m_plugins;
            MainWindow*         m_mainWindow;
            bool                m_isLoaded;
            
            mutable QDockWidget* m_calculator;
    };

} //Namespace

#endif // CORE_H

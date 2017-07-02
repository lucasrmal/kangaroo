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
  @file  core.cpp
  This file contains the method definitions for the Core class

  @author   Lucas Rioux Maldague
  @date     May 24th 2010
  @version  3.0

*/

#include "core.h"
#include "mainwindow.h"
#include "../iplugin.h"
#include "settingsmanager.h"
#include "dockwidgetmanager.h"
#include "widgets/calculatordock.h"
#include "actionmanager/actionmanager.h"
#include "actionmanager/actioncontainer.h"
#include "actionmanager/command.h"
#include "actionmanager/qttoolbardialog.h"

#include "dialogs/formaboutplugins.h"
#include "dialogs/formabout.h"
#include "dialogs/formsettings.h"
#include "dialogs/formpicturemanager.h"
#include "widgets/calculator.h"

#include "../controller/io.h"
#include "../model/account.h"
#include "../model/ledger.h"

#include <iostream>
#include <QApplication>
#include <QPluginLoader>
#include <QDateEdit>
#include <QDir>
#include <QMessageBox>
#include <QDesktopServices>
#include <QSettings>
#include <QUrl>
#include <QComboBox>
#include <QListWidget>
#include <QStatusBar>
#include <QDockWidget>
#include <QFileDialog>
#include <QAction>
#include <QTextEdit>
#include <QMenu>
//#include <unordered_set>

namespace KLib
{
    Core* Core::m_instance = nullptr;
    QHash<Path, QString> Core::m_paths = QHash<Path, QString>();

    const QString Core::APP_VERSION     = "0.99.5 (Beta)";
    const QString Core::APP_CODENAME    = "Red";
    const QString Core::APP_NAME        = "Kangaroo";
    const QString Core::APP_AUTHOR      = "Lucas Rioux-Maldague";
    const QString Core::APP_WEBSITE     = "";
    const QString Core::APP_COPYRIGHT   = "(C) 2015 Lucas Rioux-Maldague";

    Core::Core() : m_mainWindow(new MainWindow()),
                   m_calculator(nullptr)
    {
        connect(IO::instance(), &IO::isCleanNow,  this, &Core::onFileClean);
        connect(IO::instance(), &IO::isDirtyNow,  this, &Core::onFileDirty);
        connect(IO::instance(), &IO::nameChanged, this, &Core::updateWindowTitle);

        //Settings
        priv_checkDefaultSettings();
    }

    Core::~Core()
    {
        delete m_mainWindow;
        delete m_calculator;

        //Delete plugins
        foreach (IPlugin* plugin, m_plugins)
            delete plugin;
    }

    void Core::setPath(const Path p_for, const QString& p_path)
    {
        m_paths[p_for] = p_path;
    }
    
    void Core::setCentralWidget(QWidget* p_centralWidget)
    {
        m_mainWindow->setCentralWidget(p_centralWidget);
    }
            
    QWidget* Core::centralWidget() const
    {
        return m_mainWindow->centralWidget();
    }

    void Core::selectOpenFile()
    {
        priv_loadFile();
    }

    void Core::openFile(const QString& _file)
    {
        QFile f(_file);
        if (!f.exists())
        {
            QMessageBox::information(mainWindow(),
                                     tr("Open File"),
                                     tr("The specified file does not exists."));
        }
        else
        {
            priv_loadFile(_file);
        }
    }

    void Core::priv_setLoaded(bool _isLoaded)
    {
        m_isLoaded = _isLoaded;
        ActionManager::instance()->command("File.Close")->action()->setEnabled(m_isLoaded);
        ActionManager::instance()->command("File.Save")->action()->setEnabled(m_isLoaded);
        ActionManager::instance()->command("File.SaveAs")->action()->setEnabled(m_isLoaded);
        ActionManager::instance()->command("File.PictureManager")->action()->setEnabled(m_isLoaded);
    }

    void Core::priv_checkDefaultSettings()
    {
        QSettings settings;

        if (!settings.contains("General/DateFormat"))
        {
            settings.setValue("General/DateFormat", Constants::DEFAULT_DATE_FORMAT);
        }
    }


    void Core::onFileDirty()
    {
        ActionManager::instance()->command("File.Save")->action()->setEnabled(true);
    }

    void Core::onFileClean()
    {
        ActionManager::instance()->command("File.Save")->action()->setEnabled(false);
    }

    bool Core::beginLoadNew()
    {
        //Check if a file is currently opened
        if (!canClose())
            return false;

        if (m_isLoaded)
        {
            priv_notifyUnload();
            IO::instance()->unload();
        }

        try
        {
            IO::instance()->loadNew();
        }
        catch (IOException e)
        {
            QMessageBox::warning(m_mainWindow,
                                 tr("Unable to Create File"),
                                 tr("An error has occured while creating the file:\n\n%1").arg(e.what()),
                                 QMessageBox::Ok);
            priv_setLoaded(false);
            return false;
        }

        return true;
    }

    void Core::endLoadNew()
    {
        priv_notifyLoad();
        priv_setLoaded(true);
        updateWindowTitle();
        onFileDirty();
    }

    bool Core::loadNew()
    {
        if (!beginLoadNew())
            return false;

        endLoadNew();
        return true;
    }

    void Core::priv_loadFile(const QString& _file)
    {
        //Check if a file is currently opened
        if (!canClose())
            return;

        QString filePath = _file;

        if (filePath.isEmpty())
        {
            QString kanFile = tr("Kangaroo File") + " (*.kan)";
            QString allFiles = tr("All files") + " (*.*)";

            filePath = QFileDialog::getOpenFileName(m_mainWindow,
                                                    tr("Open Kangaroo File"),
                                                    "",
                                                    kanFile + ";;" + allFiles);
        }

        if ( !filePath.isEmpty())
        {
            QApplication::setOverrideCursor(Qt::WaitCursor);

            if (m_isLoaded)
            {
                priv_notifyUnload();
                IO::instance()->unload();
            }

            try
            {
                IO::instance()->load(filePath);

                priv_notifyLoad();
                priv_setLoaded(true);
                onFileClean();

                addRecentFile(filePath);
                updateWindowTitle();

                QApplication::restoreOverrideCursor();
            }
            catch (IOException e)
            {
                QApplication::restoreOverrideCursor();
                QMessageBox::warning(m_mainWindow,
                                     tr("Unable to Load File"),
                                     tr("An error has occured while loading the file:\n\n%1").arg(e.what()),
                                     QMessageBox::Ok);
                priv_setLoaded(false);
            }
        }

    }

    void Core::loadRecentFile()
    {
        //Check if the sender is a Recent Campaign menu
        QAction* mnuRecent = qobject_cast<QAction*>(sender());

        if (mnuRecent && !mnuRecent->data().toString().isEmpty())
        {
            priv_loadFile(mnuRecent->data().toString());
        }
    }

    void Core::loadRecentFileList() const
    {
        QStringList recentFiles = recentFileList();
        QMenu* mnuRecentFiles = ActionManager::instance()->actionContainer("File.OpenRecent")->menu();
        mnuRecentFiles->clear();

        foreach (QString recent, recentFiles)
        {
            mnuRecentFiles->addAction(recent, this, SLOT(loadRecentFile()));
        }
    }

    bool Core::unloadFile()
    {
        //Check if a file is currently opened
        if (!canClose())
            return false;

        priv_notifyUnload();
        IO::instance()->unload();
        priv_setLoaded(false);
        updateWindowTitle();
        return true;
    }

    QString Core::dateFormat() const
    {
        QSettings s;
        return s.value("General/DateFormat", "yyyy/MM/dd").toString();
    }

    bool Core::canClose() const
    {
        if (m_isLoaded && IO::instance()->isDirty())
        {
            int ans = QMessageBox::question(m_mainWindow,
                                      tr("File Has Modifications"),
                                      tr("The current file has modifications. Do you want to save them, discard them or cancel?"),
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
                                      QMessageBox::Save);

            switch (ans)
            {
            case QMessageBox::Cancel:
                return false;
            case QMessageBox::Save:
                saveFile();
            default:
                break;
            }
        }

        return true;
    }

    void Core::saveFile() const
    {
        if (IO::instance()->isNew())
        {
            saveFileAs();
        }
        else
        {
            priv_saveFile(IO::instance()->currentPath());
        }
    }

    void Core::saveFileAs() const
    {
        QString kanFile = tr("Kangaroo File") + " (*.kan)";

        QString filePath = QFileDialog::getSaveFileName(m_mainWindow,
                                                tr("Save Kangaroo File"),
                                                IO::instance()->currentPath(),
                                                kanFile);

        if (!filePath.isEmpty())
        {
            if (!filePath.endsWith(".kan"))
                filePath += ".kan";

            priv_saveFile(filePath);
        }
    }

    void Core::priv_saveFile(const QString& _file) const
    {
        try
        {
            QApplication::setOverrideCursor(Qt::WaitCursor);
            IO::instance()->save(_file);
            addRecentFile(_file);
            QApplication::restoreOverrideCursor();
        }
        catch (IOException e)
        {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning(m_mainWindow,
                                 tr("Unable to Save File"),
                                 tr("An error has occured while saving the file:\n\n").arg(e.what()),
                                 QMessageBox::Ok);
        }
    }

    void Core::priv_notifyLoad()
    {
        foreach (IPlugin* plugin, m_plugins)
        {
            plugin->onLoad();
        }

        //Notify the dock widget manager
        DockWidgetManager::instance()->onLoad();

        //Restore the state of the window's toolbars and dock widgets
        m_mainWindow->restoreState(QSettings().value("ToolBarState/MainWindow").toByteArray());

    }

    void Core::priv_notifyUnload()
    {
        foreach (IPlugin* plugin, m_plugins)
        {
            plugin->onUnload();
        }

        //Notify the dock widget manager
        DockWidgetManager::instance()->onUnload();

        //Save the state of the window's toolbars and dock widgets
        QSettings settings;
        settings.setValue("ToolBarState/MainWindow", m_mainWindow->saveState());

    }

    void Core::priv_updateRecentFileMenu(const QStringList &_list) const
    {
        QMenu* mnuRecent = ActionManager::instance()->actionContainer(STD_MENUS::MENU_FILE_OPENRECENT)->menu();
        mnuRecent->clear();

        foreach (QString recent, _list)
        {
            QAction* a = mnuRecent->addAction(recent, this, SLOT(loadRecentFile()));
            a->setData(recent);
        }
    }

    void Core::updateWindowTitle()
    {
        if (m_isLoaded)
        {
            if (IO::instance()->currentName().isEmpty())
            {
                m_mainWindow->setWindowTitle(tr("Untitled - %1").arg(Core::APP_NAME));
            }
            else
            {
                m_mainWindow->setWindowTitle(IO::instance()->currentName() + " - " + Core::APP_NAME);
            }
        }
        else
        {
            m_mainWindow->setWindowTitle(Core::APP_NAME);
        }
    }

    bool Core::initializePlugins()
    {
        //Initialize plugins
        QSet<QString> pluginsToDelete;
        QSet<QString> loadedPlugins;
        QSet<QString> dependencies;
        QHash<QString, QString> errors;

        foreach (IPlugin* plugin, m_plugins)
        {
            initializePlugin(plugin, pluginsToDelete, loadedPlugins, dependencies, errors);
        }

        //Show an error message if some plugins were not loaded
        if (errors.count())
        {
            QString errorMsg;

            for (auto i = errors.begin(); i != errors.end(); ++i)
            {
                errorMsg += "<br /><br /><b>" + i.key() + "</b><br />" + i.value();
            }

            QMessageBox::warning(mainWindow(),
                                 tr("Error While Loading Plugins"),
                                 tr("Could not load the following plugin(s):%1").arg(errorMsg));
        }

        //Remove all dead plugins
        for (QString n : pluginsToDelete)
        {
            IPlugin* p = m_plugins[n];
            m_plugins.remove(n);
            delete p;
        }

        //Check plugin settings
        QSettings settings;
        foreach (IPlugin* plugin, m_plugins)
        {
            plugin->checkSettings(settings);
        }

        //Add the calculator dock
        DockWidgetManager::instance()->registerDockWidget("Calculator",
                                                          tr("&Calculator"),
                                                          icon("calc"),
                                                          Qt::RightDockWidgetArea,
                                                          false,
                                                          new DockWidgetFactory<CalculatorDock>());

        priv_setLoaded(false);

        priv_updateRecentFileMenu(recentFileList());

        //Restore toolbar state
        ActionManager::toolBarManager()->restoreState(QSettings().value("ToolBarState/Manager").toByteArray());

        return true;
    }

    bool Core::initializePlugin(IPlugin* p_plugin,
                                QSet<QString>& pluginsToDelete,
                                QSet<QString>& loadedPlugins,
                                QSet<QString>& dependencies,
                                QHash<QString, QString>& errors)
    {
        //Check if the plugin is already loaded
        if (loadedPlugins.find(p_plugin->name()) != loadedPlugins.end() ) //)
            return true;

        bool canLoadPlugin = true;
        QString errorMessage;

        //Check if we have a circular dependency (this plugin was seen before)
        if (dependencies.find(p_plugin->name()) != dependencies.end())
        {
            errors[p_plugin->name()] = QObject::tr("This plugin forms "
                                                   "a circular dependency with at least one other plugin.");
            pluginsToDelete.insert(p_plugin->name());
            return false;
        }

        //Check if the dependencies are OK
        dependencies.insert(p_plugin->name());
        foreach (QString name, p_plugin->requiredPlugins())
        {
            if (pluginsToDelete.find(name) != pluginsToDelete.end())
            {
                errors[p_plugin->name()] = QObject::tr("This plugin requires another plugin: "
                                                       "%1, which could not be loaded.").arg(name);
                pluginsToDelete.insert(p_plugin->name());
                canLoadPlugin = false;
                break;
            }
            if ( !hasPlugin(name)) //Check if the plugin is found
            {
                errors[p_plugin->name()] = QObject::tr("This plugin requires another plugin: "
                                                       "%1, which could not be found.").arg(name);
                pluginsToDelete.insert(p_plugin->name());
                canLoadPlugin = false;
                break;
            }
                //If the required plugin exists, we initialize it first
            else if ( !initializePlugin(plugin(name), pluginsToDelete, loadedPlugins, dependencies, errors))
            {
                errors[p_plugin->name()] = QObject::tr("This plugin requires another plugin: "
                                                       "%1, which could not be loaded.").arg(name);
                pluginsToDelete.insert(p_plugin->name());
                canLoadPlugin = false;
                break;
            }
        }
        dependencies.remove(p_plugin->name());

        //If dependencies are OK, initialize the plugin
        if (canLoadPlugin)
        {
            if ( !p_plugin->initialize(errorMessage))
            {

                errors[p_plugin->name()] = errorMessage;
                pluginsToDelete.insert(p_plugin->name());
                canLoadPlugin = false;
            }
            else
            {
                loadedPlugins.insert(p_plugin->name());
            }
        }

        return canLoadPlugin;
    }

    QStringList Core::recentFileList() const
    {
        return QSettings().value("RecentFiles/List").toStringList();
    }

    void Core::addRecentFile(const QString& p_newPath) const
    {
        QSettings settings;
        QStringList listeBackup = settings.value("RecentFiles/List").toStringList();

        //Insert the new path in the list
        if (listeBackup.contains(p_newPath))
        {
            listeBackup.move(listeBackup.indexOf(p_newPath), 0);
        }
        else
        {
            listeBackup.insert(0, p_newPath);
        }

        //Resize the list to the max count if the list is too large
        int maxEntries = settings.value("RecentFiles/MaxCount").toInt();

        //Remove all empty values
        listeBackup.removeAll("");

        if (listeBackup.count() > maxEntries)
            while (listeBackup.count() != maxEntries)
                listeBackup.removeLast();

        //Save the list
        settings.setValue("RecentFiles/List", listeBackup);

        priv_updateRecentFileMenu(listeBackup);
        emit recentFileListModified();
    }

    void Core::removeRecentFile(const QString& p_item) const
    {
        QSettings settings;
        QStringList listeBackup = settings.value("RecentFiles/List").toStringList();

        //Remove the item
        listeBackup.removeAll(p_item);

        //Save the list
        settings.setValue("RecentFiles/List", listeBackup);

        priv_updateRecentFileMenu(listeBackup);
        emit recentFileListModified();
    }

    void Core::setupDateEdit(QDateEdit* _dte)
    {
        QSettings settings;
        _dte->setCalendarPopup(true);
        _dte->setDisplayFormat(settings.value("General/DateFormat", "yyyy/MM/dd").toString());
        _dte->setDate(QDate::currentDate());
    }

    void Core::shutdownPlugins()
    {
        //Shut down all plugins
        foreach (IPlugin* plugin, m_plugins)
        {
            plugin->onShutdown();
        }
    }

    void Core::loadPlugin(QObject* _plugin)
    {
        IPlugin* plugin = qobject_cast<IPlugin*>(_plugin);

        if (plugin)
        {
            m_plugins.insert(plugin->name(), plugin);
        }
    }

    void Core::showPictureManager() const
    {
        FormPictureManager* picMan = new FormPictureManager(mainWindow());
        picMan->exec();
        delete picMan;
    }
    
    void Core::showToolbarsConfiguration() const
    {
        QtToolBarDialog* dlToolbars = new QtToolBarDialog(mainWindow());
        
        dlToolbars->setToolBarManager(ActionManager::toolBarManager());
        dlToolbars->exec();
        
        delete dlToolbars;
    }

    void Core::showSettings() const
    {
        FormSettings* dlSettings = new FormSettings(m_mainWindow);
        dlSettings->exec();
        delete dlSettings;
    }
    
    void Core::showHelpContents() const
    {
        QString language = QSettings().value("General/Language").toString();
        QString pathFile = QString("help_%1.pdf").arg(language);

        if ( ! QFile::exists(path(Path_HelpFile) + pathFile))
        {
            pathFile = QString("help_en.pdf");
        }

        QDesktopServices::openUrl(QUrl::fromLocalFile(path(Path_HelpFile) + pathFile));
    }

    void Core::showAboutPlugins() const
    {
        FormAboutPlugins* dlAboutPlugins = new FormAboutPlugins(m_mainWindow);
        dlAboutPlugins->exec();
        delete dlAboutPlugins;
    }

    void Core::showAbout() const
    {
        frmAbout* dlAbout = new frmAbout(m_mainWindow);
        dlAbout->exec();
        delete dlAbout;
    }

    void Core::showXMLFile() const
    {
        CAMSEGDialog* dl = new CAMSEGDialog(CAMSEGDialog::DialogWithoutPicture, CAMSEGDialog::CloseButton);
        dl->setBothTitles(tr("XML File"));
        QTextEdit* e = new  QTextEdit(dl);
        e->setReadOnly(true);
        dl->setCentralWidget(e);
        e->setText(IO::instance()->xmlFileContents());
        dl->exec();
        dl->setMinimumSize(500, 500);
        delete dl;
    }


} //Namespace

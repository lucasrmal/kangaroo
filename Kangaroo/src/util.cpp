/*
CAMSEG SCM Graphical User Interface
Copyright (C) 2008-2010 CAMSEG Technologies

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

//Project includes
#include "util.h"

//SCM includes
#include <KangarooLib/ui/core.h>

//Qt includes
#include <QSettings>
#include <QPushButton>
#include <QMessageBox>
#include <QApplication>
#include <QPluginLoader>
#include <QDir>
#include <QDebug>

using namespace KLib;

namespace Util {

    void setupPaths()
    {
        Core::setPath(Path_Icons,          QApplication::applicationDirPath() +  + "/../share/icons/");
        Core::setPath(Path_HelpFile,       QApplication::applicationDirPath() +  + "/../share/help/");
        Core::setPath(Path_Translations,   QApplication::applicationDirPath() +  + "/../share/translations/");
        Core::setPath(Path_Reports,        QApplication::applicationDirPath() +  + "/../share/reports/");
        Core::setPath(Path_Plugins,        QApplication::applicationDirPath() +  + "/../plugins/");
        Core::setPath(Path_Themes,   QApplication::applicationDirPath() +  + "/../share/themes/");
        Core::setPath(Path_License,        QApplication::applicationDirPath() +  + "/../share/gpl.txt");
    }

    void checkOldSettings()
    {
        /*
        QSettings old("Lucas Rioux Maldague", "CAMSEG");

        if ( ! old.value("general/language").toString().isEmpty())
        {
            QMessageBox msgBox;
            QPushButton* convertSettings = msgBox.addButton(QObject::tr("Convert Settings"), QMessageBox::AcceptRole);
            QPushButton* deleteSettings = msgBox.addButton(QObject::tr("Delete Settings"), QMessageBox::DestructiveRole);
            msgBox.addButton(QMessageBox::Ignore);

            msgBox.setDefaultButton(convertSettings);

            msgBox.setWindowTitle(QObject::tr("Import Old Settings"));
            msgBox.setText(QObject::tr("General settings from a previous CAMSEG version (1.x) have been found. Do you want to convert them to the new format ? Note that it won't be possible to use them in a previous version of CAMSEG anymore."));

            msgBox.exec();

            if (msgBox.clickedButton() == convertSettings)
            {
                SCMApplication::convertOldSettings();
            }
            else if (msgBox.clickedButton() == deleteSettings)
            {
                SCMApplication::deleteOldSettings();
            }
            */
    }

    bool loadPlugins()
    {
        //--------------------Load plugins----------------------

        //Static plugins
        foreach (QObject *plugin, QPluginLoader::staticInstances())
            Core::instance()->loadPlugin(plugin);

        //Dynamic plugins
        QDir pluginsDir(Core::path(Path_Plugins));

        foreach (QString fileName, pluginsDir.entryList(QDir::Files))
        {
            QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
            loader.setLoadHints(QLibrary::ExportExternalSymbolsHint);

            QObject *plugin = loader.instance();

            if (plugin)
            {
                Core::instance()->loadPlugin(plugin);
            }
            else
            {
                qDebug() << QString("Unable to load " + fileName + " : " + loader.errorString());
            }
        }

        //-----------------Initialize plugins-------------------
        return Core::instance()->initializePlugins();

    }


} //Namespace

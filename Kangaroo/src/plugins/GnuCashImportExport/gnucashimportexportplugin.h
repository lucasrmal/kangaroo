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

#ifndef GNUCASHIMPORTEXPORTPLUGIN_H
#define GNUCASHIMPORTEXPORTPLUGIN_H

#include <KangarooLib/iplugin.h>
#include <QObject>

class GnuCashImportExportPlugin : public QObject, public KLib::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Kangaroo.IPlugin/1.0")
    Q_INTERFACES(KLib::IPlugin)

    public:
        GnuCashImportExportPlugin();

        bool initialize(QString& p_errorMessage);

        void checkSettings(QSettings& settings) const;

        void onLoad();
        void onUnload();

        QString name() const;
        QString version() const;
        QString description() const;
        QString author() const;
        QString copyright() const;
        QString url() const;

        QStringList requiredPlugins() const;

};

#endif // GNUCASHIMPORTEXPORTPLUGIN_H



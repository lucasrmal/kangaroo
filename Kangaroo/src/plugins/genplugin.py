import os
import re
import sys

# Python script to generate a Kangaroo plugin

simple_name = raw_input("Enter name of plugin: ")
plugin_name = simple_name + "Plugin"
lwr_name = plugin_name.lower()
upp_name = plugin_name.upper()
dir_name = simple_name + "/"

#check if dir already exists, if it does, error, else, create it and generate the code.

if (os.path.exists(dir_name)):
	print("Error: plugin directory already exists. Exiting.")
	sys.exit(0)
elif (not re.match('^[a-zA-Z][a-zA-Z0-9_]+$', simple_name)):
	print("Error: plugin name is invalid. Exiting.")
	sys.exit(0)
	
os.makedirs(dir_name)

cpp_file = open(dir_name + lwr_name + ".cpp", 'w')
h_file = open(dir_name + lwr_name + ".h", 'w')
pro_file = open(dir_name + simple_name + ".pro", 'w')

print("Creating " + dir_name + lwr_name + ".cpp ...")

cpp_file.write("""/*
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

#include \"""" + lwr_name + """.h"

#include <KangarooLib/ui/core.h>

using namespace KLib;

""" + plugin_name + """::""" + plugin_name + """()
{
}

bool """ + plugin_name + """::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    return true;
}

void """ + plugin_name + """::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void """ + plugin_name + """::onLoad()
{
}

void """ + plugin_name + """::onUnload()
{
}

QString """ + plugin_name + """::name() const
{
    return \"""" + simple_name + """\";
}

QString """ + plugin_name + """::version() const
{
    return "1.0";
}

QString """ + plugin_name + """::description() const
{
    return tr(\"\");
}

QString """ + plugin_name + """::author() const
{
    return Core::APP_AUTHOR;
}

QString """ + plugin_name + """::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString """ + plugin_name + """::url() const
{
    return Core::APP_WEBSITE;
}

QStringList """ + plugin_name + """::requiredPlugins() const
{
    return {};
}


""")

print("Creating " + dir_name + lwr_name + ".h ...")

h_file.write("""/*
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

#ifndef """ + upp_name + """_H
#define """ + upp_name + """_H

#include <KangarooLib/iplugin.h>
#include <QObject>

class """ + plugin_name + """ : public QObject, public KLib::IPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "Kangaroo.IPlugin/1.0")
    Q_INTERFACES(KLib::IPlugin)

    public:
        """ + plugin_name + """();

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

#endif // """ + upp_name + """_H


""")

print("Creating " + dir_name + simple_name + ".pro ...")

pro_file.write("""# This file is part of Kangaroo.
# Copyright (C) 2014 Lucas Rioux-Maldague
# 
# Kangaroo is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Kangaroo is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with Kangaroo. If not, see <http://www.gnu.org/licenses/>.
#

QMAKE_CXXFLAGS += -std=c++11
QT += widgets script
TARGET = """ + simple_name + """
TEMPLATE = lib
CONFIG       += plugin
DESTDIR = ../../../plugins
INCLUDEPATH += ../../../../
SOURCES += """ + lwr_name + """.cpp
HEADERS += """ + lwr_name + """.h

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui""")

cpp_file.close()
h_file.close()
pro_file.close()

print("Done!")

	
		

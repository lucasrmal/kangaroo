# This file is part of Kangaroo.
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
TARGET = Schedules
TEMPLATE = lib
CONFIG       += plugin
DESTDIR = ../../../plugins
INCLUDEPATH += ../../../../
SOURCES += schedulesplugin.cpp \
    scheduleeditor.cpp \
    scheduleentryform.cpp
HEADERS += schedulesplugin.h \
    scheduleeditor.h \
    scheduleentryform.h

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui

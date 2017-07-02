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
QT += widgets script network
TARGET = InvestingTab
TEMPLATE = lib
CONFIG       += plugin
DESTDIR = ../../../plugins
INCLUDEPATH += ../../../../
SOURCES += investingtabplugin.cpp \
    investingpane.cpp\
    views/positionstab.cpp \
    models/positionsmodel.cpp \
    models/portfolio.cpp \
    models/portfoliosectormodel.cpp \
    views/sectorstab.cpp \
    models/positionsreturnmodel.cpp \
    models/positionsvaluationmodel.cpp \
    views/returnstab.cpp \
    models/positionsoverviewmodel.cpp \
    models/dividendsmodel.cpp \
    views/dividendstab.cpp \
    views/standardtab.cpp
HEADERS += investingtabplugin.h \
    investingpane.h \
    views/positionstab.h \
    models/positionsmodel.h \
    models/portfolio.h \
    models/portfoliosectormodel.h \
    views/sectorstab.h \
    models/positionsreturnmodel.h \
    models/positionsvaluationmodel.h \
    views/returnstab.h \
    models/positionsoverviewmodel.h \
    models/dividendsmodel.h \
    views/dividendstab.h \
    views/standardtab.h

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui

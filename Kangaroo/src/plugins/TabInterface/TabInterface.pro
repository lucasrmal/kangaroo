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

QMAKE_CXXFLAGS += -std=c++20
QT += widgets script printsupport #webkitwidgets
TARGET = TabInterface
TEMPLATE = lib
CONFIG       += plugin
DESTDIR = ../../../plugins
INCLUDEPATH += ../../../../
SOURCES += \
    tabinterfaceplugin.cpp \
    tabinterface.cpp \
    hometab.cpp \
    ledgertab.cpp \
    centralwidget.cpp \
    report/formsetsettings.cpp \
    report/report.cpp \
    report/reportselector.cpp \
    report/reportviewer.cpp \
    accountpane.cpp \
    accountactions.cpp \
    incomeexpensetab.cpp \
    accountviewtab.cpp \
    accounttreewidget.cpp \
    allaccountstab.cpp \
    welcomescreen.cpp \
    homewidgets/categoryvaluechart.cpp \
    formedithometabwidgets.cpp \
    homewidgets/categoryvaluecharteditor.cpp \
    ihomewidget.cpp \
    report/ichart.cpp \
    report/charts/spendingincomeovertime.cpp \
    report/charts/accountbalancesovertime.cpp \
    homewidgets/billreminder.cpp
HEADERS += \
    tabinterfaceplugin.h \
    tabinterface.h \
    hometab.h \
    ledgertab.h \
    centralwidget.h \
    report/formsetsettings.h \
    report/report.h \
    report/reportselector.h \
    report/reportviewer.h \
    accountpane.h \
    accountactions.h \
    incomeexpensetab.h \
    accountviewtab.h \
    accounttreewidget.h \
    allaccountstab.h \
    welcomescreen.h \
    ihomewidget.h \
    homewidgets/categoryvaluechart.h \
    formedithometabwidgets.h \
    homewidgets/categoryvaluecharteditor.h \
    report/ichart.h \
    report/charts/spendingincomeovertime.h \
    report/charts/accountbalancesovertime.h \
    homewidgets/billreminder.h

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui

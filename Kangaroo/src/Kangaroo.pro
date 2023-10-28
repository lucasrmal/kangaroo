# -------------------------------------------------
# CAMSEG SCM
# Copyright (C) 2008-2010 CAMSEG Technologies
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
# -------------------------------------------------
# QT += sql \
#     network \
#     webkit \
#     xml \
#     svg \
#     script
QMAKE_CXXFLAGS += -std=c++11
QMAKE_RPATHDIR += $$PWD/../lib
QT += widgets printsupport
!win32:TARGET = kangaroo.bin
win32:TARGET = kangaroo
DESTDIR = ../bin
# RC_FILE = camseg_gui.rc
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui
TEMPLATE = app
unix:LIBS += -L/lib64 -L$$PWD/../lib -lkangaroo
      #-L/lib64
# TRANSLATIONS += camseg_fr.ts \
#     camseg_tr.ts
SOURCES += main.cpp \
    util.cpp

HEADERS += \
    util.h

INCLUDEPATH += ../../

#win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../lib/release/ -lkangaroo
#else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../lib/debug/ -lkangaroo
#else:unix: LIBS += -L$$PWD/../lib -lkangaroo

#INCLUDEPATH += $$PWD/../../KangarooLib
#DEPENDPATH += $$PWD/../../KangarooLib

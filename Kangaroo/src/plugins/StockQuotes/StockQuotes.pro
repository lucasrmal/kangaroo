QT += widgets \
network \
script

TEMPLATE = lib
DEFINES += STOCKQUOTES_LIBRARY

CONFIG += c++17
TEMPLATE = lib
CONFIG       += plugin
DESTDIR = ../../../plugins
INCLUDEPATH += ../../../../

SOURCES += \
    stockquotesplugin.cpp \
    stockwits.cpp

HEADERS += \
    stockquotesplugin.h \
    stockwits.h

OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui

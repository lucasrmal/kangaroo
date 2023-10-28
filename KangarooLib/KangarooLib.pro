#-------------------------------------------------
#
# Project created by QtCreator 2014-07-09T08:38:55
#
#-------------------------------------------------
QMAKE_CXXFLAGS += -std=c++11

TARGET = kangaroo

TEMPLATE = lib
#CONFIG += dll
DESTDIR = ../Kangaroo/lib

VERSION = 0.1
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
UI_DIR = build/ui

DEFINES += KANGAROOLIB_LIBRARY
QT += gui widgets script printsupport

INCLUDEPATH += /usr/local/include /include

FORMS += ui/actionmanager/qttoolbardialog.ui

SOURCES += \
    lib/muparser/muParser.cpp \
    lib/muparser/muParserBase.cpp \
    lib/muparser/muParserBytecode.cpp \
    lib/muparser/muParserCallback.cpp \
    lib/muparser/muParserDLL.cpp \
    lib/muparser/muParserError.cpp \
    lib/muparser/muParserInt.cpp \
    lib/muparser/muParserTest.cpp \
    lib/muparser/muParserTokenReader.cpp \
    lib/quazip/JlCompress.cpp \
    lib/quazip/qioapi.cpp \
    lib/quazip/quaadler32.cpp \
    lib/quazip/quacrc32.cpp \
    lib/quazip/quagzipfile.cpp \
    lib/quazip/quaziodevice.cpp \
    lib/quazip/quazip.cpp \
    lib/quazip/quazipdir.cpp \
    lib/quazip/quazipfile.cpp \
    lib/quazip/quazipfileinfo.cpp \
    lib/quazip/quazipnewinfo.cpp \
    lib/quazip/unzip.c \
    lib/quazip/zip.c \
    model/account.cpp \
    model/institution.cpp \
    model/stored.cpp \
    model/payee.cpp \
    model/transaction.cpp \
    model/transactionmanager.cpp \
    model/security.cpp \
    controller/payeecontroller.cpp \
    controller/institutioncontroller.cpp \
    model/ledger.cpp \
    controller/accountcontroller.cpp \
    controller/io.cpp \
    ui/actionmanager/actioncontainer.cpp \
    ui/actionmanager/actionmanager.cpp \
    ui/actionmanager/command.cpp \
    ui/actionmanager/qttoolbardialog.cpp \
    ui/dialogs/settingsdialogs/generalsettings.cpp \
    ui/dialogs/settingsdialogs/keyshortcutssettings.cpp \
    ui/dialogs/camsegdialog.cpp \
    ui/dialogs/frmdaterange.cpp \
    ui/dialogs/isaveposition.cpp \
    ui/widgets/amountedit.cpp \
    ui/widgets/filechooserbox.cpp \
    ui/widgets/shortcuteditor.cpp \
    ui/core.cpp \
    ui/mainwindow.cpp \
    ui/settingsmanager.cpp \
    model/currency.cpp \
    model/modelexception.cpp \
    model/properties.cpp \
    klib.cpp \
    ui/widgets/ledgerwidget.cpp \
    ui/widgets/accountselector.cpp \
    controller/currencycontroller.cpp \
    controller/securitycontroller.cpp \
    model/pricemanager.cpp \
    ui/dialogs/spliteditor.cpp \
    ui/widgets/splitswidget.cpp \
    controller/pricecontroller.cpp \
    controller/onlinequotes.cpp \
    ui/dialogs/formcurrencyexchange.cpp \
    controller/ledger/investmentledgercontroller.cpp \
    controller/reportgenerator.cpp \
    ui/widgets/chart.cpp \
    ui/widgets/calculator.cpp \
    ui/dockwidgetmanager.cpp \
    ui/widgets/calculatordock.cpp \
    controller/archive.cpp \
    model/picturemanager.cpp \
    controller/picturecontroller.cpp \
    ui/dialogs/formpicturemanager.cpp \
    ui/widgets/pictureselector.cpp \
    controller/ledger/ledgercontrollermanager.cpp \
    controller/ledger/ledgercontroller.cpp \
    controller/ledger/genericledgercontroller.cpp \
    ui/dialogs/formaboutplugins.cpp \
    ui/dialogs/formabout.cpp \
    controller/schedulecontroller.cpp \
    model/schedule.cpp \
    controller/simpleaccountcontroller.cpp \
    model/documentmanager.cpp \
    controller/ledger/brokerageledgercontroller.cpp \
    model/investmenttransaction.cpp \
    amount.cpp \
    model/investmentlotsmanager.cpp \
    ui/dialogs/formdistributioncomposition.cpp \
    ui/dialogs/formgainlosswizard.cpp \
    ui/widgets/splitfractionwidget.cpp \
    util/balances.cpp \
    ui/dialogs/optionsdialog.cpp \
    ui/dialogs/formeditschedule.cpp \
    controller/ledger/ledgertransactioncache.cpp \
    ui/widgets/percchart.cpp \
    controller/indexcontroller.cpp \
    ui/dialogs/formeditattachments.cpp \
    ui/widgets/returnchart.cpp \
    ui/dialogs/formsettings.cpp \
    ui/dialogs/settingsdialogs/ledgersettings.cpp \
    ui/widgets/dateintervalselector.cpp \
    ui/dialogs/formmultiinvestmententry.cpp \
    model/budget.cpp

HEADERS += \
    interfaces/scriptable.h \
    lib/muparser/muParser.h \
    lib/muparser/muParserBase.h \
    lib/muparser/muParserBytecode.h \
    lib/muparser/muParserCallback.h \
    lib/muparser/muParserDLL.h \
    lib/muparser/muParserDef.h \
    lib/muparser/muParserError.h \
    lib/muparser/muParserFixes.h \
    lib/muparser/muParserInt.h \
    lib/muparser/muParserTemplateMagic.h \
    lib/muparser/muParserTest.h \
    lib/muparser/muParserToken.h \
    lib/muparser/muParserTokenReader.h \
    lib/quazip/JlCompress.h \
    lib/quazip/crypt.h \
    lib/quazip/ioapi.h \
    lib/quazip/quaadler32.h \
    lib/quazip/quachecksum32.h \
    lib/quazip/quacrc32.h \
    lib/quazip/quagzipfile.h \
    lib/quazip/quaziodevice.h \
    lib/quazip/quazip.h \
    lib/quazip/quazip_global.h \
    lib/quazip/quazipdir.h \
    lib/quazip/quazipfile.h \
    lib/quazip/quazipfileinfo.h \
    lib/quazip/quazipnewinfo.h \
    lib/quazip/unzip.h \
    lib/quazip/zip.h \
    model/account.h \
    amount.h \
    amount.hpp \
    model/institution.h \
    model/stored.h \
    klib.h \
    model/payee.h \
    model/transaction.h \
    model/transactionmanager.h \
    model/security.h \
    controller/payeecontroller.h \
    controller/institutioncontroller.h \
    model/ledger.h \
    controller/accountcontroller.h \
    controller/io.h \
    ui/actionmanager/actioncontainer.h \
    ui/actionmanager/actionmanager.h \
    ui/actionmanager/command.h \
    ui/actionmanager/qttoolbardialog.h \
    ui/dialogs/settingsdialogs/generalsettings.h \
    ui/dialogs/settingsdialogs/keyshortcutssettings.h \
    ui/dialogs/camsegdialog.h \
    ui/dialogs/frmdaterange.h \
    ui/dialogs/isaveposition.h \
    ui/widgets/amountedit.h \
    ui/widgets/filechooserbox.h \
    ui/widgets/shortcuteditor.h \
    ui/core.h \
    iplugin.h \
    ui/mainwindow.h \
    ui/settingsmanager.h \
    model/currency.h \
    model/modelexception.h \
    model/properties.h \
    ui/widgets/ledgerwidget.h \
    ui/widgets/accountselector.h \
    controller/currencycontroller.h \
    controller/securitycontroller.h \
    model/pricemanager.h \
    ui/dialogs/spliteditor.h \
    ui/widgets/splitswidget.h \
    interfaces/iquote.h \
    controller/pricecontroller.h \
    controller/onlinequotes.h \
    ui/dialogs/formcurrencyexchange.h \
    interfaces/scriptable.h \
    controller/ledger/investmentledgercontroller.h \
    controller/reportgenerator.h \
    ui/widgets/chart.h \
    ui/widgets/calculator.h \
    ui/dockwidgetmanager.h \
    ui/widgets/calculatordock.h \
    interfaces/ifilemanager.h \
    controller/archive.h \
    model/picturemanager.h \
    controller/picturecontroller.h \
    ui/dialogs/formpicturemanager.h \
    ui/widgets/pictureselector.h \
    controller/ledger/ledgercontrollermanager.h \
    controller/ledger/ledgercontroller.h \
    controller/ledger/genericledgercontroller.h \
    ui/dialogs/formaboutplugins.h \
    ui/dialogs/formabout.h \
    controller/schedulecontroller.h \
    model/schedule.h \
    controller/simpleaccountcontroller.h \
    model/documentmanager.h \
    controller/ledger/brokerageledgercontroller.h \
    model/investmenttransaction.h \
    model/investmentlotsmanager.h \
    ui/dialogs/formdistributioncomposition.h \
    ui/dialogs/formgainlosswizard.h \
    ui/widgets/splitfractionwidget.h \
    util/augmentedtreapmap.h \
    util/fragmentedtreapmap.h \
    util/treaputil.h \
    util/balances.h \
    ui/dialogs/optionsdialog.h \
    ui/dialogs/formeditschedule.h \
    controller/ledger/ledgertransactioncache.h\
    ui/widgets/percchart.h \
    controller/indexcontroller.h \
    ui/dialogs/formeditattachments.h \
    ui/widgets/returnchart.h \
    ui/dialogs/formsettings.h \
    ui/dialogs/settingsdialogs/ledgersettings.h \
    ui/widgets/dateintervalselector.h \
    ui/dialogs/formmultiinvestmententry.h \
    model/budget.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

SUBDIRS += \
    lib/quazip/quazip.pro

DISTFILES += \
    lib/quazip/quazip.pri \
    lib/quazip/quazip.sln \
    lib/quazip/quazip.vcproj \
    lib/quazip/quazip.vcxproj \
    lib/quazip/quazip.vcxproj.filters \
    lib/quazip/run_moc.bat

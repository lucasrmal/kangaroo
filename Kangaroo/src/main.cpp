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

#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/settingsmanager.h>

#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFont>
#include <QFontDatabase>
#include <QLibraryInfo>
#include <QSettings>
#include <QSplashScreen>
#include <QTextCodec>
#include <QTranslator>
#include <ctime>

#include "util.h"

#define SPLASH_SCREEN_TIME 3

using namespace KLib;

int main(int argc, char* argv[]) {
  qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");

  QApplication a(argc, argv);

  //--------------------General Configuration--------------------
  QCoreApplication::setApplicationName(Core::APP_NAME);
  QCoreApplication::setApplicationVersion(Core::APP_VERSION);
  QCoreApplication::setOrganizationName(Core::APP_AUTHOR);
  QCoreApplication::setOrganizationDomain(Core::APP_WEBSITE);

  Util::setupPaths();

  QFont f("Droid Sans [unknown]", 9);
  f.setStyleName("Regular");
  a.setFont(f);

  QPixmap pixmap(Core::path(Path_Icons) + "promo.png");
  QSplashScreen splash(pixmap);
  splash.show();

  time_t t0 = time(NULL);

  a.processEvents();

  a.addLibraryPath(Core::path(Path_Plugins));

  //--------------Check if old settings are present--------------
  Util::checkOldSettings();

  //-------------------------Traduction--------------------------
  //    QString     language = QSettings().value("General/Language").toString();
  //    QTranslator appTranslator,
  //                libTranslator,
  //                qtTranslator;

  //    appTranslator.load(Core::path(Path_Translations) + "camseg_" + language
  //    + ".qm"); libTranslator.load(Core::path(Path_Translations) +
  //    "libcamseg_" + language + ".qm"); qtTranslator. load(QString("qt_") +
  //    language, QLibraryInfo::location(QLibraryInfo::TranslationsPath));

  //    a.installTranslator(&appTranslator);
  //    a.installTranslator(&libTranslator);
  //    a.installTranslator(&qtTranslator);

  //--------------------Application Starting---------------------
  MainWindow* w = KLib::Core::instance()->mainWindow();
  ActionManager::instance();
  SettingsManager::instance();

  // Load the plugins
  Util::loadPlugins();

  while (static_cast<int>(time(NULL) - t0) < SPLASH_SCREEN_TIME) {
  }

  splash.finish(w);
  w->show();

  return a.exec();
}

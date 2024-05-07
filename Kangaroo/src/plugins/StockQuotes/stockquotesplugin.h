#ifndef STOCKQUOTESPLUGIN_H
#define STOCKQUOTESPLUGIN_H

#include <KangarooLib/iplugin.h>

#include <QObject>

class StockQuotesPlugin : public QObject, public KLib::IPlugin {
  Q_OBJECT
  Q_PLUGIN_METADATA(IID "Kangaroo.IPlugin/1.0")
  Q_INTERFACES(KLib::IPlugin)

 public:
  StockQuotesPlugin();

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

#endif  // STOCKQUOTESPLUGIN_H

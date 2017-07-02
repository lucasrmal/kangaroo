#ifndef TAXREPORTER_H
#define TAXREPORTER_H

#include <QObject>
#include <QMap>
#include <QLinkedList>
#include <QMetaType>
#include <QDate>
#include <KangarooLib/model/transaction.h>
#include <KangarooLib/interfaces/scriptable.h>

namespace KLib
{
    class Transaction;
}

typedef QPair<int, KLib::Transaction*> TaxLineItem;
typedef QMap<QString, QList<TaxLineItem> > TaxReport;

class TaxReporter : public QObject
{
    Q_OBJECT
    K_SCRIPTABLE(TaxReporter)

        TaxReporter() {}

    public:
        static TaxReporter* instance();

        /**
         * @brief Returns the list of transactions, for each tax item, between _from and _to inclusive.
         *
         * @return For each category, sorted byu alphabetical order, the list of transactions (Account ID, Transaction)
         */
        Q_INVOKABLE TaxReport taxReportFor(const QDate& _from, const QDate& _to) const;


    private:
        static TaxReporter* m_instance;
};


class TaxReporterScriptable : public KLib::Scriptable
{
    public:
        static QScriptValue taxReportToScriptValue(QScriptEngine* eng, const TaxReport& _report)
        {
          QScriptValue a = eng->newObject();

          for (auto i = _report.begin(); i != _report.end(); ++i)
          {
              QScriptValue list = eng->newObject();

              int index = 0;
              for (const TaxLineItem& item : *i)
              {
                  QScriptValue pair = eng->newObject();

                  pair.setProperty(0, eng->newVariant(item.first));
                  pair.setProperty(1, eng->newQObject(item.second));


                  list.setProperty(index, pair);
                  ++index;
              }

              list.setProperty("length", index);

              a.setProperty(i.key(), list);
          }

          return a;
        }

        static void taxReportFromScriptValue(const QScriptValue& value, TaxReport& _report)
        {
          QScriptValueIterator i(value);
          _report.clear();

          while (i.hasNext())
          {
              i.next();
              QList<TaxLineItem> list;

              QScriptValueIterator j(i.value());

              while (j.hasNext())
              {
                  QScriptValue v = j.value();
                  list.append(TaxLineItem(v.property(0).toInteger(), qobject_cast<KLib::Transaction*>(v.property(1).toQObject())));
              }

              _report.insert(i.name(), list);
          }
        }

        void addToEngine(QScriptEngine* _engine) const
        {
            qScriptRegisterMetaType(_engine, taxReportToScriptValue, taxReportFromScriptValue);
        }
};

Q_DECLARE_METATYPE(TaxLineItem)
Q_DECLARE_METATYPE(TaxReport)
Q_DECLARE_METATYPE(TaxReporter*)

#endif // TAXREPORTER_H

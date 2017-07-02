#ifndef YAHOOQUOTE_H
#define YAHOOQUOTE_H

#include <KangarooLib/interfaces/iquote.h>

class QNetworkReply;
class QNetworkAccessManager;

class YahooQuote : public KLib::IQuote
{
    Q_OBJECT

    public:
        explicit YahooQuote(QObject *parent = 0);

        virtual ~YahooQuote() {}

        virtual QString name() const  { return tr("Yahoo!"); }
        virtual QString id() const    { return "Yahoo"; }

        virtual int makeRequest(Type _type, const QList<KLib::PricePair>& _pairs);

    private slots:
        void onReply(QNetworkReply* _reply);

    private:

        QNetworkAccessManager* m_manager;

        QHash<int, Type> m_types;
        QHash<QString, KLib::SecurityInfo> m_yahooToSecurityInfo;

};

#endif // YAHOOQUOTE_H

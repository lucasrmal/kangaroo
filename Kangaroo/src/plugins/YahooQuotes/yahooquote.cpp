#include "yahooquote.h"

#include <QNetworkAccessManager>
#include <QUrl>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QDebug>
#include <KangarooLib/controller/io.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/model/pricemanager.h>

using namespace KLib;

YahooQuote::YahooQuote(QObject *parent) :
    IQuote(parent),
    m_manager(new QNetworkAccessManager(this))
{
    //Yahoo name to security info
    m_yahooToSecurityInfo["Ask"] = SecurityInfo::AskPrice;
    m_yahooToSecurityInfo["Bid"] = SecurityInfo::BidPrice;
    m_yahooToSecurityInfo["Open"] = SecurityInfo::OpenPrice;
    m_yahooToSecurityInfo["PreviousClose"] = SecurityInfo::PreviousClosePrice;

    m_yahooToSecurityInfo["AverageDailyVolume"] = SecurityInfo::AverageDailyVolume;
    m_yahooToSecurityInfo["Volume"] = SecurityInfo::Volume;

    m_yahooToSecurityInfo["BookValue"] = SecurityInfo::BookValuePerShare;
    m_yahooToSecurityInfo["DividendShare"] = SecurityInfo::DividendPerShare;
    m_yahooToSecurityInfo["DividendYield"] = SecurityInfo::DividendYield;
    m_yahooToSecurityInfo["EarningsShare"] = SecurityInfo::EPS;
    m_yahooToSecurityInfo["EPSEstimateCurrentYear"] = SecurityInfo::EPSEstimateCurYear;
    m_yahooToSecurityInfo["EPSEstimateNextYear"] = SecurityInfo::EPSEstimateNextYear;
    m_yahooToSecurityInfo["EBITDA"] = SecurityInfo::EBITDA;

    m_yahooToSecurityInfo["PriceBook"] = SecurityInfo::PriceToBook;
    m_yahooToSecurityInfo["PriceSales"] = SecurityInfo::PriceToSales;
    m_yahooToSecurityInfo["PEGRatio"] = SecurityInfo::PEGRatio;
    m_yahooToSecurityInfo["ShortRatio"] = SecurityInfo::ShortRatio;
    m_yahooToSecurityInfo["PERatio"] = SecurityInfo::PERatio;

    m_yahooToSecurityInfo["DividendPayDate"] = SecurityInfo::DividendPayDate;
    m_yahooToSecurityInfo["ExDividendDate"] = SecurityInfo::ExDividendDate;

    m_yahooToSecurityInfo["MarketCapitalization"] = SecurityInfo::MarketCap; //***

    m_yahooToSecurityInfo["Change"] = SecurityInfo::DayChange;
    m_yahooToSecurityInfo["DaysLow"] = SecurityInfo::DayLow;
    m_yahooToSecurityInfo["DaysHigh"] = SecurityInfo::DayHigh;
    m_yahooToSecurityInfo["YearLow"] = SecurityInfo::YearLow;
    m_yahooToSecurityInfo["YearHigh"] = SecurityInfo::YearHigh;
    m_yahooToSecurityInfo["FiftydayMovingAverage"] = SecurityInfo::FiftyDayAverage;
    m_yahooToSecurityInfo["TwoHundreddayMovingAverage"] = SecurityInfo::TwoHundredDayAverage;

    connect(m_manager, &QNetworkAccessManager::finished, this, &YahooQuote::onReply);
}

int YahooQuote::makeRequest(Type _type, const QList<KLib::PricePair>& _pairs)
{
    if (_pairs.isEmpty())
        return -1;

    QString query;

    if (_type == Currency)
    {
        QString strPairs;

        for (KLib::PricePair p : _pairs)
        {
            strPairs += QString("\"%1%2\",").arg(p.first).arg(p.second);
        }

        strPairs.resize(strPairs.size()-1);

        query = QString("select * from yahoo.finance.xchange where pair in (%1)").arg(strPairs);
    }
    else //Stock
    {
        QString strPairs;

        for (KLib::PricePair p : _pairs)
        {
            strPairs += QString("\"%1\",").arg(p.first);
        }

        strPairs.resize(strPairs.size()-1);

        query = QString("select * from yahoo.finance.quotes where symbol in (%1)").arg(strPairs);
    }

    QUrl url(QString("https://query.yahooapis.com/v1/public/yql?q=%1&env=store://datatables.org/alltableswithkeys&format=xml").arg(query));

    //qDebug() << QString("https://query.yahooapis.com/v1/public/yql?q=%1&env=store://datatables.org/alltableswithkeys&format=xml").arg(query);
    int num = rand();
    m_types[num] = _type;
    QNetworkReply* reply = m_manager->get(QNetworkRequest(url));

    reply->setObjectName(QString::number(num));
    return num;
}

void YahooQuote::onReply(QNetworkReply* _reply)
{
    if (!_reply)
        return;

    int queryId = _reply->objectName().toInt();

    if (_reply->error() != QNetworkReply::NoError)
    {
        emit requestError(queryId, _reply->errorString());
        _reply->deleteLater();
        return;
    }

    QXmlStreamReader xml(_reply);
    QString from, to;
    double rate;
    QHash<SecurityInfo, Amount> extraInfos;

    while (!xml.atEnd() && !xml.hasError())
    {
       if (xml.name() == "error")
       {
           while (xml.name() != "description")
               xml.readNext();

           // Current is description, we now read its text
           xml.readNext();

           emit requestError(queryId, xml.text().toString());

           _reply->deleteLater();
           return;
       }

       if (m_types[queryId] == Currency)
       {
           if (xml.name() == "rate" && xml.tokenType() == QXmlStreamReader::StartElement)
           {
               QXmlStreamAttributes att = xml.attributes();
               QString fromto = KLib::IO::getAttribute("id", att);
               from = fromto.left(3);
               to = fromto.right(3);
           }
           else if (xml.name() == "Rate" && xml.tokenType() == QXmlStreamReader::StartElement)
           {
               xml.readNext();
               rate = xml.text().toDouble();
           }
           else if (xml.name() == "rate" && xml.tokenType() == QXmlStreamReader::EndElement)
           {
               //Emit the results
               emit quoteReady(m_types[queryId], KLib::PricePair(from, to), rate);
               from = to = "";
               rate = 0;
           }
       }
       else // Stock
       {
           if (xml.name() == "quote" && xml.tokenType() == QXmlStreamReader::StartElement)
           {
               QXmlStreamAttributes att = xml.attributes();
               extraInfos.clear();
               from = KLib::IO::getAttribute("symbol", att);
           }
           else if (xml.name() == "Currency" && xml.tokenType() == QXmlStreamReader::StartElement)
           {
               xml.readNext();
               to = xml.text().toString();
           }
           else if (xml.name() == "LastTradePriceOnly" && xml.tokenType() == QXmlStreamReader::StartElement)
           {
               xml.readNext();
               rate = xml.text().toDouble();
           }
           else if (xml.name() == "quote" && xml.tokenType() == QXmlStreamReader::EndElement)
           {
               //Emit the results
               if (!from.isEmpty() && !to.isEmpty())
               {
                    emit quoteReady(m_types[queryId], KLib::PricePair(from, to), rate, extraInfos);
               }
               from = to = "";
               rate = 0;
           }
           else if (xml.tokenType() == QXmlStreamReader::StartElement)
           {
               QString name = xml.name().toString();
               xml.readNext();

               if (m_yahooToSecurityInfo.contains(name))
               {
                   extraInfos[m_yahooToSecurityInfo[name]] = Amount(xml.text().toDouble());
               }

           }
       }

       xml.readNext();
    }

    _reply->deleteLater();
    m_types.remove(queryId);
    emit requestDone(queryId);
}

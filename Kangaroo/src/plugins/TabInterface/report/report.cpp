#include "report.h"
#include <QXmlStreamReader>
#include <QFile>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/controller/io.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/model/payee.h>
#include <KangarooLib/model/institution.h>
#include <KangarooLib/controller/reportgenerator.h>
#include <QMessageBox>
#include <QDate>

using namespace KLib;

const QString DefaultValue::CURRENT_MONTH = "CURRENT_MONTH";
const QString DefaultValue::CURRENT_YEAR = "CURRENT_YEAR";
const QString DefaultValue::CURRENT_DATE = "CURRENT_DATE";

namespace SettingType
{
    int stringToType(const QString& _type)
    {
        if (_type == "string")
        {
            return String;
        }
        else if (_type == "date")
        {
            return Date;
        }
        else if (_type == "year")
        {
            return Year;
        }
        else if (_type == "month")
        {
            return Month;
        }
        else if (_type == "boolean")
        {
            return Boolean;
        }
        else if (_type == "double")
        {
            return Double;
        }
        else if (_type == "integer")
        {
            return Integer;
        }
        else if (_type == "list")
        {
            return List;
        }
        else if (_type == "account")
        {
            return Account;
        }
        else if (_type == "payee")
        {
            return Payee;
        }
        else if (_type == "institution")
        {
            return Institution;
        }
        else if (_type == "security")
        {
            return Security;
        }

        return -1;
    }
}

QString ReportSetting::valueToCode() const
{
    switch (type)
    {
    case SettingType::String:
    case SettingType::List:
        return QString("\"%1\"").arg(value.toString());

    case SettingType::Date:
        return QString("new Date(%1, %2, %3)").arg(value.toDate().year())
                                              .arg(value.toDate().month()-1)
                                              .arg(value.toDate().day());

    case SettingType::Boolean:
        return value.toBool() ? "true" : "false";

    case SettingType::Year:
    case SettingType::Month:
    case SettingType::Integer:
        return QString::number(value.toInt());

    case SettingType::Double:
        return QString::number(value.toDouble());

    case SettingType::Account:
        return value.toInt() == Account::getTopLevel()->id() ?
                    QString("new Account()")
                  : QString("new Account().getChild(%1)").arg(value.toInt());

    case SettingType::Payee:
        return QString("new PayeeManager().get(%1)").arg(value.toInt());

    case SettingType::Security:
        return QString("new SecurityManager().get(%1)").arg(value.toInt());

    case SettingType::Institution:
        return QString("new InstitutionManager().get(%1)").arg(value.toInt());
    }

    return "";
}

enum Flags
{
    Flag_Toplevel   = 0x01,
    Flag_Asset      = 0x02, //Excludes investments
    Flag_Liability  = 0x04,
    Flag_Equity     = 0x08,
    Flag_Income     = 0x10,
    Flag_Expense    = 0x20,
    Flag_Trading    = 0x40,
    Flag_Investment = 0x80,

    Flag_All        = 0xFF,
    Flag_None       = 0x00,
    Flag_IncomeExpense = Flag_Income | Flag_Expense,
    Flag_AllAssets = Flag_Asset | Flag_Investment,
    Flag_AllButInvTrad = Flag_All & ~(Flag_Investment | Flag_Trading)
};

int ReportSetting::flagsToAccountFlags() const
{
    int flag = 0;

    QHash<QString, int> strToFlag;

    strToFlag["toplevel"]       = AccountTypeFlags::Flag_Toplevel;
    strToFlag["asset"]          = AccountTypeFlags::Flag_Asset;
    strToFlag["liability"]      = AccountTypeFlags::Flag_Liability;
    strToFlag["equity"]         = AccountTypeFlags::Flag_Equity;
    strToFlag["income"]         = AccountTypeFlags::Flag_Income;
    strToFlag["expense"]        = AccountTypeFlags::Flag_Expense;
    strToFlag["trading"]        = AccountTypeFlags::Flag_Trading;
    strToFlag["investment"]     = AccountTypeFlags::Flag_Investment;
    strToFlag["all"]            = AccountTypeFlags::Flag_All;
    strToFlag["none"]           = AccountTypeFlags::Flag_None;
    strToFlag["incomeexpense"]  = AccountTypeFlags::Flag_IncomeExpense;
    strToFlag["allassets"]      = AccountTypeFlags::Flag_AllAssets;
    strToFlag["allbutinvtrad"]  = AccountTypeFlags::Flag_AllButInvTrad;

    for (QString s : flags)
    {
        if (strToFlag.contains(s.toLower()))
        {
            flag |= strToFlag[s];
        }
    }

    return flag;
}

Report::Report()
{
}

Report* Report::loadFromFile(const QString& _path)
{
    QFile file(_path);

    // If we can't open it, throw an error message.
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
       qDebug() << QObject::tr("Unable to open report file: %1").arg(_path);
       return NULL;
    }

    QXmlStreamReader xml(&file);

    Report* r = new Report();

    if (xml.readNextStartElement())
    {
        if (xml.name() != "report")
        {
            qDebug() << QObject::tr("This is not a report file: %1").arg(_path);
        }

        while(!xml.atEnd() && !xml.hasError())
        {
           if (xml.tokenType() == QXmlStreamReader::StartElement)
           {
               if (xml.name() == "name")
               {
                   xml.readNext();
                   r->name = xml.text().toString();
               }
               else if (xml.name() == "description")
               {
                   xml.readNext();
                   r->description = xml.text().toString();
               }
               else if (xml.name() == "version")
               {
                   xml.readNext();
                   r->version = xml.text().toString();
               }
               else if (xml.name() == "icon")
               {
                   xml.readNext();
                   r->icon = xml.text().toString();
               }
               else if (xml.name() == "author")
               {
                   xml.readNext();
                   r->author = xml.text().toString();
               }
               else if (xml.name() == "content")
               {
                   xml.readNext();
                   r->path = xml.text().toString();
               }
               else if (xml.name() == "settings")
               {
                   xml.readNext();

                   while (!(xml.name() == "settings" && xml.tokenType() == QXmlStreamReader::EndElement))
                   {
                       if (xml.name() == "setting" && xml.tokenType() == QXmlStreamReader::StartElement)
                       {
                           QXmlStreamAttributes a = xml.attributes();
                           ReportSetting s;

                           try
                           {
                               s.type = SettingType::stringToType(IO::getAttribute("type", a));

                               if (s.type == -1)
                               {
                                   qDebug() << QObject::tr("In %1: Invalid setting type.").arg(_path);
                                   return NULL;
                               }

                               s.description = IO::getAttribute("description", a);
                               s.codeId = IO::getAttribute("codeid", a);
                               s.defaultVal = IO::getOptAttribute("default", a);

                               if (s.defaultVal == DefaultValue::CURRENT_MONTH)
                               {
                                   s.defaultVal = QString::number(QDate::currentDate().month());
                               }
                               else if (s.defaultVal == DefaultValue::CURRENT_YEAR)
                               {
                                   s.defaultVal = QString::number(QDate::currentDate().year());
                               }
                               else if (s.defaultVal == DefaultValue::CURRENT_DATE)
                               {
                                   s.defaultVal = QDate::currentDate().toString(Qt::ISODate);
                               }

                               if (s.type == SettingType::List)
                               {
                                   s.list = IO::getAttribute("choices", a).split(',');
                               }

                               if (s.type == SettingType::Account)
                               {
                                   s.allowPlaceholders = IO::getOptAttribute("placeholder", a, "true") == "true";
                                   s.flags = IO::getOptAttribute("flags", a, "all").split(',');
                               }

                               r->settings << s;
                           }
                           catch (IOException e)
                           {
                               qDebug() << QObject::tr("In %1: %2").arg(_path).arg(e.description());
                               return NULL;
                           }
                       }
                       xml.readNext();
                   }

               }
               else
               {
                   xml.readNext();
               }
           }
           else
           {
               xml.readNext();
           }
        }

    }

    /* Error handling. */
    if(xml.hasError())
    {
       qDebug() << QObject::tr("Error in %1: %2").arg(_path).arg(xml.errorString());
       delete r;
       r = NULL;
    }
    else if (r->name.isEmpty() || r->path.isEmpty())
    {
        qDebug() << QObject::tr("Invalid report file: %1").arg(_path);
        delete r;
        r = NULL;
    }

    xml.clear();
    file.close();

    return r;
}

bool Report::requiresManualSettings(const QList<ReportSetting>& _settings)
{
    for (ReportSetting s : _settings)
    {
        if (s.defaultVal.isEmpty())
            return true;
    }

    return false;
}

void Report::setSettingsToDefault(QList<ReportSetting>& _settings)
{
    for (ReportSetting& s : _settings)
    {
        if (!s.defaultVal.isEmpty())
        {
            switch (s.type)
            {
            case SettingType::String:
            case SettingType::List:
                s.value = s.defaultVal;
                break;

            case SettingType::Date:
                s.value = QDate::fromString(s.defaultVal, Qt::ISODate);
                break;

            case SettingType::Boolean:
                s.value = s.defaultVal == "true" ? true : false;
                break;

            case SettingType::Year:
            case SettingType::Month:
            case SettingType::Integer:
            case SettingType::Account:
            case SettingType::Security:
            case SettingType::Institution:
            case SettingType::Payee:
                s.value = s.defaultVal.toInt();
                break;

            case SettingType::Double:
                s.value = s.defaultVal.toDouble();
                break;

            }
        }
    }
}

QString Report::generateHtml(const ReportSettings& _settings) const
{
    QString settingStatement = "var Settings = {";

    for (int i = 0; i < _settings.count(); ++i)
    {
        settingStatement += QString("%1: %2").arg(_settings[i].codeId)
                                             .arg(_settings[i].valueToCode());

        if (i < _settings.count()-1) settingStatement += ",\n";
    }

    settingStatement += "\n}\n";

    //Generate the HTML
    QString html;

    try
    {
        html = ReportGenerator::generateHtml(path, settingStatement);
    }
    catch (IOException e)
    {
        QMessageBox::information(Core::instance()->mainWindow(),
                                 QObject::tr("Open Report"),
                                 QObject::tr("An error has occured while compiling the report file:\n\n%1").arg(e.description()));
        return QString();
    }

    return html;
}






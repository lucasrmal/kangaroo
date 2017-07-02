#ifndef REPORT_H
#define REPORT_H

#include <QString>
#include <QVariant>
#include <QList>

namespace SettingType
{
    enum Type
    {
        String,
        Date,
        Year,
        Month,
        Boolean,
        Integer,
        Double,
        List,
        Account,
        Payee,
        Institution,
        Security
    };

    int stringToType(const QString& _type);
}

struct DefaultValue
{
    static const QString CURRENT_YEAR;
    static const QString CURRENT_MONTH;
    static const QString CURRENT_DATE;
};

struct ReportSetting
{
    int type;
    QString description;
    QString defaultVal;
    QString codeId;
    QVariant value;
    QStringList list;
    QStringList flags;

    QString valueToCode() const;

    int flagsToAccountFlags() const;

    bool allowPlaceholders;         //This field is for Account type
};

typedef QList<ReportSetting> ReportSettings;

class Report
{
    public:
        Report();

        static Report* loadFromFile(const QString &_path);

        static bool requiresManualSettings(const QList<ReportSetting>& _settings);

        static void setSettingsToDefault(QList<ReportSetting>& _settings);

        QString generateHtml(const ReportSettings& _settings) const;

        QString name;
        QString author;
        QString version;
        QString description;
        QString icon;
        QString path;

        QList<ReportSetting> settings;

};

#endif // REPORT_H

#ifndef ICHART_H
#define ICHART_H

#include <QWidget>
#include <QIcon>
#include <functional>

class QPrinter;

class IChart : public QWidget
{
    Q_OBJECT

    public:
        explicit IChart(QWidget *parent = nullptr);

        /**
         * @brief Prints the document on the specified printer _printer.
         *
         * The printer settings are already configured
         */
        virtual void print(const QPrinter& _printer) const = 0;

        /**
         * @brief Saves the chart to a PDF document indicated by _path
         */
        virtual void saveAs(const QString& _path) const = 0;

    public slots:
        /**
         * @brief Refreshed the chart
         */
        virtual void refresh() = 0;


};

struct ChartInfo
{
    typedef std::function<IChart*(QWidget*)> fn_build;

    ChartInfo() :
        build([](QWidget*) { return nullptr; }) {}

    ChartInfo(const QString& _code,
              const QString& _name,
              const QIcon& _icon,
                   const fn_build& _build) :
        code(_code),
        name(_name),
        icon(_icon),
        build(_build) {}


    QString  code;          ///< Unique code
    QString  name;          ///< Name of the chart, used for the report browser and as title of the tab.
    QIcon    icon;          ///< Icon representing the chart
    fn_build build;         ///< Takes the parent widget in parameter and returs the chart
};

#endif // ICHART_H

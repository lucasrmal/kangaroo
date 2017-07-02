#ifndef REPORTVIEWER_H
#define REPORTVIEWER_H

#include <QWidget>
#include <QPrinter>
#include "report.h"

class QWebView;

class ReportViewer : public QWidget
{
    Q_OBJECT
    public:
        explicit ReportViewer(Report* _report, const ReportSettings& _settings, QWidget *parent = 0);

        void setContent(const QString& _htmlContent);

    public slots:
        void print();
        void saveAs();
        void reload();

    private:
        QWebView* m_view;
        QString m_html;

        Report* m_report;
        ReportSettings m_settings;

        static QString  m_lastPath;
        static QPrinter* m_printer;

};

#endif // REPORTVIEWER_H

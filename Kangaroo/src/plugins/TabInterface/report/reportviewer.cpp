#include "reportviewer.h"

//#include <QWebView>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QPrintDialog>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QPushButton>
#include <KangarooLib/ui/core.h>
#include <QLabel>

using namespace KLib;

/*
   Convert to ODF

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
    "untitled",tr("Open Document ('''.odf)"));
    QTextDocumentWriter odfWritter(fileName);
    odfWritter.write(doc); // doc is QTextDocument*
*/

QPrinter* ReportViewer::m_printer = new QPrinter();
QString ReportViewer::m_lastPath = QString();

ReportViewer::ReportViewer(Report* _report, const ReportSettings& _settings, QWidget *parent) :
    QWidget(parent),
    m_report(_report),
    m_settings(_settings)
{
    QPushButton* btnPrint = new QPushButton(Core::icon("print"), "", this);
    QPushButton* btnSaveAs = new QPushButton(Core::icon("save-as"), "", this);
    QPushButton* btnReload = new QPushButton(Core::icon("view-refresh"), "", this);

    btnPrint->setFlat(true);
    btnSaveAs->setFlat(true);
    btnReload->setFlat(true);

//    m_view = new QWebView(this);
//    m_view->settings()->setUserStyleSheetUrl(QUrl::fromLocalFile(Core::path(Path_Themes) + "report_style.css"));

    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(2);
    buttonLayout->addWidget(btnPrint);
    buttonLayout->addWidget(btnSaveAs);
    buttonLayout->addWidget(btnReload);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(buttonLayout);
//    layout->addWidget(m_view, 1);

    connect(btnPrint,  &QPushButton::clicked, this, &ReportViewer::print);
    connect(btnSaveAs, &QPushButton::clicked, this, &ReportViewer::saveAs);
    connect(btnReload, &QPushButton::clicked, this, &ReportViewer::reload);
}

void ReportViewer::setContent(const QString& _htmlContent)
{
    m_html = _htmlContent;
//    m_view->setHtml(_htmlContent);
}

void ReportViewer::reload()
{
    QString newHtml = m_report->generateHtml(m_settings);

    if (!newHtml.isEmpty())
    {
        setContent(newHtml);
    }
}

void ReportViewer::print()
{
    QPrintDialog p(m_printer, this);

    if (p.exec() == QDialog::Accepted)
    {
//        m_view->print(m_printer);
    }
}

void ReportViewer::saveAs()
{
    QString selected;
    QString path = QFileDialog::getSaveFileName(this,
                                                tr("Save To File"),
                                                m_lastPath,
                                                tr("PDF (*.pdf)") + ";;" + tr("HTML (*.html)"),
                                                &selected);


    if (!path.isEmpty())
    {
        if (selected.contains("pdf"))
        {
            if (!path.endsWith(".pdf"))
            {
                path += ".pdf";
            }

            QPrinter printer;
            printer.setPageMargins(1.0, 1.0, 1.0, 1.3, QPrinter::Inch);
            printer.setOutputFormat(QPrinter::PdfFormat);
            printer.setOutputFileName(path);
//            m_view->print(&printer);
        }
        else //html
        {
            if (!path.endsWith(".html"))
            {
                path += ".html";
            }
            QFile f(path);

            if (!f.open(QIODevice::Truncate))
            {
                QMessageBox::warning(this,
                                     tr("Save To HTML"),
                                     tr("Unable to save the file. Check the permissions."));
                return;
            }

            QTextStream str(&f);
            str << m_html;
            f.close();
        }

        m_lastPath = path;
    }
}

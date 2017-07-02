/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2008-2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  US
 */

#include <poppler/qt5/poppler-qt5.h>
#include <QFile>
#include <QDir>
#include <QPainter>
#include <QPrinter>
#include <QScrollArea>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QIntValidator>
#include "pdfviewer.h"
#include "../core.h"

namespace KLib
{

    PDFViewer::PDFViewer(QWidget* parent) : QWidget(parent),
                                            m_document(NULL),
                                            m_currentPage(NULL),
                                            m_currentIndex(0),
                                            m_scaleFactor(1.0)
    {
        QVBoxLayout* lay = new QVBoxLayout();

        m_area = new QScrollArea();
        m_label = new QLabel();
        m_txtCurPage = new QLineEdit(this);
        m_lblNumPages = new QLabel(this);
        m_btnNext = new QPushButton(Core::icon("1rightarrow"), "", this);
        m_btnPrevious = new QPushButton(Core::icon("1leftarrow"), "", this);
        m_validator = new QIntValidator(this);

        m_txtCurPage->setValidator(m_validator);
        m_validator->setBottom(1);

        m_btnNext->setFlat(true);
        m_btnPrevious->setFlat(true);
        m_btnNext->setMaximumWidth(30);
        m_btnPrevious->setMaximumWidth(30);
        m_txtCurPage->setMaximumWidth(35);

        m_area->setWidget(m_label);
        m_area->setBackgroundRole(QPalette::Dark);
        m_label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

        QHBoxLayout* commandLayout = new QHBoxLayout();
        commandLayout->addStretch(2);
        commandLayout->addWidget(m_btnPrevious);
        commandLayout->addWidget(m_txtCurPage);
        commandLayout->addWidget(m_lblNumPages);
        commandLayout->addWidget(m_btnNext);
        commandLayout->addStretch(2);

        lay->addWidget(m_area);
        lay->addLayout(commandLayout);
        setLayout(lay);

        connect(m_btnNext, SIGNAL(clicked()), this, SLOT(nextPage()));
        connect(m_btnPrevious, SIGNAL(clicked()), this, SLOT(previousPage()));
        connect(m_txtCurPage, SIGNAL(returnPressed()), this, SLOT(onTxtPageEdited()));
    }

    PDFViewer::~PDFViewer()
    {
        delete m_document;
        delete m_currentPage;
    }

    bool PDFViewer::setFile(const QString& _path)
    {
        //Load it in the Document
        Poppler::Document* temp = Poppler::Document::load(_path);

        //Check if it's loaded OK
        if ( !temp || temp->isLocked())
        {
            delete m_document;
            m_document = NULL;
            return false;
        }

        temp->setRenderHint(Poppler::Document::Antialiasing);
        temp->setRenderHint(Poppler::Document::TextAntialiasing);

        delete m_document;          //Delete the old document
        m_document = temp;          //Replace by the new data

        m_lblNumPages->setText(tr("of %1").arg(m_document->numPages()));
        m_validator->setTop(m_document->numPages());

        return setCurrentPage(0);   //Set the current page to 0
    }

    void PDFViewer::closeFile()
    {
        setPixmap(QPixmap());
    }

    int PDFViewer::pageCount() const
    {
        return m_document ? m_document->numPages() : 0;
    }

    bool PDFViewer::setCurrentPage(int _page)
    {
        if (m_document)
        {
            //Generate the page
            Poppler::Page* pdfPage = m_document->page(_page);

            if ( ! pdfPage)
                return false;

            //Delete the current page
            delete m_currentPage;

            //Set the current page
            m_currentPage = pdfPage;
            m_currentIndex = _page;

            //Set the picture
            QImage img = m_currentPage->renderToImage(m_scaleFactor * physicalDpiX(), m_scaleFactor * physicalDpiY());
            //img.save("/home/lucas/ToDelete/test.png");
            setPixmap(QPixmap::fromImage(img));

            m_txtCurPage->setText(QString::number(_page+1));
            m_btnNext->setEnabled(hasNextPage());
            m_btnPrevious->setEnabled(hasPreviousPage());

            return true;
        }
        else return false;
    }

    bool PDFViewer::hasNextPage()
    {
        return m_currentIndex < pageCount() - 1;
    }

    bool PDFViewer::hasPreviousPage()
    {
        return m_currentIndex > 0;
    }

    void PDFViewer::nextPage()
    {
        if (hasNextPage())
            setCurrentPage(m_currentIndex+1);
    }

    void PDFViewer::previousPage()
    {
        if (hasPreviousPage())
            setCurrentPage(m_currentIndex-1);
    }

    void PDFViewer::onTxtPageEdited()
    {
        bool ok;
        int page = m_txtCurPage->text().toInt(&ok);

        if (ok && page <= pageCount() && page > 0)
        {
            setCurrentPage(page-1);
        }
    }

    void PDFViewer::setScaleFactor(float _scale)
    {
        if (m_currentPage && _scale > 0.0 && _scale <= 4.0)
        {
            m_scaleFactor = _scale;
            setPixmap(QPixmap::fromImage(m_currentPage->renderToImage(m_scaleFactor * physicalDpiX(), m_scaleFactor * physicalDpiY())));
        }
    }

    void PDFViewer::resetScaleFactor()
    {
        setScaleFactor(1.0);
    }

    float PDFViewer::scaleFactor() const
    {
        return m_scaleFactor;
    }

    QPixmap PDFViewer::pageThumbnail() const
    {
        return m_currentPage ? QPixmap::fromImage(m_currentPage->thumbnail()) : QPixmap();
    }

    bool PDFViewer::print(QPrinter* p_printer) const
    {
        if (m_currentPage == NULL)
            return false;

        QImage image(m_currentPage->renderToImage(m_scaleFactor * physicalDpiX(), m_scaleFactor * physicalDpiY()));

        QPainter painter(p_printer);
        QRect rect = painter.viewport();
        QSize size = image.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(image.rect());
        painter.drawImage(0, 0, image);

        return true;
    }

    void PDFViewer::setPixmap(const QPixmap& _pixmap)
    {
        m_label->setPixmap(_pixmap);
        m_label->setFixedSize(_pixmap.size());
    }

}

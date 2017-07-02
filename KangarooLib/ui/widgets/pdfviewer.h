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

#ifndef REPORTVIEWER_H
#define REPORTVIEWER_H

#include <QWidget>

class QPrinter;
class QScrollArea;
class QLabel;
class QPushButton;
class QLineEdit;
class QIntValidator;

namespace Poppler
{
    class Document;
    class Page;
}

namespace KLib
{
    class PDFViewer : public QWidget
    {
        Q_OBJECT

        public:
                            PDFViewer(QWidget* parent = NULL);
                            ~PDFViewer();

            bool            setFile(const QString& _path);

            void            closeFile();

            int             pageCount() const;
            int             currentPage() const { return m_currentIndex; }

            void            setScaleFactor(float _scale);
            void            resetScaleFactor();
            float           scaleFactor() const;

            QPixmap         pageThumbnail() const;

            bool            print(QPrinter* p_printer) const;

            bool            hasNextPage();
            bool            hasPreviousPage();

        public slots:
            bool            setCurrentPage(int _page);
            void            nextPage();
            void            previousPage();

        private slots:
            void            onTxtPageEdited();


        private:
            void            setPixmap(const QPixmap& _pixmap);

            QScrollArea*        m_area;
            QLabel*             m_label;
            QLineEdit*          m_txtCurPage;
            QLabel*             m_lblNumPages;
            QPushButton*        m_btnNext;
            QPushButton*        m_btnPrevious;
            QIntValidator*      m_validator;

            Poppler::Document*  m_document;
            Poppler::Page*      m_currentPage;
            int                 m_currentIndex;
            float               m_scaleFactor;
    };

}

#endif // REPORTVIEWER_H

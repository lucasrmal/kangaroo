/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
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

#include "formpicturemanager.h"
#include "../../controller/picturecontroller.h"
#include "../../model/picturemanager.h"
#include "../core.h"
#include "../../klib.h"

#include <QListView>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QFormLayout>
#include <QMessageBox>
#include <QSortFilterProxyModel>
#include <QFileDialog>

namespace KLib
{
    QString LAST_FOLDER = "";

    FormPictureManager::FormPictureManager(QWidget *parent) :
        CAMSEGDialog(DialogWithPicture, CloseButton, parent),
        m_inSelect(false),
        m_selectedNone(false)
    {
        setupUI();
        setBothTitles(tr("Picture Manager"));
    }

    FormPictureManager::FormPictureManager(int _current, PictureSelectionMode _mode, QWidget *parent) :
        CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
        m_inSelect(true),
        m_selectedNone(false)
    {
        setupUI();
        setBothTitles(tr("Select Picture"));

        selectPicture(_current);

        if (_mode == AllowNoPicture)
        {
            QPushButton* btnNone = new QPushButton(Core::icon("clear"), tr("Select &None"), this);
            connect(btnNone, &QPushButton::clicked, this, &FormPictureManager::selectNone);
            addButton(btnNone, 1, AtRight);
        }

        connect(m_view, &QListView::doubleClicked, this, &FormPictureManager::accept);
    }

    void FormPictureManager::selectPicture(int _id)
    {
        if (_id != Constants::NO_ID)
        {
            for (int i = 0; i < m_view->model()->rowCount(); ++i)
            {
                QModelIndex cur = m_view->model()->index(i, 0);

                if (m_view->model()->data(cur, Qt::UserRole).toInt() == _id)
                {
                    m_view->setCurrentIndex(cur);
                    break;
                }
            }
        }
    }

    void FormPictureManager::setupUI()
    {
        setPicture(Core::pixmap("picture-manager"));

        m_view = new QListView(this);

        QPushButton* btnAdd = new QPushButton(Core::icon("add"), tr("&Add"), this);
        QPushButton* btnEdit = new QPushButton(Core::icon("modify"), tr("&Edit"), this);
        QPushButton* btnRemove = new QPushButton(Core::icon("remove"), tr("&Remove"), this);

        connect(btnAdd,     &QPushButton::clicked, this, &FormPictureManager::addPicture);
        connect(btnEdit,    &QPushButton::clicked, this, &FormPictureManager::editPicture);
        connect(btnRemove,  &QPushButton::clicked, this, &FormPictureManager::removePicture);

        addButton(btnAdd,    0, AtLeft);
        addButton(btnEdit,   1, AtLeft);
        addButton(btnRemove, 2, AtLeft);

        m_view->setViewMode(QListView::IconMode);
        m_view->setGridSize(PictureManager::THUMBNAIL_SIZE + QSize(20,20));
        m_view->setModel(PictureController::sortProxy(false, this));

        setCentralWidget(m_view);
    }

    int FormPictureManager::currentPictureId() const
    {
        return (m_selectedNone || m_view->currentIndex().row() < 0)
                ? Constants::NO_ID
                : m_view->model()->data(m_view->currentIndex(),
                                        Qt::UserRole).toInt();
    }

    QSize FormPictureManager::sizeHint() const
    {
        return QSize(600, 600);
    }

    void FormPictureManager::accept()
    {
        if (m_inSelect && currentPictureId() == Constants::NO_ID)
        {
            QMessageBox::information(this,
                                     tr("Select Picture"),
                                     tr("Please select a picture by clicking on it, adding one if necessary."));
        }
        else
        {
            done(QDialog::Accepted);
        }
    }

    void FormPictureManager::selectNone()
    {
        m_selectedNone = true;
        done(QDialog::Accepted);
    }

    void FormPictureManager::addPicture()
    {
//        FormAddEditPicture form(Constants::NO_ID, this);

//        if (form.exec() == QDialog::Accepted)
//        {
//            selectPicture(form.idAddedPicture());
//        }

        QString filter = tr("Images")     + " (*.png *.xpm *.jpg *.jpeg *.gif *.bmp *.pbm  *.pgm *.ppm *.xbm);;"
                       + tr("All files") + " (*.*)";

        QString name = QFileDialog::getOpenFileName(this,
                                                    tr("Select Picture"),
                                                    LAST_FOLDER.isEmpty() ? QDir::homePath() : LAST_FOLDER,
                                                    filter);

        if (!name.isEmpty())
        {
            QFileInfo info(name);
            LAST_FOLDER = info.canonicalPath();

            //Test if valid QPixmap
            QPixmap p(name);

            if (p.isNull())
            {
                QMessageBox::information(this,
                                         tr("Select Picture"),
                                         tr("The selected picture is invalid."));
            }
            else
            {
                selectPicture(PictureManager::instance()->add(info.fileName(), p));
            }
        }
    }

    void FormPictureManager::removePicture()
    {
        if (currentPictureId() == Constants::NO_ID)
            return;

        if (QMessageBox::question(this,
                                  tr("Remove Picture"),
                                  tr("Remove the selected picture?"),
                                  QMessageBox::Yes | QMessageBox::No,
                                  QMessageBox::No) == QMessageBox::Yes)
        {
            PictureManager::instance()->remove(currentPictureId());
        }
    }

    void FormPictureManager::editPicture()
    {
        if (currentPictureId() == Constants::NO_ID)
            return;

        FormAddEditPicture form(currentPictureId(), this);
        form.exec();
    }

    //FormAddedPicture

    FormAddEditPicture::FormAddEditPicture(int _id, QWidget* _parent) :
        CAMSEGDialog(DialogWithPicture, OkCancelButtons, _parent),
        m_id(_id)
    {
        m_txtName = new QLineEdit(this);
        m_lblPicture = new QLabel(this);

        QFormLayout* layout = new QFormLayout(centralWidget());
        layout->addRow(tr("&Name:"), m_txtName);
        layout->addRow(tr("Picture:"), m_lblPicture);

        m_lblPicture->setFixedSize(128, 128);

        setPicture(Core::pixmap("picture"));

        QPushButton* btnSelectPicture = new QPushButton(Core::icon("open-picture"), tr("&Browse"), this);
        addButton(btnSelectPicture, 0, AtLeft);
        connect(btnSelectPicture, &QPushButton::clicked, this, &FormAddEditPicture::selectPicture);

        //Load
        if (_id != Constants::NO_ID)
        {
            setBothTitles(tr("Edit Picture"));
            m_txtName->setText(PictureManager::instance()->get(_id)->name);
            m_currentPicture = PictureManager::instance()->get(_id)->picture;
            updatePicture();
        }
        else
        {
            setBothTitles(tr("Add Picture"));
        }

        m_txtName->setFocus();
    }

    void FormAddEditPicture::updatePicture()
    {
        m_lblPicture->setPixmap(m_currentPicture.scaled(QSize(128, 128),
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation));
    }

    int FormAddEditPicture::idAddedPicture() const
    {
        return m_id;
    }

    void FormAddEditPicture::accept()
    {
//        if (m_txtName->text().isEmpty())
//        {
//            QMessageBox::information(this,
//                                     windowTitle(),
//                                     tr("The picture name cannot be empty."));
//            m_txtName->setFocus();
//            return;
//        }
        if (m_currentPicture.isNull())
        {
            QMessageBox::information(this,
                                     windowTitle(),
                                     tr("The picture cannot be empty."));
            return;
        }

        if (m_id == Constants::NO_ID) //Add mode
        {
            m_id = PictureManager::instance()->add(m_txtName->text(),
                                                   m_currentPicture);
        }
        else
        {
            PictureManager::instance()->setName   (m_id, m_txtName->text());
            PictureManager::instance()->setPicture(m_id, m_currentPicture);
        }

        done(QDialog::Accepted);
    }

    void FormAddEditPicture::selectPicture()
    {
        QString filter = tr("Images")     + " (*.png *.xpm *.jpg *.jpeg *.gif *.bmp *.pbm  *.pgm *.ppm *.xbm);;"
                       + tr("All files") + " (*.*)";

        QString name = QFileDialog::getOpenFileName(this,
                                                    tr("Select Picture"),
                                                    LAST_FOLDER.isEmpty() ? QDir::homePath() : LAST_FOLDER,
                                                    filter);

        if (!name.isEmpty())
        {
            QFileInfo info(name);
            LAST_FOLDER = info.canonicalPath();

            //Test if valid QPixmap
            QPixmap p(name);

            if (p.isNull())
            {
                QMessageBox::information(this,
                                         tr("Select Picture"),
                                         tr("The selected picture is invalid."));
            }
            else
            {
                m_currentPicture = p;
                updatePicture();
            }
        }
    }

}

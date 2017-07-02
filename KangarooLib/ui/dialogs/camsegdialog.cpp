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

/**
  @file  camsegdialog.cpp
  This file contains the method definitions for the CAMSEGDialog class

  @author   Lucas Rioux Maldague
  @date     June 17th 2009
  @version  3.0

*/

#include "camsegdialog.h"
#include "../core.h"

#include <QPushButton>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

namespace KLib {

    CAMSEGDialog::CAMSEGDialog(const DialogType p_dialogType, const ButtonType p_buttonType, QWidget* p_parent) :
            QDialog(p_parent),
            m_centralWidget(new QWidget()),
            m_dialogType(p_dialogType),
            m_buttonType(p_buttonType),
            m_nbButtonsLeft(1)
    {
        loadMainUI();
    }

    CAMSEGDialog::CAMSEGDialog(QWidget* p_centralWidget, const DialogType p_dialogType, const ButtonType p_buttonType, QWidget* p_parent) :
            QDialog(p_parent),
            m_centralWidget(p_centralWidget),
            m_dialogType(p_dialogType),
            m_buttonType(p_buttonType),
            m_nbButtonsLeft(1)
    {
        loadMainUI();
    }

    void CAMSEGDialog::setPicture(const QPixmap& p_picture)
    {
        if (m_lblPicture) m_lblPicture->setPixmap(p_picture);
    }

    void CAMSEGDialog::setBothTitles(const QString& p_title)
    {
        if (m_lblTitle) m_lblTitle->setText(p_title);
        setWindowTitle(p_title);
    }

    void CAMSEGDialog::setTitle(const QString& p_title)
    {
        if (m_lblTitle) m_lblTitle->setText(p_title);
    }

    void CAMSEGDialog::setCentralWidget(QWidget* _centralWidget)
    {
        m_mainLayout->removeWidget(m_centralWidget);

        if (m_topLayout)
        {
            m_mainLayout->insertWidget(1, _centralWidget);
        }
        else
        {
            m_mainLayout->insertWidget(0, _centralWidget);
        }

        m_centralWidget->deleteLater();
        m_centralWidget = _centralWidget;
        _centralWidget->raise();
    }

    void CAMSEGDialog::setBottomLineVisible(const bool p_show)
    {
        m_bottomLine->setVisible(p_show);
    }

    bool CAMSEGDialog::isBottomLineVisible() const
    {
        return m_bottomLine->isVisible();
    }

    void CAMSEGDialog::lockSize()
    {
        m_mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    }

    void CAMSEGDialog::setPictureLabel(QLabel* _label)
    {
        if (m_topLayout)
        {
            if (m_lblPicture)
            {
                m_topLayout->removeWidget(m_lblPicture);
                m_lblPicture->deleteLater();
            }

            m_lblPicture = _label;

            if (m_lblPicture)
            {
                m_topLayout->insertWidget(0, m_lblPicture);
            }
        }
    }

    void CAMSEGDialog::addButton(QWidget* p_button, const quint8 p_index, const ButtonPosition p_position)
    {
        if (p_position == AtLeft)
        {
            m_buttonLayout->insertWidget(p_index, p_button);
            ++m_nbButtonsLeft;
        }
        else
        {
            m_buttonLayout->insertWidget(m_nbButtonsLeft + p_index, p_button);
        }

        if (m_btnClose) m_btnClose->raise();
        if (m_btnCancel) m_btnCancel->raise();
        if (m_btnOK) m_btnOK->raise();
    }

    const QPixmap* CAMSEGDialog::picture() const
    {
        return m_lblPicture ? m_lblPicture->pixmap() : nullptr;
    }

    QString CAMSEGDialog::title() const
    {
        return m_lblTitle ? m_lblTitle->text() : "";
    }

    QWidget* CAMSEGDialog::centralWidget() const
    {
        return m_centralWidget;
    }

    void CAMSEGDialog::loadMainUI()
    {
        //---------------------Widget creation----------------------

        //Create layouts
        m_mainLayout    = new QVBoxLayout(this);
        m_topLayout     = m_dialogType != DialogWithoutPictureAndTitle  ? new QHBoxLayout()           : nullptr;
        m_buttonLayout  = new QHBoxLayout();

        //Create top widgets
        m_lblPicture    = m_dialogType == DialogWithPicture             ? new QLabel(this)                  : nullptr;
        m_lblTitle      = m_dialogType != DialogWithoutPictureAndTitle  ? new QLabel("Untitled Dialog", this) : nullptr;

        //Create buttons
        m_btnOK =       m_buttonType == OkCancelButtons ? new QPushButton(Core::icon("ok"), tr("&OK"), this)        : nullptr;
        m_btnCancel =   m_buttonType == OkCancelButtons ? new QPushButton(Core::icon("close"), tr("&Cancel"), this) : nullptr;
        m_btnClose =    m_buttonType == CloseButton     ? new QPushButton(Core::icon("close"), tr("&Close"), this)  : nullptr;

        //Bottom line
        m_bottomLine = new QFrame(this);
        m_bottomLine->setFrameShape(QFrame::HLine);

        //Window title
        setWindowTitle("Untitled Dialog");

        //---------------------Widget properties----------------------
        if (m_lblTitle)
        {
            QFont titleFont;
            titleFont.setBold(true);
            titleFont.setPointSize(TITLE_FONT_SIZE);
            m_lblTitle->setFont(titleFont);
            m_lblTitle->setMaximumHeight(PICTURE_SIZE);
        }

        if (m_dialogType == DialogWithPicture)
        {
            m_lblPicture->setMaximumSize(PICTURE_SIZE, PICTURE_SIZE);
            m_topLayout->addWidget(m_lblPicture);
        }

        //---------------------Add widgets to layouts----------------------

        if (m_dialogType != DialogWithoutPictureAndTitle)
            m_topLayout->addWidget(m_lblTitle);

        m_spacerItem = new QSpacerItem(10, 2, QSizePolicy::Expanding);
        m_buttonLayout->addSpacerItem(m_spacerItem);

        if (m_buttonType == OkCancelButtons)
        {
            m_buttonLayout->addWidget(m_btnOK);
            m_buttonLayout->addWidget(m_btnCancel);
            m_btnOK->setDefault(true);
        }
        else if (m_buttonType == CloseButton)
        {
            m_buttonLayout->addWidget(m_btnClose);
            m_btnClose->setDefault(true);
            m_btnClose->setEnabled(true);
        }

        if (m_dialogType != DialogWithoutPictureAndTitle)
            m_mainLayout->addLayout(m_topLayout);

        m_mainLayout->addWidget(m_centralWidget);
        m_mainLayout->addWidget(m_bottomLine);
        m_mainLayout->addLayout(m_buttonLayout);

        //Connexions

        if (m_btnOK)
        {
            connect(m_btnOK, SIGNAL(clicked()), this, SLOT(accept()));
        }

        if (m_btnCancel)
        {
            connect(m_btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
        }

        if (m_btnClose)
        {
            connect(m_btnClose, SIGNAL(clicked()), this, SLOT(reject()));
        }
    }

}

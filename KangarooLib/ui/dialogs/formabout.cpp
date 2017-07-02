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
  @file  frmabout.cpp
  The "frmAbout" UI class shows information about CAMSEG (licence, version, ...)

  @author   Lucas Rioux Maldague
  @date     January 8th 2009
  @version  3.0

*/

#include "formabout.h"
#include "../core.h"

#include <QDate>
#include <QDesktopServices>
#include <QLabel>
#include <QTabWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QUrl>
#include <QFile>
#include <QTextStream>

namespace KLib {

    frmAbout::frmAbout(QWidget* _parent) : QDialog(_parent)
    {
        loadUI();
    }

    void frmAbout::loadUI()
    {
        //Widget Creation
        m_lblAppName = new QLabel(this);
        m_lblIcon  = new QLabel(this);
        m_lblInfos  = new QLabel(this);
        onglets   = new QTabWidget(this);
        m_btnClose = new QPushButton(Core::icon("close"), tr("&Close"), this);

        m_lblPicture       = new QLabel(this);
        m_lblGeneral = new QLabel(this);
        m_lblAuthors    = new QLabel(this);
        m_txtLicence   = new QTextEdit(this);

        //Widgets Properties
        setWindowTitle(tr("About %1 ...").arg(Core::APP_NAME));

        QFont policeTitre;
        policeTitre.setBold(true);
        policeTitre.setPointSize(14);

        m_lblAppName->setFont(policeTitre);

        m_lblIcon->setFixedSize(80, 80);
        m_lblIcon->setPixmap(Core::pixmap("kangaroo80"));
        m_txtLicence->setReadOnly(true);

        m_lblGeneral->setWordWrap(true);
        m_lblPicture->setPixmap(Core::pixmap("promo"));

        m_lblAuthors->setAlignment(Qt::AlignTop);
        m_lblGeneral->setAlignment(Qt::AlignTop);

        //-------------------------Informations----------------------

        //Top Screen
        m_lblAppName->setText(tr("Kangaroo Personal Finance Manager"));
        m_lblInfos->setText(tr("Version %1 - <i>%2</i>").arg(Core::APP_VERSION).arg(Core::APP_CODENAME) +
                              "<br />" + tr("Using Qt %1").arg(QT_VERSION_STR));

        //Informations Page
        QString description = tr("%1 is an application ...").arg(Core::APP_NAME);

        QString contact = tr("Please contact <a href=\"mailto:scm@camsegtech.com\">scm@camsegtech.com</a> for bugs and requests.");

        m_lblGeneral->setText(tr("Description : %1"
                                   "<br /><br />%2").arg(description).arg(contact) +
                                   "<br /><br />" + Core::APP_COPYRIGHT + "<br />" +
                                   tr("Build on %1").arg(__DATE__) + "<br />" +
                                   tr("Website : %1").arg("<a href=\"http://sourceforge.net/projects/kangaroopfm/\">"
                                                          "http://sourceforge.net/projects/kangaroopfm/</a>") );

        //Authors
        QStringList authors;

        authors << QString("<b>Lucas Rioux-Maldague</b>") + "<br />&nbsp;&nbsp;&nbsp;" +
                        tr("Project Director") + "<br />&nbsp;&nbsp;&nbsp;" +
                        "<a href=\"mailto:lucasr.mal@gmail.com\">lucasr.mal@gmail.com</a>" + "<br /><br />"
                << QString("<b>Caleb DeWitt</b>") + "<br />&nbsp;&nbsp;&nbsp;" +
                   tr("Graphic Designer") + "<br />&nbsp;&nbsp;&nbsp;" +
                   "<a href=\"mailto:calebdewitt@gmail.com\">calebdewitt@gmail.com</a>" + "<br /><br />";

        authors << tr("<b>Artwork Credits</b>") + "<br />&nbsp;&nbsp;&nbsp;"
                "<a href=\"http://www.oxygen-icons.org\">The Oxygen Project</a><br />&nbsp;&nbsp;&nbsp;"
                "<a href=\"http://www.flickr.com/photos/paulmannix\">paulmannix</a>"
                   "and <a href=\"http://animalphotos.info/a/\">Animal Photos!</a>" + "<br /><br />";

        m_lblAuthors->setText("");

        m_lblGeneral->setContentsMargins(5,5,5,5);
        m_lblAuthors->setContentsMargins(5,5,5,5);

        foreach (QString string, authors)
            m_lblAuthors->setText(m_lblAuthors->text() + string);

        //Layout Creation
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        QHBoxLayout* topLayout = new QHBoxLayout();
        QHBoxLayout* buttonLayout = new QHBoxLayout();

        QVBoxLayout* topTextLayout = new QVBoxLayout();

        onglets->addTab(m_lblPicture, tr("&Main"));
        onglets->addTab(m_lblGeneral, tr("&About"));
        onglets->addTab(m_lblAuthors, tr("A&uthors"));
        onglets->addTab(m_txtLicence, tr("&License"));

        topTextLayout->addWidget(m_lblAppName);
        topTextLayout->addWidget(m_lblInfos);

        topLayout->addWidget(m_lblIcon);
        topLayout->addLayout(topTextLayout);

        buttonLayout->addWidget(m_btnClose);
        buttonLayout->setAlignment(Qt::AlignRight);

        mainLayout->addLayout(topLayout);
        mainLayout->addSpacing(5);
        mainLayout->addWidget(onglets);
        mainLayout->addSpacing(5);
        mainLayout->addLayout(buttonLayout);

        mainLayout->setSizeConstraint(QLayout::SetFixedSize);

        //Connexions
        connect(m_btnClose, SIGNAL(clicked()), this, SLOT(close()));
        connect(m_lblGeneral, SIGNAL(linkActivated(QString)), this, SLOT(launchBrowser(QString)));
        connect(m_lblAuthors, SIGNAL(linkActivated(QString)), this, SLOT(launchBrowser(QString)));

        //Show the license
        showLicense();
    }

    void frmAbout::launchBrowser(const QString& _address)
    {
        QDesktopServices::openUrl(QUrl(_address));
    }

    void frmAbout::showLicense()
    {
        QFile file(Core::path(Path_License));

        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QTextStream stream(&file);
            m_txtLicence->setText(stream.readAll());
            file.close();
        }
    }

}

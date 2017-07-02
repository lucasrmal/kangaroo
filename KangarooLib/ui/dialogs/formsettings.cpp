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

#include "formsettings.h"

#include "../core.h"
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QListWidget>
#include <QTabWidget>
#include <QMessageBox>

namespace KLib
{

    FormSettings::FormSettings(QWidget* _parent) :
        CAMSEGDialog(DialogWithPicture, OkCancelButtons, _parent),
        m_categories(SettingsManager::instance()->categories())
    {
        setBothTitles(tr("%1 Settings").arg(Core::APP_NAME));
        setPicture(Core::pixmap("configure"));

        m_listCategories = new QListWidget(this);
        m_stack = new QStackedWidget(this);

        m_listCategories->setFixedWidth(175);
        m_listCategories->setIconSize(QSize(24,24));

        //Load the UI
        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget());
        mainLayout->addWidget(m_listCategories);
        mainLayout->addWidget(m_stack);

        //Load the categories and tabs
        for (const SettingsManager::SettingsCategory* c : m_categories)
        {
            m_listCategories->addItem(new QListWidgetItem(Core::icon(c->icon), c->title, m_listCategories));

            QTabWidget* tabs = new QTabWidget(this);

            for (const SettingsManager::SettingsPage& p : c->pages)
            {
                ISettingsPage* page = p.builder(this);
                page->load();

                m_locations[page] = QPair<int, int>(m_listCategories->count()-1, tabs->count());
                tabs->addTab(page, p.title);
            }

            m_stack->addWidget(tabs);
        }

        if (m_listCategories->count())
        {
            m_listCategories->setCurrentRow(0);
        }

        connect(m_listCategories, &QListWidget::currentRowChanged, m_stack, &QStackedWidget::setCurrentIndex);
    }

    void FormSettings::accept()
    {
        //Validate everything first
        for (auto i = m_locations.begin(); i != m_locations.end(); ++i)
        {
            QStringList errors = i.key()->validate();

            if (!errors.isEmpty())
            {
                QString strErrors;
                for (QString s : errors)
                {
                    strErrors += "\t" + s + "\n";
                }
                //Show the correct page
                m_listCategories->setCurrentRow(i.value().first);
                m_tabWidgets[i.value().first]->setCurrentIndex(i.value().second);

                QMessageBox::information(this,
                                         tr("Save Changes"),
                                         tr("Please correct the following errors:\n %1").arg(strErrors),
                                         QMessageBox::Ok, QMessageBox::Ok);
                return;
            }
        }

        //Now all is good, save everything
        for (auto i = m_locations.begin(); i != m_locations.end(); ++i)
        {
            i.key()->save();
        }

        done(QDialog::Accepted);
    }

}


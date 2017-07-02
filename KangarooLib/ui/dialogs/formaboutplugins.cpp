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

#include "formaboutplugins.h"

#include "../core.h"
#include "../../iplugin.h"

#include <QTreeWidget>
#include <QPushButton>
#include <QMessageBox>

namespace KLib {

    FormAboutPlugins::FormAboutPlugins(QWidget* parent) :
            CAMSEGDialog(DialogWithPicture, CloseButton, parent)
    {
        setBothTitles(tr("About Plugins"));
        setPicture(Core::pixmap("plugin"));
        setMinimumSize(400, 400);

        tablePlugins = new QTreeWidget(this);
        btnDetails = new QPushButton(Core::icon("about"), tr("&Details"), this);

        fillTable();

        setCentralWidget(tablePlugins);
        addButton(btnDetails);

        connect(btnDetails, SIGNAL(clicked()), this, SLOT(aboutPlugin()));
        connect(tablePlugins, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(aboutPlugin()));
    }

    void FormAboutPlugins::aboutPlugin()
    {
        if (tablePlugins->currentIndex().isValid())
        {
            QString msgBoxContent;
            IPlugin* plugin = m_plugins.at(tablePlugins->currentIndex().row());

            QStringList required = plugin->requiredPlugins();
            QString list;

            for (QString s : required)
            {
                list += QString("<br />&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;") + s;
            }

            msgBoxContent = "<b>" + plugin->name() + "</b><br /><br />" +
                            tr("Version: %1").arg(plugin->version()) + "<br />" +
                            tr("Author: %1").arg(plugin->author()) + "<br />" +
                            tr("Website: %1").arg("<a href=\"" + plugin->url() + "\">"+ plugin->url() + "</a>") + "<br />" +
                            tr("Copyright: %1").arg(plugin->copyright()) + "<br />" +
                            tr("Description: %1").arg(plugin->description()) + "<br />" +
                            tr("Required Plugins: %1").arg(list);

            QMessageBox::information(this,
                                    tr("About Plugin"),
                                    msgBoxContent);
        }
    }

    void FormAboutPlugins::fillTable()
    {
        QTreeWidgetItem* currentItem = NULL;

        tablePlugins->setColumnCount(3);

        tablePlugins->setColumnWidth(0, 150);
        tablePlugins->setColumnWidth(1, 50);
        tablePlugins->setColumnWidth(2, 60);

        QStringList columnNames;
        columnNames << tr("Name")
                    << tr("Version")
                    << tr("Author");
        tablePlugins->setHeaderLabels(columnNames);

        for (IPlugin* plugin : Core::instance()->plugins())
        {
            m_plugins << plugin;
            currentItem = new QTreeWidgetItem();

            currentItem->setText(0, plugin->name());
            currentItem->setText(1, plugin->version());
            currentItem->setText(2, plugin->author());

            tablePlugins->addTopLevelItem(currentItem);
        }

        tablePlugins->setSortingEnabled(true);
        tablePlugins->sortByColumn(0, Qt::AscendingOrder);
    }

}

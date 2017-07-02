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

#include "keyshortcutssettings.h"
#include "../../widgets/shortcuteditor.h"
#include "../../actionmanager/actionmanager.h" 
#include "../../actionmanager/actioncontainer.h"
#include "../../actionmanager/command.h"
#include "../../core.h"

#include <QTreeWidget>
#include <QVBoxLayout>
#include <QAction>
#include <QMenu>

namespace KLib {

    //Widget
    KeyShortcutsSettings::KeyShortcutsSettings(QWidget* parent) : ISettingsPage(parent)
    {
        //Create widgets
        treeShortcuts  = new QTreeWidget();
        shortcutEditor = new ShortcutEditor();

        //Set properties
        QStringList columns;
        columns << tr("ID") << tr("Label") << tr("Shortcut");

        treeShortcuts->setColumnCount(3);
        treeShortcuts->setHeaderLabels(columns);

        treeShortcuts->setColumnWidth(0, 180);
        treeShortcuts->setColumnWidth(1, 140);
        treeShortcuts->setColumnWidth(2, 70);



        //Set layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addWidget(treeShortcuts, 3);
        mainLayout->addWidget(shortcutEditor);

        //Connexions
        connect(treeShortcuts, &QTreeWidget::currentItemChanged, this, &KeyShortcutsSettings::selectionChanged);
        connect(shortcutEditor, &ShortcutEditor::shortcutChanged, this, &KeyShortcutsSettings::shortcutChanged);
    }

    QStringList KeyShortcutsSettings::validate()
    {
        return {};
    }

    void KeyShortcutsSettings::save()
    {
        ActionManager* am = ActionManager::instance();

        foreach (QString id, m_newShortcuts.keys())
        {
            am->setShortcut(id, m_newShortcuts[id]);
        }
    }

    void KeyShortcutsSettings::load()
    {
        //Add actions to treeWidget
        ActionManager* am = ActionManager::instance();
        Command* command = nullptr;
        QTreeWidgetItem* treeItem = nullptr;
        QTreeWidgetItem* topLevelItem = nullptr;
        QString cat;

        //Find top level categories
        const QString OTHER_ACTIONS = tr("Other Actions");
        QHash<QString, QTreeWidgetItem*> topLevelCats;
        topLevelCats[OTHER_ACTIONS] = nullptr;

        QFont boldFont;
        boldFont.setBold(true);

        ActionContainer* ac = nullptr;

        foreach (QString id, am->commandNames())
        {
            if (id.indexOf('.') != -1)
            {
                cat = id.section('.', 0, 0);

                if ( ! topLevelCats.contains(cat))
                {
                    //Find the action container
                    QString trCat = cat;

                    ac = am->actionContainer(cat);

                    if (ac && ac->menu())
                        trCat = ac->menu()->title();

                    topLevelItem = new QTreeWidgetItem(treeShortcuts);
                    topLevelItem->setText(0, trCat);
                    topLevelItem->setFont(0, boldFont);

                    topLevelCats[cat] = topLevelItem;
                }
            }
            else
            {
                cat = OTHER_ACTIONS;

                if ( !topLevelCats[OTHER_ACTIONS])
                    topLevelCats[OTHER_ACTIONS] = new QTreeWidgetItem(treeShortcuts);
            }

            //Create the command
            treeItem = new QTreeWidgetItem(topLevelCats[cat]);
            command = am->command(id);

            treeItem->setIcon(0, command->action()->icon());
            treeItem->setData(0, Qt::UserRole, id);

            treeItem->setText(0, id.section('.', 1));
            treeItem->setText(1, command->action()->text());
            treeItem->setText(2, command->shortcut().toString());

            m_treeWidgetsItems[id] = treeItem;
        }

        treeShortcuts->expandAll();
        treeShortcuts->sortByColumn(0, Qt::AscendingOrder);
    }

    void KeyShortcutsSettings::selectionChanged(QTreeWidgetItem* p_current, QTreeWidgetItem*)
    {
        QString id = p_current->data(0, Qt::UserRole).toString();

        if ( ! id.isEmpty())
        {
            Command* command = ActionManager::instance()->command(id);

            if (command)
            {
                shortcutEditor->setCommand(command);
            }
        }
    }

    void KeyShortcutsSettings::shortcutChanged(const Command* p_command, const QKeySequence& p_newShortcut)
    {
        //Set the new shortcut in the QHash
        m_newShortcuts[p_command->id()] = p_newShortcut;
        m_treeWidgetsItems[p_command->id()]->setText(2, p_newShortcut.toString(QKeySequence::NativeText));
    }

} //Namespace

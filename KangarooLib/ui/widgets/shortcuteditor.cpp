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

#include "shortcuteditor.h"
#include "../actionmanager/command.h"
#include "../core.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QKeyEvent>

namespace KLib {

    ShortcutEditor::ShortcutEditor(QWidget* parent) :
        QGroupBox(parent), m_command(NULL)
    {
        setCommand(NULL);
        loadUI();
    }

    void ShortcutEditor::setCommand(Command* p_command)
    {
        if (p_command)
        {
            setTitle(tr("Shortcut for %1").arg(p_command->id()));
            setEnabled(true);
            lblCurrent->setText(p_command->shortcut().toString(QKeySequence::NativeText));
            txtShortcut->setFocus();
        }
        else
        {
            setTitle(tr("No action selected."));
            setEnabled(false);
        }

        m_command = p_command;
    }


    void ShortcutEditor::clear()
    {
        txtShortcut->clear();
        txtShortcut->setFocus();
    }

    void ShortcutEditor::apply()
    {
        if (shortcutInLineEdit() != shortcut())
        {
            m_shortcut = shortcutInLineEdit();
            emit shortcutChanged(m_command, m_shortcut);
        }
    }

    void ShortcutEditor::revert()
    {
        setShortcutInLineEdit(m_command->shortcut());
        txtShortcut->setFocus();
    }

    void ShortcutEditor::setDefault()
    {
        setShortcutInLineEdit(m_command->defaultShortcut());
        txtShortcut->setFocus();
    }

    void ShortcutEditor::loadUI()
    {
        //Create widgets
        lblCurrent  = new QLabel(this);
        txtShortcut = new QLineEdit(this);
        btnClear    = new QPushButton(Core::icon("clear"), tr("C&lear"), this);
        btnApply    = new QPushButton(Core::icon("apply"), tr("&Apply"), this);
        btnRevert   = new QPushButton(Core::icon("undo"), tr("&Revert"), this);
        btnDefault  = new QPushButton(Core::icon("eraser"), tr("&Default"), this);
        
        //Set properties
        txtShortcut->installEventFilter(this);
        
        //Layout
        QFormLayout* mainLayout = new QFormLayout(this);
        QHBoxLayout* shortcutLayout = new QHBoxLayout();
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        
        shortcutLayout->addWidget(txtShortcut);
        shortcutLayout->addWidget(btnClear);
        
        buttonLayout->addWidget(btnApply);
        buttonLayout->addWidget(btnRevert);
        buttonLayout->addWidget(btnDefault);
        
        mainLayout->addRow(tr("Current"), lblCurrent);
        mainLayout->addRow(tr("New"), shortcutLayout);
        mainLayout->addRow(buttonLayout);
        
        //Connexions
        connect(btnClear, SIGNAL(clicked()), this, SLOT(clear()));
        connect(btnApply, SIGNAL(clicked()), this, SLOT(apply()));
        connect(btnRevert, SIGNAL(clicked()), this, SLOT(revert()));
        connect(btnDefault, SIGNAL(clicked()), this, SLOT(setDefault()));
    }

    bool ShortcutEditor::eventFilter(QObject* p_object, QEvent* p_event)
    {
        if (p_object == txtShortcut && p_event->type() == QEvent::KeyPress)
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(p_event);

            if (keyEvent->key() == Qt::Key_unknown)
                return true;

            QString newShortcut;
            int total = 0;

            //Modifiers
            if ( (Qt::ShiftModifier & keyEvent->modifiers()) == Qt::ShiftModifier)
            {
                total += Qt::SHIFT;
            }
            if ( (Qt::ControlModifier & keyEvent->modifiers()) == Qt::ControlModifier)
            {
                total += Qt::CTRL;
            }
            if ( (Qt::AltModifier & keyEvent->modifiers()) == Qt::AltModifier)
            {
                total += Qt::ALT;
            }
            if ( (Qt::MetaModifier & keyEvent->modifiers()) == Qt::MetaModifier)
            {
                total += Qt::META;
            }

            total += keyEvent->key();

            newShortcut = QKeySequence(total).toString(QKeySequence::NativeText);

            if ( !txtShortcut->text().isEmpty())
                newShortcut.prepend(", ");

            txtShortcut->setText(txtShortcut->text() + newShortcut);
            return true;
        }
        else
        {
            return QObject::eventFilter(p_object, p_event);
        }
    }
    
    QKeySequence ShortcutEditor::shortcutInLineEdit()
    {
        return QKeySequence::fromString(txtShortcut->text(), QKeySequence::NativeText);
    }
    
    void ShortcutEditor::setShortcutInLineEdit(const QKeySequence& p_shortcut)
    {
        txtShortcut->setText(p_shortcut.toString(QKeySequence::NativeText));
        txtShortcut->setFocus();
    }
    
} //Namespace

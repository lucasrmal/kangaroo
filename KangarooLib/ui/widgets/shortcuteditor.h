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

#ifndef SHORTCUTEDITOR_H
#define SHORTCUTEDITOR_H

#include <QGroupBox>
#include <QKeySequence>

class QLabel;
class QLineEdit;
class QPushButton;

namespace KLib {
    
    class Command;
    
    /**
      The ShortcutEditor widget allows editing of the keyboard shortcut for a Command.
      When a shortcut is modified, the command is <b>not</b> modified. Instead, the 
      shortcutChanged() signal is emmited and the new shortcut can be retrived using the 
      shortcut() method.
      
      @brief Shortcut editor widget
    */
    class ShortcutEditor : public QGroupBox
    {
        Q_OBJECT
        
        public:
            /**
              Class constructor

              @param[in] parent The widget parent
            */
            explicit        ShortcutEditor(QWidget* parent = NULL);
            
            /**
              @return The current shortcur
            */
            QKeySequence    shortcut() { return m_shortcut; }
            
            /**
              @return The command for which the shortcut is edited
            */
            Command*        command() { return m_command; }

            /**
              Sets the command for which the shortcut is edited
            */
            void            setCommand(Command* p_command);
            
        public slots:
        
            /**
              Clears the shortcut in the lineedit.This does not affect the current shortcut before
              apply() is called.
            */
            void            clear();
            
            /**
              The shortcut currently in the lineedit becomes the current shortcut and
              the shortcutChanged() signal is emited if it's different.
            */
            void            apply();
            
            /**
               The shortcut in the line edit is changed for the original one. This does not affect the current shortcut before
              apply() is called.
            */
            void            revert();
            
            /**
              The shortcut in the line edit is changed for the default one. This does not affect the current shortcut before
              apply() is called.
            */
            void            setDefault();
        
        signals:
            /**
              This signal is emited whether the current shortcut is changed.
            */
            void            shortcutChanged(const Command* p_command, const QKeySequence& p_newShortcut);

        protected:
            bool            eventFilter(QObject* p_object, QEvent* p_event);
        
        private:
            void            loadUI();
            QKeySequence    shortcutInLineEdit();
            void            setShortcutInLineEdit(const QKeySequence& p_shortcut);
        
            QLabel*         lblCurrent;
            QLineEdit*      txtShortcut;
            QPushButton*    btnClear;
            QPushButton*    btnApply;
            QPushButton*    btnRevert;
            QPushButton*    btnDefault;
            
            Command*        m_command;
            QKeySequence    m_shortcut;
    };
}

#endif //SHORTCUTEDITOR_H

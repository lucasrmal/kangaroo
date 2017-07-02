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
  @file  camsegdialog.h
  This file contains the method declarations for the CAMSEGDialog class

  @author   Lucas Rioux Maldague
  @date     June 17th 2009
  @version  3.0

*/

#ifndef CAMSEGDIALOG_H
#define CAMSEGDIALOG_H

#include <QDialog>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QFrame;
class QSpacerItem;

namespace KLib {

    /**
      The CAMSEGDialog class is a sub-class of QDialog. It provides additionnal functionnality such as a
      picture, a title and a button bar. It's the base class of all dialogs in CAMSEG SCM.
      
      To create a custom dialog, simply inherit CAMSEGDialog, initilize it with the constructor, and set the central
      widget (the main part of the dialog) with setCentralWidget.
      
      If you choose an OkCancelButtons dialog, the accept() slot is called when OK is pressed, and the reject() slot is called when
      Cancel is clicked. If you cloose a CloseOnly dialog, the close() slot is called when Close is clicked.
      
      You can disable the default button (the one pressed by default when Return is pressed) with setHasDefaultButton(). This also
      disable the connexion between Esc key and reject()/close().
      
      @brief Dialog box
    */
    class CAMSEGDialog : public QDialog
    {
        Q_OBJECT

        public:

            /**
              Different types of dialog
            */
            enum DialogType
            {
                DialogWithPicture,              ///< Dialog with a picture and a title aligned top-left
                DialogWithoutPicture,           ///< Dialog with a title aligned top-left
                DialogWithoutPictureAndTitle    ///< Dialog without any title and picture. The central widget is then at the top of the dialog.
            };

            /**
              Different types of button layout
            */
            enum ButtonType
            {
                OkCancelButtons,
                CloseButton,
                NoButtons
            };

            /**
              Button position (when adding buttons with addButton()
              
              @see addButton()
            */
            enum ButtonPosition
            {
                AtLeft,
                AtRight
            };

                            /**
                              Creates a new dialog
                              
                              @param[in] p_dialogType The type of dialog
                              @param[in] p_buttonType The type of buttons to display
                              @param[in] p_parent The dialog's parent
                            */
                            CAMSEGDialog(const DialogType p_dialogType, const ButtonType p_buttonType, QWidget* p_parent = nullptr);
                            
                            /**
                              Creates a new dialog with a central widget
                              
                              @param[in] p_dialogType The type of dialog
                              @param[in] p_buttonType The type of buttons to display
                              @param[in] p_centralWidget The central widget
                              @param[in] p_parent The dialog's parent
                            */
                            CAMSEGDialog(QWidget* p_centralWidget, const DialogType p_dialogType, const ButtonType p_buttonType, QWidget* p_parent = nullptr);

            virtual         ~CAMSEGDialog() {}

            /**
              Sets the picture to p_picture (for DialogWithPicture dialogs only).

              @see picture()
            */
            void            setPicture(const QPixmap& p_picture);
            
            /**
              @return The current picture, or NULL if the dialog is not a DialogWithPicture

              @see setPicture()
            */
            const QPixmap*  picture() const;

            /**
              Sets the title and the window title to p_title

              @see setTitle()
            */
            void            setBothTitles(const QString& p_title);
            
            /**
              Sets the title to p_title

              @see title()
            */
            void            setTitle(const QString& p_title);
            
            /**
              @return The dialog title

              @see setTitle()
            */
            QString         title() const;

            /**
              Sets the central widget to p_centralWidget

              @see centralWidget()
            */
            void            setCentralWidget(QWidget* p_centralWidget);
            
            /**
              @return The central widget

              @see setCentralWidget()
            */
            QWidget*        centralWidget() const;

            /**
              Sets the visible state of the bottom line. The bottom line is an horizontal line located
              between the central widgets and the buttons.

              @see isBottomLineVisible()
            */
            void            setBottomLineVisible(const bool p_show);
            
            /**
              @return If the bottom line is visible (default is True). The bottom line is an horizontal line located
              between the central widgets and the buttons.

              @see setBottomLineVisible()
            */
            bool            isBottomLineVisible() const;

            /**
              Locks the widget to the central widget's reccomended size. After locking the size, the dialog
              cannot be resized by the user anymore.
            */
            void            lockSize();

            /**
              Adds a button (or any widget) to the button bar (can be used even if the button type is NoButtons).
              
              For example, to add a button at the left of the dialog, write :
              @code
              myDialog->addButton(btnDoSomething, 0, AtLeft);
              @endcode
              
              To add a button at the far right of a OKCancel :
              @code
              myDialog->addButton(btnDoSomething, 2);
              @endcode
              
              @param[in] p_putton The button to add.
              @param[in] p_index the button index (starting at 0, left to right).
              @param[in] p_position The position (left or right).
            */
            void            addButton(QWidget* p_button, const quint8 p_index = 0, const ButtonPosition p_position = AtRight);

            void            setPictureLabel(QLabel* _label);

        protected:
            QPushButton*        m_btnOK;
            QPushButton*        m_btnCancel;
            QPushButton*        m_btnClose;

        private:
            void                loadMainUI();

            QWidget*            m_centralWidget;
            QFrame*             m_bottomLine;
            QSpacerItem*        m_spacerItem;
            QLabel*             m_lblPicture;
            QLabel*             m_lblTitle;
            QVBoxLayout*        m_mainLayout;
            QHBoxLayout*        m_topLayout;
            QHBoxLayout*        m_buttonLayout;

            const DialogType    m_dialogType;
            const ButtonType    m_buttonType;
            quint32             m_nbButtonsLeft;
            
            static const quint8 TITLE_FONT_SIZE = 18;
            static const quint8 PICTURE_SIZE = 48;


    };

} //Namespace

#endif // CAMSEGDIALOG_H

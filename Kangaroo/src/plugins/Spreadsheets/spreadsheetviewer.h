#ifndef SPREADSHEETVIEWER_H
#define SPREADSHEETVIEWER_H

#include <QWidget>

class QAction;
class QToolBar;
class Spreadsheet;

class SpreadsheetViewer : public QWidget
{
    Q_OBJECT

    public:
        SpreadsheetViewer(QWidget* _parent = NULL);

    public slots:
        void alignLeft();
        void alignCenter();
        void alignRight();

        void toggleBold();
        void toggleItalic();
        void toggleUnderline();

        void changeFont();
        void changeBackground();
        void changeForeground();

    private:
        Spreadsheet* m_spreadsheet;
        QToolBar* m_toolbar;

        QAction* act_alignLeft;
        QAction* act_alignCenter;
        QAction* act_alignRight;

        QAction* act_toggleBold;
        QAction* act_toggleItalic;
        QAction* act_toggleUnderline;

        QAction* act_changeFont;
        QAction* act_changeBackground;
        QAction* act_changeForeground;
};



#endif // SPREADSHEETVIEWER_H

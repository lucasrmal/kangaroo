#ifndef FORMEDITHOMETABWIDGETS_H
#define FORMEDITHOMETABWIDGETS_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>

class HomeTab;
class QListWidget;

class FormEditHomeTabWidgets : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        FormEditHomeTabWidgets(HomeTab* _parent);

        QSize sizeHint() const { return QSize(650, 400); }

    public slots:
        void accept();

        void select();
        void unselect();
        void moveUp();
        void moveDown();

        void onRowChanged_All(int _row);
        void onRowChanged_Selected(int _row);

    private:
        void loadWidgets();

        HomeTab* m_homeTab;

        QListWidget* m_listAll;
        QListWidget* m_listSelected;

        QPushButton* m_btnSelect;
        QPushButton* m_btnUnselect;
        QPushButton* m_btnMoveUp;
        QPushButton* m_btnMoveDown;
};

#endif // FORMEDITHOMETABWIDGETS_H

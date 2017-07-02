#ifndef FORMEDITFREQUENTACCOUNTS_H
#define FORMEDITFREQUENTACCOUNTS_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>

class QListWidget;

class FormEditFrequentAccounts : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        explicit FormEditFrequentAccounts(QWidget *parent = 0);

        QSize sizeHint() const { return QSize(650, 750); }

    public slots:
        void accept();

        void select();
        void unselect();
        void moveUp();
        void moveDown();

        void onRowChanged_All();
        void onRowChanged_Selected();

    private:
        void loadAccounts();

        QListWidget* m_listAll;
        QListWidget* m_listSelected;

        QPushButton* m_btnSelect;
        QPushButton* m_btnUnselect;
        QPushButton* m_btnMoveUp;
        QPushButton* m_btnMoveDown;

};

#endif // FORMEDITFREQUENTACCOUNTS_H

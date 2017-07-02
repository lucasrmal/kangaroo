#ifndef FORMMANAGEPROGRAMS_H
#define FORMMANAGEPROGRAMS_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>
#include "rewardsprogram.h"

class QTableView;
class QLineEdit;
class QComboBox;
class TierEditor;

class FormAddProgram : public  KLib::CAMSEGDialog
{
    Q_OBJECT
    public:
        explicit FormAddProgram(QWidget* _parent = 0);

        //QSize sizeHint() const { return QSize(400, 400); }

    public slots:
        void accept();

        void addCurrency();

    private:
        QLineEdit* m_txtName;
        QComboBox* m_cboCurrency;
        TierEditor* m_tierEditor;
        QTimer*    m_timer;

        QList<RewardTier> m_tiers;
};

class FormManagePrograms : public KLib::CAMSEGDialog
{
        Q_OBJECT
    public:
        explicit FormManagePrograms(QWidget* _parent = 0);

        QSize sizeHint() const { return QSize(400, 400); }

    private slots:
        void add();
        void remove();
        void editTiers();

    private:
        QTableView* m_tableView;

};

#endif // FORMMANAGEPROGRAMS_H

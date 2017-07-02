#ifndef FORMMERGE_H
#define FORMMERGE_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>
#include <QSet>
#include "managerwidget.h"

class QRadioButton;

class FormMerge : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        explicit FormMerge(const QSet<int>& _mergeSet, ManageType _type, QWidget *parent = nullptr);

    public slots:
        void accept();

    private:
        QList<QRadioButton*> m_options;
        QList<int> m_mergeList; //We need the list to keep the order of the radio buttons
        QSet<int> m_mergeSet;
        ManageType m_type;

};

#endif // FORMMERGE_H

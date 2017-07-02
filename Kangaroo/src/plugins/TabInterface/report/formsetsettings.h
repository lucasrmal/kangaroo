#ifndef FORMSETSETTINGS_H
#define FORMSETSETTINGS_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>
#include "report.h"

class FormSetSettings : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        explicit FormSetSettings(ReportSettings& _settings, QWidget *parent = 0);

    public slots:
        void accept();

    private:
        ReportSettings& m_settings;

        QList<QWidget*> m_widgets;

};

#endif // FORMSETSETTINGS_H

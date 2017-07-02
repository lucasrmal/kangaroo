#ifndef FORMBOOKSETTINGS_H
#define FORMBOOKSETTINGS_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>
#include <QList>

class BookSettingsTabFactorySuper;
class BookSettingsTab;
class QTabWidget;
class QLineEdit;
class QComboBox;

class FormBookSettings : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        explicit FormBookSettings(QWidget* _parent = nullptr);

        static void registerTab(BookSettingsTabFactorySuper* _tab);

        QSize sizeHint() const { return QSize(500,450); }

    private slots:
        void accept();

    private:

        //Tabs
        QTabWidget* m_tabs;
        QList<BookSettingsTab*> m_extraTabs;

        //Main tab stuff
        QLineEdit* m_txtBookName;
        QComboBox* m_cboMainCurrency;

        static QList<BookSettingsTabFactorySuper*> m_registeredTabs;
};

#endif // FORMBOOKSETTINGS_H

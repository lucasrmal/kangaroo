#ifndef FRMEXECUTEQUERIES_H
#define FRMEXECUTEQUERIES_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>

class QPlainTextEdit;
class QGroupBox;

class FormConsole : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        FormConsole(QWidget *parent = 0);

        void loadUI();

        QSize sizeHint() const { return QSize(450, 500); }

    public slots:
        void executeQuery();
        void clear();
        void load();
        void save();
        void close();

    private:
        QPlainTextEdit* txtQuery;
        QPlainTextEdit* txtResult;
        QPushButton* btnExecute;
        QPushButton* btnClear;
        QPushButton* btnLoad;
        QPushButton* btnSave;

        QGroupBox* grbQuery;
        QGroupBox* grbOutput;

        QString m_lastPath;

};

#endif // FRMEXECUTEQUERIES_H

#ifndef WELCOMESCREEN_H
#define WELCOMESCREEN_H

#include <QWidget>

class QPushButton;
class QListWidget;
class QListWidgetItem;

class WelcomeScreen : public QWidget
{
    Q_OBJECT

    public:
        explicit WelcomeScreen(QWidget *parent = nullptr);

    private slots:
        void createNewBook();

        void onRecentFileClicked(QListWidgetItem* _item);
        void loadRecentList();

    private:
        QPushButton* m_btnCreate;
        QPushButton* m_btnOpen;
        QPushButton* m_btnImport;
        QListWidget* m_listRecent;

        QWidget* m_welcomeWidget;
};

#endif // WELCOMESCREEN_H

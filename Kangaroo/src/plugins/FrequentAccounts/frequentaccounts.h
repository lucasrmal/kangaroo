#ifndef FREQUENTACCOUNTS_H
#define FREQUENTACCOUNTS_H

#include <QDockWidget>

class QToolButton;
class QVBoxLayout;
class QSignalMapper;

class FrequentAccounts : public QDockWidget
{
    Q_OBJECT

    public:
        explicit FrequentAccounts(QWidget *parent = 0);

    signals:

    public slots:
        void reloadAccounts();
        void openAccount(int _id);

    private:
        QVBoxLayout* m_layout;
        QList<QToolButton*> m_buttons;
        QSignalMapper* m_mapper;

        QWidget* m_central;

};

#endif // FREQUENTACCOUNTS_H

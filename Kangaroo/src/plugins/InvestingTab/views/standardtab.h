#ifndef STANDARDTAB_H
#define STANDARDTAB_H

#include <QWidget>

class Portfolio;
class QLabel;
class QStackedWidget;
class QPushButton;
class QVBoxLayout;
class QHBoxLayout;

class StandardTab : public QWidget
{
    Q_OBJECT

    public:
        explicit StandardTab(Portfolio* _portfolio, const QString& _title, QWidget *parent = 0);

        void addTab(const QString& _title, QWidget* _widget);

    signals:
        void refreshRequested();

    private slots:
        void buttonToggled(bool _toggled);
        void onPortfolioNameChanged();

    protected:
        Portfolio* m_portfolio;

        QVBoxLayout* m_mainLayout;
        QHBoxLayout* m_layoutTop;
        QHBoxLayout* m_layoutBottom;
        QHBoxLayout* m_layoutButton;

    private:
        QString m_title;

        QList<QPushButton*> m_buttons;
        QLabel*             m_lblTitle;
        QStackedWidget*     m_stack;

};

#endif // STANDARDTAB_H

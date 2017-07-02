#ifndef CREATEBOOKWIZARD_H
#define CREATEBOOKWIZARD_H

#include <QWizard>
#include <QHash>
#include <KangarooLib/model/currency.h>

class QComboBox;
class QLineEdit;
class QListWidget;

class SettingsPage : public QWizardPage
{
    Q_OBJECT

    public:
        SettingsPage(QWidget *parent);

    private:
        QLineEdit* m_lblName;
        QComboBox* m_cboCurrency;

};

class CurrenciesPage : public QWizardPage
{
    Q_OBJECT

    public:
        CurrenciesPage(QWidget *parent);

        void initializePage();

        QListWidget* m_listCurrencies;

};

class ConclusionPage : public QWizardPage
{
    Q_OBJECT

    public:
        ConclusionPage(QWidget *parent);

        bool validatePage();

    private:

};

class CreateBookWizard : public QWizard
{
    Q_OBJECT

    public:
        explicit CreateBookWizard(QWidget* parent = nullptr);

        QList<KLib::CurrencyManager::WorldCurrency> currencies() const;

    private:
        QHash<QString, KLib::CurrencyManager::WorldCurrency> m_currencies;

        CurrenciesPage* m_currencyPage;

};

#endif // CREATEBOOKWIZARD_H

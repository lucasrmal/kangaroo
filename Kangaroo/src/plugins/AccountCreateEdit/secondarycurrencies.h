#ifndef SECONDARYCURRENCIES_H
#define SECONDARYCURRENCIES_H

#include "accountedittab.h"

class QListWidget;

class SecondaryCurrencies : public AccountEditTab
{
    Q_OBJECT

    public:
        explicit SecondaryCurrencies(QWidget *parent = nullptr);

        QString tabName() const { return tr("Secondary Currencies"); }
        QString tabIcon() const { return "currency"; }

        bool enableIf(int _accountType) const;

        void fillData(const KLib::Account* _account);

        void save(KLib::Account* _account) const;

        QStringList validate() const;

    signals:

    private:
        QListWidget* m_listCurrencies;

};

#endif // SECONDARYCURRENCIES_H

#ifndef TAXSTATUSTAB_H
#define TAXSTATUSTAB_H

#include "../AccountCreateEdit/accountedittab.h"

class QGroupBox;
class QComboBox;

class TaxStatusTab : public AccountEditTab
{
    Q_OBJECT

    public:
        explicit TaxStatusTab(QWidget* _parent = nullptr);

        QString tabName() const { return tr("Tax Status"); }
        QString tabIcon() const { return "taxes-finances"; }

        bool enableIf(int _accountType) const;

        void fillData(const KLib::Account* _account);

        void save(KLib::Account* _account) const;

        QStringList validate() const;

        static const QString TAX_ITEM_PROPERTY;

    private:
        QGroupBox* m_grpTaxRelated;
        QComboBox* m_cboLineItem;

};

#endif // TAXSTATUSTAB_H

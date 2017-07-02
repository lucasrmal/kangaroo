#ifndef NEWPROPERTYWIZARD_H
#define NEWPROPERTYWIZARD_H

/*
 * FOR V2: New Real Estate Wizard

Intro
- Explain what wizard can help do
- Ask if want to proceed with New Real Estate Wizard. If no, exit.

Property value
- Ask what value of property is
- Ask if want to record purchase details or if want to create transfer from Opening Balances

Mortgage loan (only if choose to record purchase details)
- Choose a name and parent for the account or an existing account
- Enter an amount for the loan
- Enter loan %, compounding, etc.
- Payment schedule: monthly, bimonthly, etc.

Purchase details
- Can enter details about closing costs

Finalization
- Shows details of everything to be created.

*/

#include <QWizard>

class NewPropertyWizard : public QWizard
{
    public:
        NewPropertyWizard();
};

#endif // NEWPROPERTYWIZARD_H

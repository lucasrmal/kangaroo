#include "accountactions.h"
#include "../AccountCreateEdit/formeditaccount.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/ledger.h>
#include <KangarooLib/model/modelexception.h>

#include <QMessageBox>

using namespace KLib;

void AccountActions::createAccount(Account* _parent)
{
    FormEditAccount formCreate(nullptr, Core::instance()->mainWindow());
    formCreate.setParentAccount(_parent);
    formCreate.exec();
}

void AccountActions::editAccount(Account* _account)
{
    if (_account)
    {
        FormEditAccount formEdit(_account, Core::instance()->mainWindow());
        formEdit.exec();
    }
}

bool AccountActions::closeAccount(Account* _account)
{
    if (!(_account && _account->canBeClosed()))
    {
        return false;
    }


    QString msg = QObject::tr("Close this account: <b>%1</b>? <br><br>"
                              "Note that if schedules are linked with the account, they will be deleted. "
                              "Reopening the account will not "
                              "recreate these schedules.").arg(_account->name());

    if (QMessageBox::question(Core::instance()->mainWindow(),
                              QObject::tr("Close Account"),
                              msg,
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {
        try
        {
            _account->setOpen(false);
            return true;
        }
        catch (ModelException e)
        {
            QMessageBox::warning(Core::instance()->mainWindow(),
                                 QObject::tr("Close Account"),
                                 QObject::tr("An error has occured while closing the account:\n\n%1").arg(e.description()));
        }
    }

    return false;
}

bool AccountActions::reopenAccount(Account* _account)
{
    if (!_account) return false;

    if (QMessageBox::question(Core::instance()->mainWindow(),
                              QObject::tr("Reopen Account"),
                              QObject::tr("Reopen account %1 ?").arg(_account->name()),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {

        try
        {
            _account->setOpen(true);
            return true;
        }
        catch (ModelException e)
        {
            QMessageBox::warning(Core::instance()->mainWindow(),
                                 QObject::tr("Reopen Account"),
                                 QObject::tr("An error has occured while re-opening the account:\n\n%1").arg(e.description()));
        }
    }

    return false;
}

bool AccountActions::removeAccount(Account* _account)
{
    if (!(_account && _account->canBeRemoved()))
    {
        return false;
    }

    if (QMessageBox::question(Core::instance()->mainWindow(),
                              QObject::tr("Confirm Delete"),
                              QObject::tr("Delete this account: <b>%1</b>?").arg(_account->name())
                              + "<br><br>"
                              + QObject::tr("<b>This action cannot be undone.</b>")
                              + "<br><br>"
                              + QObject::tr("Note that if schedules are linked with the account, they will be deleted."),
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::No) == QMessageBox::Yes)
    {
        try
        {
            _account->parent()->removeChild(_account->id());
            return true;
        }
        catch (ModelException e)
        {
            QMessageBox::warning(Core::instance()->mainWindow(),
                                 QObject::tr("Remove Account"),
                                 QObject::tr("An error has occured while removing the account:\n\n%1").arg(e.description()));
        }
    }

    return false;
}

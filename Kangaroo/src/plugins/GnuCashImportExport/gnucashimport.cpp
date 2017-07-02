#include "gnucashimport.h"
#include <KangarooLib/ui/core.h>
#include <KangarooLib/controller/io.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/payee.h>
#include <KangarooLib/model/institution.h>
#include <KangarooLib/model/transactionmanager.h>
#include <KangarooLib/model/investmenttransaction.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/model/ledger.h>

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamAttributes>
#include <QApplication>

using namespace KLib;

const char* GNUCashTags::ROOT = "KMYMONEY-FILE";
const char* GNUCashTags::FILEINFO = "FILEINFO";
const char* GNUCashTags::INSTITUTIONS = "INSTITUTIONS";
const char* GNUCashTags::INSTITUTION = "INSTITUTION";
const char* GNUCashTags::PAYEES = "PAYEES";
const char* GNUCashTags::PAYEE = "PAYEE";
const char* GNUCashTags::CURRENCIES = "CURRENCIES";
const char* GNUCashTags::CURRENCY = "CURRENCY";
const char* GNUCashTags::SECURITIES = "SECURITIES";
const char* GNUCashTags::SECURITY = "SECURITY";
const char* GNUCashTags::ACCOUNTS = "ACCOUNTS";
const char* GNUCashTags::ACCOUNT = "ACCOUNT";
const char* GNUCashTags::PAIR = "PAIR";
const char* GNUCashTags::PRICES = "PRICES";
const char* GNUCashTags::PRICEPAIR = "PRICEPAIR";
const char* GNUCashTags::PRICE = "PRICE";
const char* GNUCashTags::TRANSACTIONS = "TRANSACTIONS";
const char* GNUCashTags::TRANSACTION = "TRANSACTION";
const char* GNUCashTags::SPLIT = "SPLIT";
const char* GNUCashTags::SCHEDULES = "SCHEDULES";
const char* GNUCashTags::BUDGETS = "BUDGETS";

GnuCashImport::GnuCashImport()
{
}

QString GnuCashImport::fileType() const
{
    return QObject::tr("GnuCash file") + " (*.gnucash)";
}

QString GnuCashImport::description() const
{
    return QObject::tr("GnuCash file");
}

QIcon GnuCashImport::icon() const
{
    return Core::icon("gnucash");
}

void GnuCashImport::import(const QString& _path, ErrorList& _errors)
{
    /* Cursor */
    QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

    try
    {


    }
    catch (ModelException e)
    {
        _errors << Error(Critical,
                         QObject::tr("Invalid data discovered in the file: %1").arg(e.description()));
    }

    catch (IOException e)
    {
        _errors << Error(Critical,
                         QObject::tr("An error occured while parsing the file: %1").arg(e.description()));
    }

    /* Cursor */
    QApplication::restoreOverrideCursor();
}


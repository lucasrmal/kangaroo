#include "formupdateprices.h"

#include <KangarooLib/controller/onlinequotes.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/ui/core.h>
#include <QTextEdit>

using namespace KLib;

FormUpdatePrices::FormUpdatePrices(QWidget *parent) :
    CAMSEGDialog(DialogWithPicture, CloseButton, parent)
{
    m_textEdit = new QTextEdit(this);
    setCentralWidget(m_textEdit);
    m_textEdit->setReadOnly(true);

    connect(OnlineQuotes::instance(), SIGNAL(requestError(QString)), this, SLOT(onRequestError(QString)));
    connect(OnlineQuotes::instance(), SIGNAL(requestDone()), this, SLOT(onRequestDone()));
    connect(OnlineQuotes::instance(), SIGNAL(quoteReady(KLib::PricePair,double)), this, SLOT(onQuoteReady(KLib::PricePair,double)));

    m_textEdit->insertHtml("<b>Begin Update...</b><br />");
    OnlineQuotes::instance()->updateAll();

    setTitle(tr("Update Prices"));
    setWindowTitle(title());
    setPicture(Core::pixmap("download"));
}

void FormUpdatePrices::onQuoteReady(const PricePair& _pair, double _quote)
{
    m_textEdit->insertHtml(tr("Updated %1-%2 : %3<br />").arg(_pair.first).arg(_pair.second).arg(_quote));
    m_textEdit->ensureCursorVisible();
}

void FormUpdatePrices::onRequestError(const QString& _message)
{
    m_textEdit->insertHtml(QString("<span color='red'>%1</span><br />").arg(_message));
    m_textEdit->ensureCursorVisible();
}

void FormUpdatePrices::onRequestDone()
{
    m_textEdit->insertHtml(QString("<b>%1</b>").arg(tr("Done!")));
    m_textEdit->ensureCursorVisible();
}

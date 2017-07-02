#ifndef FORMUPDATEPRICES_H
#define FORMUPDATEPRICES_H

#include <KangarooLib/ui/dialogs/camsegdialog.h>
#include <KangarooLib/model/pricemanager.h>

class QTextEdit;

class FormUpdatePrices : public KLib::CAMSEGDialog
{
    Q_OBJECT

    public:
        explicit FormUpdatePrices(QWidget *parent = 0);

        QSize sizeHint() const { return QSize(400, 500); }

    signals:

    public slots:
        void onQuoteReady(const KLib::PricePair& _pair, double _quote);
        void onRequestError(const QString& _message);
        void onRequestDone();

    private:
        QTextEdit* m_textEdit;

};

#endif // FORMUPDATEPRICES_H

#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include <QTableView>

enum FontType
{
    Bold,
    Italic,
    Underline
};

enum ColorType
{
    Background,
    Foreground
};

class Spreadsheet : public QTableView
{
    Q_OBJECT

    public:
        explicit Spreadsheet(QWidget *parent = 0);

        bool hasSelectedCells() const;

        void deleteContentsOfSelection() const;
        void setSelectionFont(const QFont& _font);
        void setSelectionFont(bool _value, FontType _type);
        void setSelectionColor(const QColor& _color, ColorType _type);
        void setSelectionAlignment(Qt::Alignment _align);

        QFont selectionFont() const;
        QColor selectionColor(ColorType _type) const;
        bool selectionFont(FontType _type) const;

    protected:
        void keyPressEvent(QKeyEvent* _event);


    signals:

    public slots:

};

#endif // SPREADSHEET_H

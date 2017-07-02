#ifndef IHOMEWIDGET
#define IHOMEWIDGET

#include <QWidget>
#include <QIcon>
#include <QFont>
#include <QColor>
#include <functional>

class IHomeWidget : public QWidget
{
    Q_OBJECT

    public:
        IHomeWidget(QWidget* _centralWidget, QWidget* _parent = nullptr);

        static const QFont   TITLEBAR_FONT;
        static const int     TITLEBAR_HEIGHT;
        static const QColor  TITLEBAR_BACKCOLOR;
        static const QColor  WIDGET_BACKCOLOR;

    protected:
        void paintEvent(QPaintEvent* _event);

        virtual QString title() const = 0;

        QWidget* centralWidget() const { return m_widget; }

    public slots:
        virtual void configure() = 0;   ///< Will be called if the user clicks on the "Configure" button.

    private:
        QWidget* m_widget;
};

struct HomeWidgetInfo
{
    typedef std::function<IHomeWidget*(QWidget*)> fn_build;

    HomeWidgetInfo() :
        showByDefault(false),
        build([](QWidget*) { return nullptr; }) {}

    HomeWidgetInfo(const QString& _code,
                   const QString& _name,
                   const QString& _description,
                   const QIcon& _icon,
                   bool _showByDefault,
                   const fn_build& _build) :
        code(_code),
        name(_name),
        description(_description),
        icon(_icon),
        showByDefault(_showByDefault),
        build(_build) {}


    QString  code;          ///< Unique code
    QString  name;          ///< Name of the widget. Should be a translated string
    QString  description;   ///< Description. Will be showed to the user in the Select Widget dialog.
    QIcon    icon;          ///< Icon representing the widget
    bool     showByDefault; ///< If the Home tab is not configured yet, will show this widget if set to True.
    fn_build build;         ///< Takes the parent widget in parameter and returns the Home Widget
};

#endif // IHOMEWIDGET


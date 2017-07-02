#include "welcomescreen.h"

#include <QPushButton>
#include <QListWidget>
#include <QMenu>
#include <QGridLayout>
#include <QLabel>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>

#include "../CreateBook/createbookwizard.h"

using namespace KLib;

WelcomeScreen::WelcomeScreen(QWidget *parent) : QWidget(parent)
{
    auto createFont = [] (int pointSize, bool bold = false)
    {
        QFont f;
        f.setPointSize(pointSize);
        f.setBold(bold);
        return f;
    };

    QWidget* m_welcomeWidget = new QWidget(this);
    //m_welcomeWidget->setFixedSize(760, 300);
    m_welcomeWidget->setFixedHeight(310);
//    QPalette p;
//    p.setColor(QPalette::Background, Qt::white);
//    m_welcomeWidget->setPalette(p);
//    m_welcomeWidget->setAutoFillBackground(true);

    //Center the widget in the middle
    QHBoxLayout* vbox = new QHBoxLayout();
    vbox->addStretch(5);
    vbox->addWidget(m_welcomeWidget);
    vbox->addStretch(5);

    QHBoxLayout* hbox = new QHBoxLayout(this);
    hbox->addStretch(5);
    hbox->addLayout(vbox);
    hbox->addStretch(5);

    //Labels and logo
    QLabel* lblLogo = new QLabel(m_welcomeWidget);
    QLabel* lblTitle = new QLabel(QString("%1 %2").arg(Core::APP_NAME).arg(Core::APP_VERSION), m_welcomeWidget);
    QLabel* lblWelcome = new QLabel(tr("Welcome! What would you like to do?"), m_welcomeWidget);

    lblLogo->setFixedSize(QSize(80,80));
    lblLogo->setPixmap(Core::pixmap("kangaroo80"));

    lblTitle->setFont(createFont(40));
    lblWelcome->setFont(createFont(16));
    lblTitle->setMinimumHeight(80);
    lblTitle->setAlignment(Qt::AlignVCenter);
    //lblTitle->setMinimumWidth(220);

    lblWelcome->setMaximumHeight(20);
    lblWelcome->setFont(createFont(13));

    //Buttons
    auto createButton = [m_welcomeWidget] (QPushButton** button, const QString& icon, const QString& text)
    {
        *button = new QPushButton(Core::icon(icon), text, m_welcomeWidget);
        (*button)->setFlat(true);
        (*button)->setStyleSheet("text-align: left");
    };

    createButton(&m_btnCreate, "new-book",       tr("Create a new book"));
    createButton(&m_btnOpen,   "open",           tr("Open a book"));
    createButton(&m_btnImport, "document-import", tr("Import my data"));

    connect(m_btnCreate, &QPushButton::clicked, this, &WelcomeScreen::createNewBook);
    connect(m_btnOpen,   &QPushButton::clicked, Core::instance(), &Core::selectOpenFile);

    m_btnImport->setMenu(ActionManager::instance()->actionContainer("File.Import")->menu());

    //Recent List
    m_listRecent = new QListWidget(m_welcomeWidget);
    m_listRecent->setMaximumSize(420, 250);

    QLabel* lblRecent = new QLabel(tr("Open a recent book:"), m_welcomeWidget);

    connect(m_listRecent,       &QListWidget::itemDoubleClicked, this, &WelcomeScreen::onRecentFileClicked);
    connect(Core::instance(),   &Core::recentFileListModified,   this, &WelcomeScreen::loadRecentList);

    //Layout
    QGridLayout* layout = new QGridLayout(m_welcomeWidget);
    layout->addWidget(lblLogo,      0, 0);           //Row, Col, RowSpan, ColSpan
    layout->addWidget(lblTitle,     0, 1, 1, 2);
    layout->addItem(new QSpacerItem(10, 20),
                                    1, 1, 1, 3);
    layout->addWidget(lblWelcome,   2, 0, 1, 3);
    layout->addItem(new QSpacerItem(10, 40),
                                    2, 1, 1, 3);

    layout->addWidget(m_btnCreate,  5, 0, 1, 2);
    layout->addWidget(m_btnOpen,    6, 0, 1, 2);
    layout->addWidget(m_btnImport,  7, 0, 1, 2);

    layout->addWidget(lblRecent,    4, 2, 1, 1);
    layout->addWidget(m_listRecent, 5, 2, 4, 1);
    layout->addItem(new QSpacerItem(10, 50), 9, 1, 1, 3);

    loadRecentList();
}

void WelcomeScreen::onRecentFileClicked(QListWidgetItem* _item)
{
    if (_item)
    {
        Core::instance()->openFile(_item->text());
    }
}

void WelcomeScreen::loadRecentList()
{
    m_listRecent->clear();

    for (QString f : Core::instance()->recentFileList())
    {
        m_listRecent->addItem(f);
    }
}

void WelcomeScreen::createNewBook()
{
    if (!Core::instance()->unloadFile())
        return;

    CreateBookWizard* w = new CreateBookWizard(Core::instance()->mainWindow());
    w->exec();
    delete w;
}



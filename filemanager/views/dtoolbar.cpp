#include "dtoolbar.h"
#include "dicontextbutton.h"
#include "dcheckablebutton.h"
#include "dsearchbar.h"
#include "dtabbar.h"
#include "dcrumbwidget.h"
#include "historystack.h"
#include "dhoverbutton.h"

#include "../app/fmevent.h"
#include "../app/global.h"
#include "../app/filesignalmanager.h"

#include "../views/filemenumanager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include "historystack.h"
#include "dhoverbutton.h"
#include "windowmanager.h"
#include <QDebug>

const int DToolBar::ButtonHeight = 20;

DToolBar::DToolBar(QWidget *parent) : QFrame(parent)
{
    initData();
    initUI();
    initConnect();
    startup();
}

DToolBar::~DToolBar()
{
    delete m_navStack;
}

void DToolBar::initData()
{
    m_navStack = new HistoryStack(50);
}

void DToolBar::initUI()
{
    initAddressToolBar();
    initContollerToolBar();

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_addressToolBar);
    mainLayout->addWidget(m_contollerToolBar);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
}

void DToolBar::initAddressToolBar()
{
    m_addressToolBar = new QFrame;
    m_addressToolBar->setObjectName("AddressToolBar");
    m_addressToolBar->setFixedHeight(40);

    QHBoxLayout * backForwardLayout = new QHBoxLayout;


    m_backButton = new DStateButton(":/icons/images/icons/backward_normal.png", this);
    m_backButton->setObjectName("backButton");
    m_backButton->setFixedWidth(25);
    m_backButton->setFixedHeight(20);
    m_backButton->setDisabled(true);
    m_backButton->setFocusPolicy(Qt::NoFocus);
    m_forwardButton = new DStateButton(":/icons/images/icons/forward_normal.png", this);
    m_forwardButton->setObjectName("forwardButton");
    m_forwardButton->setFixedWidth(25);
    m_forwardButton->setFixedHeight(20);
    m_forwardButton->setDisabled(true);
    m_forwardButton->setFocusPolicy(Qt::NoFocus);

    backForwardLayout->addWidget(m_backButton);
    backForwardLayout->addWidget(m_forwardButton);
    backForwardLayout->setSpacing(0);
    backForwardLayout->setContentsMargins(0, 0, 0, 0);

    m_searchBar = new DSearchBar;
    m_searchBar->setPlaceholderText("Search or enter address");
    m_searchBar->setAlignment(Qt::AlignHCenter);

    m_crumbWidget = new DCrumbWidget;

    QHBoxLayout * comboLayout = new QHBoxLayout;
    comboLayout->addWidget(m_crumbWidget);
    comboLayout->addWidget(m_searchBar);
    comboLayout->setSpacing(0);
    comboLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(backForwardLayout);
    mainLayout->addLayout(comboLayout);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(10);
    m_addressToolBar->setLayout(mainLayout);
}

void DToolBar::initContollerToolBar()
{
    m_contollerToolBar = new QFrame;
    m_contollerToolBar->setObjectName("ContollerToolBar");
    m_contollerToolBar->setFixedHeight(40);
    m_iconViewButton = new QPushButton(this);
    m_iconViewButton->setGeometry(QRect(0,0,26,20));
    m_iconViewButton->setFixedHeight(20);
    m_iconViewButton->setObjectName("iconViewButton");
    m_iconViewButton->setCheckable(true);
    m_iconViewButton->setChecked(true);
    m_iconViewButton->setFocusPolicy(Qt::NoFocus);

    m_listViewButton = new QPushButton(this);
    m_listViewButton->setGeometry(QRect(0,0,26,20));
    m_listViewButton->setFixedHeight(20);
    m_listViewButton->setObjectName("listViewButton");
    m_listViewButton->setCheckable(true);
    m_listViewButton->setFocusPolicy(Qt::NoFocus);

    m_viewButtonGroup = new QButtonGroup(this);
    m_viewButtonGroup->addButton(m_iconViewButton, 0);
    m_viewButtonGroup->addButton(m_listViewButton, 1);

    m_sortingButton = new DHoverButton(":/icons/images/icons/expanded_view_normal.svg",
                                       ":/icons/images/icons/expanded_view_checked.svg",
                                        this);
    m_sortingButton->setFixedHeight(20);
    m_sortingButton->setFixedWidth(26);
    m_sortingButton->setObjectName("SortingButton");
    m_sortingButton->setFocusPolicy(Qt::NoFocus);
    m_sortingButton->setMenu(FileMenuManager::createToolBarSortMenu());

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_iconViewButton);
    mainLayout->addWidget(m_listViewButton);
    mainLayout->addWidget(m_sortingButton);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(0);
    m_contollerToolBar->setLayout(mainLayout);
}

void DToolBar::initConnect()
{
    connect(m_iconViewButton, &DStateButton::clicked, this, &DToolBar::requestIconView);
    connect(m_listViewButton, &DStateButton::clicked, this, &DToolBar::requestListView);
    connect(m_backButton, &DStateButton::clicked,this, &DToolBar::backButtonClicked);
    connect(m_forwardButton, &DStateButton::clicked,this, &DToolBar::forwardButtonClicked);
    connect(m_searchBar, &DSearchBar::returnPressed, this, &DToolBar::searchBarTextEntered);
    connect(m_crumbWidget, &DCrumbWidget::crumbSelected, this, &DToolBar::crumbSelected);
    connect(m_searchBar, &DSearchBar::searchBarFocused, this, &DToolBar::searchBarActivated);
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged, this, &DToolBar::crumbChanged);
    connect(m_searchBar, SIGNAL(searchBarFocused()), this, SLOT(searchBarActivated()));
    connect(m_searchBar->getClearAction(), &QAction::triggered, this, &DToolBar::searchBarDeactivated);
}

void DToolBar::startup()
{
    QString user = getenv("USER");
    FMEvent event(-1, FMEvent::SearchLine);
    event = "/home/" + user;
    m_crumbWidget->setCrumb(event.fileUrl());

    emit fileSignalManager->requestChangeCurrentUrl(event);
}



void DToolBar::searchBarActivated()
{
    qDebug() << "active";
    m_crumbWidget->hide();
    m_searchBar->setAlignment(Qt::AlignLeft);
    m_searchBar->clear();
    m_searchBar->setActive(true);
    m_searchBar->setClearAction();
}

void DToolBar::searchBarDeactivated()
{
    m_crumbWidget->show();
    m_searchBar->clear();
    m_searchBar->setAlignment(Qt::AlignHCenter);
    m_searchBar->setActive(false);
    m_searchBar->removeClearAction();
    m_searchBar->window()->setFocus();
}


/**
 * @brief DToolBar::searchBarTextEntered
 *
 * Set the tab bar when return press is detected
 * on search bar.
 */
void DToolBar::searchBarTextEntered()
{
    QString path = m_searchBar->text();
    qDebug() << path;

    FMEvent event;

    event = FMEvent::SearchLine;
    event = path;

    emit fileSignalManager->requestChangeCurrentUrl(event);
}

void DToolBar::crumbSelected(const FMEvent &e)
{
    if(e.windowId() != WindowManager::getWindowId(window()))
        return;

    FMEvent event;

    event = WindowManager::getWindowId(window());
    event = FMEvent::CrumbButton;
    event = QUrl::fromLocalFile(e.fileUrl()).toString();


    emit fileSignalManager->requestChangeCurrentUrl(event);
}

void DToolBar::crumbChanged(const FMEvent &event)
{
    if(event.windowId() != WindowManager::getWindowId(window()))
        return;

    if(event.source() == FMEvent::CrumbButton)
        return;
    QUrl qurl(event.fileUrl());
    m_crumbWidget->setCrumb(qurl.toLocalFile());
    if(m_searchBar->isActive())
        m_searchBar->setText(qurl.toLocalFile());
    if(event.source() == FMEvent::BackAndForwardButton)
        return;
    m_navStack->insert(event.fileUrl());
    m_backButton->setEnabled(true);
}

/**
 * @brief DToolBar::upButtonClicked
 *
 * Move or shrink to the given index of the tabs depending
 * on the amount of tabs shown. This will be triggered when
 * up button is clicked.
 */
void DToolBar::upButtonClicked()
{
    QString text = m_crumbWidget->back();

    FMEvent event;

    event = FMEvent::UpButton;
    event = text;

    emit fileSignalManager->requestChangeCurrentUrl(event);
}

void DToolBar::searchBarChanged(QString path)
{
    m_searchBar->setText(path);
}

void DToolBar::backButtonClicked()
{
    QString path = m_navStack->back();
    if(path != "")
    {
        FMEvent event;
        event = WindowManager::getWindowId(window());
        event = FMEvent::BackAndForwardButton;
        event = path;
        if(m_navStack->isFirst())
            m_backButton->setDisabled(true);
        m_forwardButton->setEnabled(true);
        emit fileSignalManager->requestChangeCurrentUrl(event);
    }
}

void DToolBar::forwardButtonClicked()
{
    QString path = m_navStack->forward();
    if(path != "")
    {
        FMEvent event;
        event = WindowManager::getWindowId(window());
        event = FMEvent::BackAndForwardButton;
        event = path;
        if(m_navStack->isLast())
            m_forwardButton->setDisabled(true);
        m_backButton->setEnabled(true);
        emit fileSignalManager->requestChangeCurrentUrl(event);
    }
}



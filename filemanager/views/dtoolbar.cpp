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
    m_navStack = new HistoryStack(65536);
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

    m_searchButton = new DStateButton(":/images/images/light/appbar.magnify.png", this);
    m_searchButton->setObjectName("searchButton");
    m_searchButton->setFixedWidth(25);
    m_searchButton->setFixedHeight(20);
    m_searchButton->setFocusPolicy(Qt::NoFocus);

    backForwardLayout->addWidget(m_backButton);
    backForwardLayout->addWidget(m_forwardButton);
    backForwardLayout->setSpacing(0);
    backForwardLayout->setContentsMargins(0, 0, 0, 0);


    QFrame * crumbAndSearch = new QFrame;
    m_searchBar = new DSearchBar;
    m_searchBar->hide();
    m_searchBar->setAlignment(Qt::AlignHCenter);
    m_crumbWidget = new DCrumbWidget;
    crumbAndSearch->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    QHBoxLayout * comboLayout = new QHBoxLayout;
    comboLayout->addWidget(m_crumbWidget);
    comboLayout->addWidget(m_searchBar);
    comboLayout->addWidget(m_searchButton);
    comboLayout->setSpacing(0);
    comboLayout->setContentsMargins(0, 0, 0, 0);

    crumbAndSearch->setLayout(comboLayout);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addLayout(backForwardLayout);
    mainLayout->addWidget(crumbAndSearch);
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

    m_extendButton = new QPushButton(this);
    m_extendButton->setGeometry(QRect(0,0,26,20));
    m_extendButton->setFixedHeight(20);
    m_extendButton->setObjectName("hierarchicalButton");
    m_extendButton->setCheckable(true);
    m_extendButton->setFocusPolicy(Qt::NoFocus);

    m_viewButtonGroup = new QButtonGroup(this);
    m_viewButtonGroup->addButton(m_iconViewButton, 0);
    m_viewButtonGroup->addButton(m_listViewButton, 1);
    m_viewButtonGroup->addButton(m_extendButton, 2);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_iconViewButton);
    mainLayout->addWidget(m_listViewButton);
    mainLayout->addWidget(m_extendButton);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(0);
    m_contollerToolBar->setLayout(mainLayout);
}

void DToolBar::initConnect()
{
    connect(m_iconViewButton, &DStateButton::clicked, this, &DToolBar::requestIconView);
    connect(m_listViewButton, &DStateButton::clicked, this, &DToolBar::requestListView);
    connect(m_extendButton, &DStateButton::clicked, this, &DToolBar::requestExtendView);
    connect(m_backButton, &DStateButton::clicked,this, &DToolBar::backButtonClicked);
    connect(m_forwardButton, &DStateButton::clicked,this, &DToolBar::forwardButtonClicked);
    connect(m_searchBar, &DSearchBar::returnPressed, this, &DToolBar::searchBarTextEntered);
    connect(m_crumbWidget, &DCrumbWidget::crumbSelected, this, &DToolBar::crumbSelected);
    connect(m_crumbWidget, &DCrumbWidget::searchBarActivated, this, &DToolBar::searchBarActivated);
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged, this, &DToolBar::crumbChanged);
    connect(m_searchButton, &DStateButton::clicked, this, &DToolBar::searchBarClicked);
    connect(m_searchBar, SIGNAL(focusedOut()), this, SLOT(searchBarDeactivated()));
}

void DToolBar::startup()
{
    QString user = getenv("USER");
    FMEvent event(-1, FMEvent::SearchLine);
    event = DUrl::fromLocalFile("/home/" + user);
    m_crumbWidget->setCrumb(event.fileUrl());

    emit fileSignalManager->requestChangeCurrentUrl(event);
}

void DToolBar::searchBarClicked()
{
    if(!m_searchState)
    {
        searchBarActivated();
        m_searchState = true;
    }
    else
    {
        searchBarDeactivated();
        m_searchState = false;
    }
}

void DToolBar::searchBarActivated()
{
    m_searchBar->setPlaceholderText(tr("Search or enter address"));
    m_searchBar->show();
    m_crumbWidget->hide();
    m_searchBar->setAlignment(Qt::AlignLeft);
    m_searchBar->clear();
    m_searchBar->setActive(true);
    m_searchBar->setFocus();
    m_searchBar->setCurrentPath(m_crumbWidget->getUrl());
}

void DToolBar::searchBarDeactivated()
{
    m_searchBar->setPlaceholderText("");
    m_searchBar->hide();
    m_crumbWidget->show();
    m_searchBar->clear();
    m_searchBar->setAlignment(Qt::AlignHCenter);
    m_searchBar->setActive(false);
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
    QString text = m_searchBar->text();

    FMEvent event;

    event = WindowManager::getWindowId(window());
    event = FMEvent::SearchLine;
    event = DUrl::fromUserInput(text);

    if(m_searchBar->hasScheme() || m_searchBar->isPath())
    {
        event = DUrl::fromUserInput(text);
    }
    else
    {
        DUrl url = DUrl::fromSearchFile(m_crumbWidget->path());
        url.setQuery(text);
        event = url;
    }

    emit fileSignalManager->requestChangeCurrentUrl(event);
}

void DToolBar::crumbSelected(const FMEvent &e)
{
    if(e.windowId() != WindowManager::getWindowId(window()))
        return;

    FMEvent event;

    event = WindowManager::getWindowId(window());
    event = FMEvent::CrumbButton;
    event = e.fileUrl();


    emit fileSignalManager->requestChangeCurrentUrl(event);
}

void DToolBar::crumbChanged(const FMEvent &event)
{
    if(event.windowId() != WindowManager::getWindowId(window()))
        return;

    if(event.source() == FMEvent::CrumbButton)
        return;

    m_crumbWidget->setCrumb(event.fileUrl());

    if(event.source() == FMEvent::BackAndForwardButton)
        return;
    m_navStack->insert(event.fileUrl());
    if(m_navStack->size() > 1)
        m_backButton->setEnabled(true);
    else
        m_backButton->setEnabled(false);
}

/**
 * @brief DToolBar::upButtonClicked
 *
 * Move or shrink to the given index of the tabs depending
 * on the amount of tabs shown. This will be triggered when
 * up button is clicked.
 */

void DToolBar::searchBarChanged(QString path)
{
    m_searchBar->setText(path);
}

void DToolBar::backButtonClicked()
{
    DUrl url = m_navStack->back();
    if(!url.isEmpty())
    {
        FMEvent event;
        event = WindowManager::getWindowId(window());
        event = FMEvent::BackAndForwardButton;
        event = url;
        if(m_navStack->isFirst())
            m_backButton->setDisabled(true);
        m_forwardButton->setEnabled(true);
        emit fileSignalManager->requestChangeCurrentUrl(event);
    }
}

void DToolBar::forwardButtonClicked()
{
    DUrl url = m_navStack->forward();
    if(!url.isEmpty())
    {
        FMEvent event;
        event = WindowManager::getWindowId(window());
        event = FMEvent::BackAndForwardButton;
        event = url;
        if(m_navStack->isLast())
            m_forwardButton->setDisabled(true);
        m_backButton->setEnabled(true);
        emit fileSignalManager->requestChangeCurrentUrl(event);
    }
}

void DToolBar::checkViewModeButton(DFileView::ViewMode mode)
{
    switch (mode) {
    case DFileView::IconMode:
        m_iconViewButton->setChecked(true);
        break;
    case DFileView::ListMode:
        m_listViewButton->setChecked(true);
        break;
    case DFileView::ExtendMode:
        m_extendButton->setChecked(true);
        break;
    default:
        break;
    }
}



#include "dtoolbar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "dicontextbutton.h"
#include "dsearchbar.h"
#include "dtabbar.h"
#include "../app/global.h"
#include "dcrumbwidget.h"
#include <QDebug>

const int DToolBar::ButtonHeight = 20;

DToolBar::DToolBar(QWidget *parent) : QFrame(parent)
{
    initData();
    initUI();
    initConnect();
}

DToolBar::~DToolBar()
{

}

void DToolBar::initData()
{

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

    m_backButton = new DStateButton(":/icons/images/icons/backward_normal.png", this);
    m_backButton->setObjectName("backButton");
    m_backButton->setFixedWidth(25);
    m_backButton->setFixedHeight(20);
    m_forwardButton = new DStateButton(":/icons/images/icons/forward_normal.png", this);
    m_forwardButton->setObjectName("forwardButton");
    m_forwardButton->setFixedWidth(25);
    m_forwardButton->setFixedHeight(20);
    m_upButton = new DStateButton(":/images/images/dark/appbar.arrow.up.png", this);
    m_searchButton = new DStateButton(":/images/images/dark/appbar.magnify.png", this);
    m_refreshButton = new DStateButton(":/images/images/dark/appbar.refresh.png", this);

    m_searchBar = new DSearchBar;
    m_searchBar->setPlaceholderText("Enter address");
    m_searchBar->setAlignment(Qt::AlignHCenter);

    m_crumbWidget = new DCrumbWidget;

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_backButton);
    mainLayout->addWidget(m_forwardButton);
    mainLayout->addWidget(m_upButton);
    mainLayout->addWidget(m_crumbWidget);
    mainLayout->addWidget(m_searchBar);
    mainLayout->addWidget(m_searchButton);
    mainLayout->addWidget(m_refreshButton);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(0);
    m_addressToolBar->setLayout(mainLayout);
}

void DToolBar::initContollerToolBar()
{
    m_contollerToolBar = new QFrame;
    m_contollerToolBar->setObjectName("ContollerToolBar");
    m_contollerToolBar->setFixedHeight(40);
    m_layoutButton = new DStateButton(":/images/images/dark/appbar.layout.body.png",
                                      ":/images/images/dark/appbar.layout.sidebar.png",
                                      this);

    m_sortButton = new DStateButton(":/images/images/dark/appbar.sort.png", this);

    m_hideShowButton = new DStateButton(":/images/images/dark/appbar.eye.hide.png",
                                        ":/images/images/dark/appbar.eye.png",
                                        this);

    m_viewSwitchButton = new DStateButton(":/images/images/dark/appbar.tiles.four.png",
                                          ":/images/images/dark/appbar.list.png",
                                          this);
    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_layoutButton);
    mainLayout->addWidget(m_sortButton);
    mainLayout->addWidget(m_hideShowButton);
    mainLayout->addWidget(m_viewSwitchButton);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    m_contollerToolBar->setLayout(mainLayout);
}

void DToolBar::initConnect()
{
    connect(m_layoutButton, SIGNAL(clicked()), this, SIGNAL(requestSwitchLayout()));
    connect(m_backButton, &DStateButton::clicked,
            this, &DToolBar::backButtonClicked);
    connect(m_viewSwitchButton, &DStateButton::clicked,
            this, &DToolBar::switchLayoutMode);
    //m_searchButton released --> searchBarSwitched
    connect(m_searchButton, SIGNAL(released()), this, SLOT(searchBarSwitched()));
    //search bar return pressed --> searchBarTextEntered
    connect(m_searchBar, SIGNAL(returnPressed()), this, SLOT(searchBarTextEntered()));
    //tab bar clicked --> tabBarClicked
    connect(m_crumbWidget, &DCrumbWidget::crumbSelected, this, &DToolBar::crumbSelected);
    //up button released --> upButtonClicked
    connect(m_upButton, SIGNAL(released()), this, SLOT(upButtonClicked()));
    connect(m_refreshButton, &DStateButton::clicked, this, &DToolBar::refreshButtonClicked);
    connect(m_searchBar, SIGNAL(searchBarFocused()), this, SLOT(searchBarActivated()));
    connect(fileSignalManager, &FileSignalManager::currentUrlChanged, this, &DToolBar::crumbChanged);
}

DStateButton::ButtonState DToolBar::getLayoutbuttonState()
{
    return m_layoutButton->getButtonState();
}

void DToolBar::setLayoutButtonState(DStateButton::ButtonState state)
{
    m_layoutButton->setButtonState(state);
}

void DToolBar::searchBarSwitched()
{
    m_switchState = !m_switchState;
    if(m_switchState)
    {
        m_crumbWidget->hide();
        m_searchBar->setAlignment(Qt::AlignLeft);
        m_searchBar->clear();
    }
    else
    {
        m_crumbWidget->show();
        m_searchBar->setAlignment(Qt::AlignHCenter);
    }
}

void DToolBar::searchBarActivated()
{
    m_crumbWidget->hide();
    m_searchBar->setAlignment(Qt::AlignLeft);
    m_searchBar->clear();
    QAction * action = m_searchBar->setClearAction();
    connect(action, &QAction::triggered, this, &DToolBar::searchBarDeactivated);
    disconnect(m_searchBar, SIGNAL(searchBarFocused()), this, SLOT(searchBarActivated()));
}

void DToolBar::searchBarDeactivated()
{
    m_crumbWidget->show();
    m_searchBar->clear();
    m_searchBar->setAlignment(Qt::AlignHCenter);
    QAction * action = m_searchBar->removeClearAction();
    disconnect(action, &QAction::triggered, this, &DToolBar::searchBarDeactivated);
    connect(m_searchBar, SIGNAL(searchBarFocused()), this, SLOT(searchBarActivated()));
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
    m_crumbWidget->setCrumb(path);
    emit fileSignalManager->currentUrlChanged(path);
}

void DToolBar::crumbSelected(QString path)
{
    emit fileSignalManager->currentUrlChanged(path);
}

void DToolBar::crumbChanged(const QString &url)
{
    qDebug() << "crumb chagned";
    m_crumbWidget->setCrumb(url);
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
    emit fileSignalManager->currentUrlChanged(text);
}

void DToolBar::searchBarChanged(QString path)
{
    m_searchBar->setText(path);
}

void DToolBar::tabBarChanged(QString path)
{

}



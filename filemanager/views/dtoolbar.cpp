#include "dtoolbar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "dicontextbutton.h"
#include "dsearchbar.h"
#include "dtabbar.h"
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

    QVBoxLayout* mainLayout = new QVBoxLayout;
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

    m_backButton = new DStateButton(":/images/images/dark/appbar.arrow.left.png", this);
    m_forwardButton = new DStateButton(":/images/images/dark/appbar.arrow.right.png", this);
    m_upButton = new DStateButton(":/images/images/dark/appbar.arrow.up.png", this);
    m_searchButton = new DStateButton(":/images/images/dark/appbar.magnify.png", this);
    m_refreshButton = new DStateButton(":/images/images/dark/appbar.refresh.png", this);

    m_stackedWidget = new QStackedWidget;
    m_searchBar = new DSearchBar;
    m_tabBar = new DTabBar;
    m_stackedWidget->addWidget(m_tabBar);
    m_stackedWidget->addWidget(m_searchBar);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_backButton);
    mainLayout->addWidget(m_forwardButton);
    mainLayout->addWidget(m_upButton);
    mainLayout->addWidget(m_stackedWidget);
    mainLayout->addWidget(m_searchButton);
    mainLayout->addWidget(m_refreshButton);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    m_addressToolBar->setLayout(mainLayout);
}

void DToolBar::initContollerToolBar()
{
    m_contollerToolBar = new QFrame(this);
    m_contollerToolBar->setObjectName("ContollerToolBar");
    m_contollerToolBar->setFixedHeight(40);
    m_newFolderButton = new DIconTextButton(QIcon(":/images/images/dark/appbar.folder.png"), tr("New Folder"), this);
    m_deleteFileButton = new DIconTextButton(QIcon(":/images/images/dark/appbar.delete.png"), tr("Delete File"), this);

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
    mainLayout->addWidget(m_newFolderButton);
    mainLayout->addWidget(m_deleteFileButton);
    mainLayout->addStretch();
    mainLayout->addWidget(m_layoutButton);
    mainLayout->addWidget(m_sortButton);
    mainLayout->addWidget(m_hideShowButton);
    mainLayout->addWidget(m_viewSwitchButton);
    mainLayout->setContentsMargins(5, 0, 5, 0);
    m_contollerToolBar->setLayout(mainLayout);
}

void DToolBar::initConnect()
{
    connect(m_layoutButton, SIGNAL(clicked()), this, SIGNAL(requestSwitchLayout()));
    connect(m_backButton, &DStateButton::clicked,
            this, &DToolBar::backButtonClicked);
    connect(m_viewSwitchButton, &DStateButton::clicked,
            this, &DToolBar::switchLayoutMode);
    connect(m_searchButton, SIGNAL(released()), this, SLOT(searchBarSwitched()));
    connect(m_searchBar, SIGNAL(returnPressed()), this, SLOT(searchBarTextEntered()));
    connect(m_tabBar, SIGNAL(tabBarClicked(int)), this, SLOT(tabBarClicked(int)));
    connect(m_upButton, SIGNAL(released()), this, SLOT(upButtonClicked()));
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
    if(m_switchState == 0)
    {
        m_stackedWidget->setCurrentIndex(1);
        m_switchState = 1;
    }
    else
    {
        m_stackedWidget->setCurrentIndex(0);
        m_switchState = 0;
    }
}

void DToolBar::searchBarTextEntered()
{
    m_tabBar->setTabBarPath(m_searchBar->text());
}

void DToolBar::tabBarClicked(int index)
{
    m_searchBar->setText(m_tabBar->shrinkToIndex(index));
}

void DToolBar::upButtonClicked()
{
    int index = m_tabBar->getSelectedIndex();
    if(index >= 1)
        m_searchBar->setText(m_tabBar->moveToIndex(index - 1));
    else
        m_searchBar->setText(m_tabBar->shrinkToIndex(index));
}



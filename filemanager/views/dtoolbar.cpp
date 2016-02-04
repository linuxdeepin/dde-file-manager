#include "dtoolbar.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

#include "dicontextbutton.h"

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
    m_searchButton = new DStateButton(":/images/images/dark/appbar.magnify.png", this);
    m_refreshButton = new DStateButton(":/images/images/dark/appbar.refresh.png", this);

    QHBoxLayout* mainLayout = new QHBoxLayout;
    mainLayout->addWidget(m_backButton);
    mainLayout->addWidget(m_forwardButton);
    mainLayout->addStretch();
    mainLayout->addWidget(m_searchButton);
    mainLayout->addWidget(m_refreshButton);
    mainLayout->setContentsMargins(5, 0, 5, 0);
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
}

DStateButton::ButtonState DToolBar::getLayoutbuttonState()
{
    return m_layoutButton->getButtonState();
}

void DToolBar::setLayoutButtonState(DStateButton::ButtonState state)
{
    m_layoutButton->setButtonState(state);
}



#include "dtitlebar.h"
#include <QIcon>
#include <QHBoxLayout>
#include <QPushButton>
#include <QMouseEvent>

const int DTitleBar::ButtonHeight = 20;

DTitleBar::DTitleBar(QWidget *parent) : QFrame(parent)
{
    initData();
    initUI();
    initConnect();
}

DTitleBar::~DTitleBar()
{

}

void DTitleBar::initData()
{
    m_settingsIcon = new QIcon(":/icons/images/icons/menu_normal.png");
    m_minIcon = new QIcon(":/icons/images/icons/minimise_normal.png");
    m_normalIcon = new QIcon(":/icons/images/icons/maximize_normal.png");
    m_maxIcon= new QIcon(":/icons/images/icons/unmaximize_normal.png");
    m_closeIcon = new QIcon(":/icons/images/icons/close_normal.png");
}

void DTitleBar::initUI()
{
    m_settingButton = new QPushButton(this);
    m_settingButton->setIcon(*m_settingsIcon);
    m_settingButton->setIconSize(QSize(QSize(ButtonHeight, ButtonHeight)));
    m_settingButton->setFocusPolicy(Qt::NoFocus);

    m_minButton = new QPushButton(this);
    m_minButton->setIconSize(QSize(ButtonHeight, ButtonHeight));
    m_minButton->setIcon(*m_minIcon);
    m_minButton->setFocusPolicy(Qt::NoFocus);

    m_maxNormalButton = new QPushButton(this);
    m_maxNormalButton->setIconSize(QSize(ButtonHeight, ButtonHeight));
    m_maxNormalButton->setFocusPolicy(Qt::NoFocus);
    setNormalIcon();

    m_closeButton = new QPushButton(this);
    m_closeButton->setObjectName("close");
    m_closeButton->setIconSize(QSize(ButtonHeight, ButtonHeight));
    m_closeButton->setIcon(*m_closeIcon);
    m_closeButton->setFocusPolicy(Qt::NoFocus);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_settingButton);
    buttonLayout->addWidget(m_minButton);
    buttonLayout->addWidget(m_maxNormalButton);
    buttonLayout->addWidget(m_closeButton);

    QVBoxLayout* controlLayout = new QVBoxLayout;
    controlLayout->addSpacing(6);
    controlLayout->addLayout(buttonLayout);
    controlLayout->addStretch();

    QHBoxLayout* mainlayout = new QHBoxLayout;
    mainlayout->addLayout(controlLayout);
    mainlayout->setContentsMargins(0, 3, 4, 0);
    mainlayout->setSpacing(0);
    setLayout(mainlayout);
}

void DTitleBar::initConnect()
{
    connect(m_minButton, SIGNAL(clicked()), this, SIGNAL(minimuned()));
    connect(m_maxNormalButton, SIGNAL(clicked()), this, SIGNAL(switchMaxNormal()));
    connect(m_closeButton, SIGNAL(clicked()), this, SIGNAL(closed()));
}

void DTitleBar::setNormalIcon()
{
    m_maxNormalButton->setIcon(*m_normalIcon);
}

void DTitleBar::setMaxIcon()
{
    m_maxNormalButton->setIcon(*m_maxIcon);
}

void DTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->pos().x() < (width() - 4 * ButtonHeight - 20)){
        switchMaxNormal();
    }
    QFrame::mouseDoubleClickEvent(event);
}

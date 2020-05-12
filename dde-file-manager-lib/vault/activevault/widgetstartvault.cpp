#include "widgetstartvault.h"

#include <QLabel>
#include <QPushButton>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

WidgetStartVault::WidgetStartVault(QWidget *parent) : QWidget(parent)
{
    QLabel *pLabel1 = new QLabel(tr("File Vault"), this);
    pLabel1->setStyleSheet("font: 16pt 'CESI黑体-GB13000'");
    QLabel *pLabel2 = new QLabel(tr("Create your secure private space"), this);
    QLabel *pLabel3 = new QLabel(tr("Advanced encryption technology"), this);
    QLabel *pLabel4 = new QLabel(tr("Convenient and easy to use"), this);

    DIconButton * icon = new DIconButton(this);
    icon->setFlat(true);
    icon->setIcon(QIcon::fromTheme("dfm_safebox"));
    icon->setIconSize(QSize(64, 64));
    icon->setWindowFlags(Qt::WindowTransparentForInput);
    icon->setFocusPolicy(Qt::NoFocus);
    icon->setMinimumHeight(64);

    m_pStartBtn = new QPushButton(tr("Create"), this);
    m_pStartBtn->setMinimumWidth(450);
    connect(m_pStartBtn, &QPushButton::clicked,
            this, &WidgetStartVault::slotStartBtnClicked);

    // 布局
    QVBoxLayout *play3 = new QVBoxLayout(this);
    play3->setMargin(1);

    QGridLayout *play = new QGridLayout();
    play->setMargin(1);
    play->addWidget(pLabel1, 0, 0, 1, 4, Qt::AlignHCenter);
    play->addWidget(pLabel2, 1, 0, 1, 4, Qt::AlignHCenter);
    play->addWidget(pLabel3, 2, 0, 1, 4, Qt::AlignHCenter);
    play->addWidget(pLabel4, 3, 0, 1, 4, Qt::AlignHCenter);
    play->addWidget(icon, 4, 1, 3, 2, Qt::AlignHCenter);

    play3->addLayout(play);
    play3->addStretch();
    play3->addWidget(m_pStartBtn, 0, Qt::AlignCenter);
}

WidgetStartVault::~WidgetStartVault()
{

}

void WidgetStartVault::slotStartBtnClicked()
{
    emit sigAccepted();
}

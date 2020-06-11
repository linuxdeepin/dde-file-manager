#include "dfmvaultactivestartview.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

DFMVaultActiveStartView::DFMVaultActiveStartView(QWidget *parent)
    : QWidget(parent)
    , m_pStartBtn(nullptr)
{
    QLabel *pLabel1 = new QLabel(tr("File Vault"), this);
    pLabel1->setStyleSheet("font-family: SourceHanSansSC;"
                           "font-size: 14px;"
                           "font-weight: 500;"
                           "font-streth: normal;"
                           "font-style: normal;"
                           "line-height: normal;"
                           "text-align: center;"
                           "color: rgba(0, 0, 0, 0.9);");
    pLabel1->setAlignment(Qt::AlignHCenter);
    QLabel *pLabel2 = new QLabel(tr("Create your secure private space") + '\n' +
                                 tr("Advanced encryption technology") + '\n' +
                                 tr("Convenient and easy to use"), this);
    pLabel2->setStyleSheet("font-family: SourceHanSansSC;"
                           "font-size: 14px;"
                           "font-weight: normal;"
                           "font-stretch: normal;"
                           "font-style: normal;"
                           "line-height: 1.43;"
                           "letter-spaceing: normal;"
                           "text-align: center;"
                           "color: rgba(0, 0, 0, 0.7);");
    pLabel2->setAlignment(Qt::AlignHCenter);

    QLabel *pLabel3 = new QLabel();
    pLabel3->setPixmap(QIcon::fromTheme("dfm_vault_active_start").pixmap(88, 100));
    pLabel3->setAlignment(Qt::AlignHCenter);

    m_pStartBtn = new QPushButton(tr("Create"), this);
    m_pStartBtn->setStyleSheet("width: 452px;"
                               "height: 30px;");
    connect(m_pStartBtn, &QPushButton::clicked,
            this, &DFMVaultActiveStartView::slotStartBtnClicked);

    // 布局
    QVBoxLayout *play = new QVBoxLayout(this);
    play->setMargin(0);
    play->addWidget(pLabel1);
    play->addSpacing(5);
    play->addWidget(pLabel2);
    play->addSpacing(15);
    play->addWidget(pLabel3);
    play->addStretch();
    play->addWidget(m_pStartBtn);
}

void DFMVaultActiveStartView::slotStartBtnClicked()
{
    emit sigAccepted();
}

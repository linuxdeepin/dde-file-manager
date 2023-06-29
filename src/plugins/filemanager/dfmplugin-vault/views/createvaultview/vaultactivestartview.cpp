// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultactivestartview.h"

#include <dfm-framework/event/event.h>

#include <DIconButton>
#include <DLabel>

#include <QVBoxLayout>
#include <QSpacerItem>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultActiveStartView::VaultActiveStartView(QWidget *parent)
    : QWidget(parent)
{
    initUi();
    initConnect();
}

void VaultActiveStartView::slotStartBtnClicked()
{
    emit sigAccepted();
}

void VaultActiveStartView::initUi()
{
    DLabel *pLabel1 = new DLabel(tr("File Vault"), this);
    QFont font = pLabel1->font();
    font.setPixelSize(18);
    pLabel1->setFont(font);
    pLabel1->setAlignment(Qt::AlignHCenter);

    DLabel *pLabel2 = new DLabel(tr("Create your secure private space") + '\n' + tr("Advanced encryption technology") + '\n' + tr("Convenient and easy to use"), this);
    pLabel2->setAlignment(Qt::AlignHCenter);

    DLabel *pLabel3 = new DLabel();
    pLabel3->setPixmap(QIcon::fromTheme("dfm_vault_active_start").pixmap(88, 100));
    pLabel3->setAlignment(Qt::AlignHCenter);

    startBtn = new DPushButton(tr("Create"), this);

    QVBoxLayout *play = new QVBoxLayout(this);
    play->setMargin(0);
    play->addWidget(pLabel1);
    play->addSpacing(5);
    play->addWidget(pLabel2);
    play->addSpacing(15);
    play->addWidget(pLabel3);
    play->addStretch();
    play->addWidget(startBtn);

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(pLabel1), AcName::kAcLabelVaultStartTitle);
    AddATTag(qobject_cast<QWidget *>(pLabel2), AcName::kAcLabelVaultStartContent);
    AddATTag(qobject_cast<QWidget *>(pLabel3), AcName::kAcLabelVaultStartImage);
    AddATTag(qobject_cast<QWidget *>(startBtn), AcName::kAcBtnVaultStartOk);
#endif
}

void VaultActiveStartView::initConnect()
{
    connect(startBtn, &DPushButton::clicked,
            this, &VaultActiveStartView::slotStartBtnClicked);
}

// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultactivestartview.h"

#include <dfm-framework/event/event.h>

#ifdef DTKWIDGET_CLASS_DSizeMode
#    include <DSizeMode>
#endif
#include <DIconButton>
#include <DLabel>
#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QSpacerItem>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_vault;

VaultActiveStartView::VaultActiveStartView(QWidget *parent)
    : VaultBaseView(parent)
{
    initUi();
    initConnect();
}

void VaultActiveStartView::initUi()
{
    titleLabel = new DLabel(tr("File Vault"), this);
    titleLabel->setAlignment(Qt::AlignHCenter);

    DLabel *pLabel2 = new DLabel(tr("Create your secure private space") + '\n' + tr("Advanced encryption technology") + '\n' + tr("Convenient and easy to use"), this);
    pLabel2->setAlignment(Qt::AlignHCenter);

    DLabel *pLabel3 = new DLabel();
    pLabel3->setPixmap(QIcon::fromTheme("dfm_vault_active_start").pixmap(88, 100));
    pLabel3->setAlignment(Qt::AlignHCenter);

    startBtn = new DSuggestButton(tr("Create"), this);
    startBtn->setFixedWidth(200);

    QVBoxLayout *play = new QVBoxLayout(this);
    play->setContentsMargins(0, 0, 0, 0);
    play->addWidget(titleLabel);
    play->addSpacing(5);
    play->addWidget(pLabel3);
    play->addSpacing(15);
    play->addWidget(pLabel2);
    play->addStretch();
    play->addWidget(startBtn, 0, Qt::AlignCenter);

    initUiForSizeMode();

#ifdef ENABLE_TESTING
    AddATTag(qobject_cast<QWidget *>(titleLabel), AcName::kAcLabelVaultStartTitle);
    AddATTag(qobject_cast<QWidget *>(pLabel2), AcName::kAcLabelVaultStartContent);
    AddATTag(qobject_cast<QWidget *>(pLabel3), AcName::kAcLabelVaultStartImage);
    AddATTag(qobject_cast<QWidget *>(startBtn), AcName::kAcBtnVaultStartOk);
#endif
}

void VaultActiveStartView::initUiForSizeMode()
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    DFontSizeManager::instance()->bind(titleLabel, DSizeModeHelper::element(DFontSizeManager::SizeType::T7, DFontSizeManager::SizeType::T5), QFont::Medium);
#else
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::SizeType::T5, QFont::Medium);
#endif
}

void VaultActiveStartView::initConnect()
{
    connect(startBtn, &DPushButton::clicked,
            this, &VaultActiveStartView::accepted);
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        initUiForSizeMode();
    });
#endif
}

// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dpcresultwidget.h"

#include <DPushButton>
#include <DLabel>
#include <DFontSizeManager>

#include <QVBoxLayout>

#include <dfm-base/utils/iconutils.h>

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_titlebar;

DPCResultWidget::DPCResultWidget(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnect();
}

void DPCResultWidget::setResult(bool success, const QString &msg)
{
    msgLabel->setText(msg);
    msgLabel->setVisible(!msg.isEmpty());

    if (success) {
        titleLabel->setText(tr("Disk password changed"));
        resultIcon->setPixmap(IconUtils::hiDpiPixmap(QIcon::fromTheme("dfm_success"), QSize(128, 128), this));
    } else {
        titleLabel->setText(tr("Failed to change the disk password"));
        resultIcon->setPixmap(IconUtils::hiDpiPixmap(QIcon::fromTheme("dfm_fail"), QSize(128, 128), this));
    }
}

void DPCResultWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    titleLabel = new DLabel(this);
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    titleLabel->setWordWrap(true);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager *fontManager = DFontSizeManager::instance();
    fontManager->bind(titleLabel, DFontSizeManager::T5, QFont::Medium);

    msgLabel = new DLabel(this);
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignHCenter);

    resultIcon = new DLabel(this);
    resultIcon->setAlignment(Qt::AlignHCenter);

    closeBtn = new DPushButton(tr("Close", "button"), this);

    mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(resultIcon, 0, Qt::AlignHCenter);
    mainLayout->addWidget(msgLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(closeBtn);
}

void DPCResultWidget::initConnect()
{
    connect(closeBtn, &DPushButton::clicked, this, &DPCResultWidget::sigCloseDialog);
}

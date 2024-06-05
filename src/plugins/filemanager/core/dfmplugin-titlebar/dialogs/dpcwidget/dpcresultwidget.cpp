// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dpcresultwidget.h"

#include <DPushButton>
#include <DLabel>
#include <DFontSizeManager>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
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
        resultIcon->setPixmap(QIcon::fromTheme("dfm_success").pixmap(128, 128));
    } else {
        titleLabel->setText(tr("Failed to change the disk password"));
        resultIcon->setPixmap(QIcon::fromTheme("dfm_fail").pixmap(128, 128));
    }
}

void DPCResultWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mainLayout->setMargin(0);
#else
    mainLayout->setContentsMargins(0, 0, 0, 0);
#endif
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

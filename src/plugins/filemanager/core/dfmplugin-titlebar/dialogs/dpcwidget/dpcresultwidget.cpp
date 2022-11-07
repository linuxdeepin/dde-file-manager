/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
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
    mainLayout->setMargin(0);
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

    closeBtn = new DPushButton(tr("Close"), this);

    mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(resultIcon, 0, Qt::AlignHCenter);
    mainLayout->addWidget(msgLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(closeBtn);
}

void DPCResultWidget::initConnect()
{
    connect(closeBtn, &DPushButton::clicked, this, &DPCResultWidget::sigCloseDialog);
}

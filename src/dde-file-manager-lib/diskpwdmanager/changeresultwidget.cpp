// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "changeresultwidget.h"

#include <DPushButton>
#include <DLabel>
#include <DFontSizeManager>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE

ChangeResultWidget::ChangeResultWidget(QWidget *parent)
    : DWidget(parent)
{
    initUI();
    initConnect();
}

void ChangeResultWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    m_titleLabel = new DLabel(this);
    m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager *fontManager = DFontSizeManager::instance();
    fontManager->bind(m_titleLabel, DFontSizeManager::T5, QFont::Medium);

    m_msgLabel = new DLabel(this);
    m_msgLabel->setWordWrap(true);
    m_msgLabel->setAlignment(Qt::AlignHCenter);

    m_resultIcon = new DLabel(this);
    m_resultIcon->setAlignment(Qt::AlignHCenter);

    m_closeBtn = new DPushButton(tr("Close"), this);

    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_resultIcon, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_msgLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_closeBtn);
}

void ChangeResultWidget::initConnect()
{
    connect(m_closeBtn, &DPushButton::clicked, this, &ChangeResultWidget::sigClosed);
}

void ChangeResultWidget::setResult(bool success, const QString &msg)
{
    m_msgLabel->setText(msg);
    m_msgLabel->setVisible(!msg.isEmpty());

    if (success) {
        m_titleLabel->setText(tr("Disk password changed"));
        m_resultIcon->setPixmap(QIcon::fromTheme("dfm_success").pixmap(128, 128));
    } else {
        m_titleLabel->setText(tr("Failed to change the disk password"));
        m_resultIcon->setPixmap(QIcon::fromTheme("dfm_fail").pixmap(128, 128));
    }
}

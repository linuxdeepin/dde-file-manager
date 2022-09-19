// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pwdchangeprogressview.h"
#include "pwdconfirmwidget.h"

#include <DWaterProgress>
#include <DLabel>
#include <DPushButton>
#include <DFontSizeManager>
#include <DPaletteHelper>

#include <QFrame>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QTimer>

DWIDGET_USE_NAMESPACE

ChangeResultWidget::ChangeResultWidget(QWidget *parent)
    : DWidget(parent)
{
    initUI();
}

void ChangeResultWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);

    m_titleLabel = new DLabel(this);
    m_msgLabel = new DLabel(this);
    m_resultIcon = new DLabel(this);
    m_closeBtn = new DPushButton(tr("Close"), this);

    mainLayout->addWidget(m_titleLabel);
    mainLayout->addSpacerItem(new QSpacerItem(1, 25));
    mainLayout->addWidget(m_resultIcon);
    mainLayout->addSpacerItem(new QSpacerItem(1, 20));
    mainLayout->addWidget(m_msgLabel);
    mainLayout->addSpacerItem(new QSpacerItem(1, 10));
    mainLayout->addWidget(m_closeBtn);
}

SwitchPageWidget::SwitchPageWidget(QWidget *parent)
    : DWidget(parent)
{
    initUI();
}

void SwitchPageWidget::switchPage(SwitchPageWidget::PageType type)
{
    switch (type) {
    case ConfirmPage:
        switchToConfirmPage();
        break;
    case ProgressPage:
        switchToProgressPage();
        break;
    case SuccessedPage:
        switchToSuccessedPage();
        break;
    case FailedPage:
        switchToFailedPage();
        break;
    }
}

void SwitchPageWidget::initUI()
{
    // progress
    m_progress = new DWaterProgress(this);
    m_progress->setFixedSize(98, 98);

    // result icon label
    m_resultIcon = new DLabel(this);
    m_resultIcon->setAlignment(Qt::AlignHCenter);

    m_confirmWidget = new PwdConfirmWidget(this);

    // message
    m_msgLabel = new DLabel(this);
    m_msgLabel->setWordWrap(true);

    m_switchWidget = new QStackedWidget(this);
    m_switchWidget->addWidget(m_confirmWidget);
    m_switchWidget->addWidget(m_progress);
    m_switchWidget->addWidget(m_resultIcon);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(m_switchWidget);
    layout->addWidget(m_msgLabel, 0, Qt::AlignCenter);
    layout->setContentsMargins(0, 30, 0, 0);

    setLayout(layout);
}

void SwitchPageWidget::setMessage(const QString &msg)
{
    m_msgLabel->setText(msg);
}

bool SwitchPageWidget::checkPassword()
{
    QTimer::singleShot(2000, [this] {
        emit this->changePasswordFinished(true, "");
    });

    return m_confirmWidget->confirm();
}

void SwitchPageWidget::switchToConfirmPage()
{
    m_msgLabel->clear();
    m_switchWidget->setCurrentIndex(ConfirmPage);
}

void SwitchPageWidget::switchToProgressPage()
{
    Q_ASSERT(m_progress);

    //    layout()->setAlignment(Qt::AlignHCenter);
    //    m_switchWidget->layout()->setAlignment(Qt::AlignCenter);
    m_switchWidget->setCurrentIndex(ProgressPage);
    setMessage(tr("The window cannot be closed during the process"));

    m_progress->setValue(1);
}

void SwitchPageWidget::switchToSuccessedPage()
{
    Q_ASSERT(m_resultIcon);

    m_msgLabel->clear();
    m_switchWidget->setCurrentIndex(SuccessedPage);
    m_resultIcon->setPixmap(QIcon::fromTheme("dfm_vault_active_finish").pixmap(98, 98));
}

void SwitchPageWidget::switchToFailedPage()
{
    Q_ASSERT(m_resultIcon);
}

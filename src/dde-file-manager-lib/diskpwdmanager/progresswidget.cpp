// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "progresswidget.h"
#include "globaldefine.h"
#include "dbusinterface/disk_interface.h"

#include <DLabel>
#include <DWaterProgress>
#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QTimer>

DWIDGET_USE_NAMESPACE

ProgressWidget::ProgressWidget(QWidget *parent)
    : DWidget(parent)
{
    m_diskInterface = new DiskInterface("com.deepin.filemanager.daemon",
                                        "/com/deepin/filemanager/daemon/DiskManager",
                                        QDBusConnection::systemBus(),
                                        this);

    initUI();
    initConnect();
}

void ProgressWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    m_progressTimer = new QTimer(this);
    m_progressTimer->setInterval(1000);

    m_titleLabel = new DLabel(tr("Changing disk password..."), this);
    m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_titleLabel->setWordWrap(true);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager *fontManager = DFontSizeManager::instance();
    fontManager->bind(m_titleLabel, DFontSizeManager::T5, QFont::Medium);

    m_msgLabel = new DLabel(tr("The window cannot be closed during the process"), this);
    m_msgLabel->setWordWrap(true);
    m_msgLabel->setAlignment(Qt::AlignHCenter);
    m_msgLabel->setMinimumHeight(50);

    m_progress = new DWaterProgress(this);
    m_progress->setFixedSize(98, 98);
    m_progress->setValue(1);

    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(25);
    mainLayout->addWidget(m_progress, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(m_msgLabel, 0, Qt::AlignHCenter);
}

void ProgressWidget::initConnect()
{
    connect(m_progressTimer, &QTimer::timeout, this, &ProgressWidget::changeProgressValue);
    connect(m_diskInterface, &DiskInterface::finished, this, &ProgressWidget::onFinished);
}

void ProgressWidget::changeProgressValue()
{
    int value = m_progress->value();
    if (value + 5 < 95) {
        m_progress->setValue(value + 5);
    } else {
        m_progressTimer->stop();
    }
}

void ProgressWidget::onFinished(int result)
{
    switch (result) {
    case Success:
        if (m_progressTimer->isActive())
            m_progressTimer->stop();

        m_progress->setValue(100);
        QTimer::singleShot(500, [this] {
            emit sigChangeFinished(true, "");
        });

        break;
    case PasswordInconsistent:
        emit sigChangeFinished(false, tr("Passwords of disks are different"));
        break;
    case AccessDiskFailed:
        emit sigChangeFinished(false, tr("Unable to get the encrypted disk list"));
        break;
    case InitFailed:
        emit sigChangeFinished(false, tr("Initialization failed"));
        break;
    default:
        emit sigChangeFinished(false, tr("Unknown error,code: %1").arg(result));
        break;
    }
}

void ProgressWidget::start()
{
    m_progressTimer->start();
    m_progress->start();
}

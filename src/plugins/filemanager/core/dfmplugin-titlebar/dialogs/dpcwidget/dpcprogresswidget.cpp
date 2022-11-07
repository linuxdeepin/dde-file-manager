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
#include "dpcprogresswidget.h"

#include <DLabel>
#include <DWaterProgress>
#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QTimer>
#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusPendingCall>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;

namespace DaemonServiceIFace {
static constexpr char kInterfaceService[] { "com.deepin.filemanager.daemon" };
static constexpr char kInterfacePath[] { "/com/deepin/filemanager/daemon/AccessControlManager" };
static constexpr char kInterfaceInterface[] { "com.deepin.filemanager.daemon.AccessControlManager" };

static constexpr char kSigPwdChanged[] { "DiskPasswordChanged" };
}   // namespace DBusInterfaceInfo

DPCProgressWidget::DPCProgressWidget(QWidget *parent)
    : DWidget(parent)
{
    accessControlInter.reset(new QDBusInterface(DaemonServiceIFace::kInterfaceService,
                                                DaemonServiceIFace::kInterfacePath,
                                                DaemonServiceIFace::kInterfaceInterface,
                                                QDBusConnection::systemBus(),
                                                this));
    initUI();
    initConnect();
}

void DPCProgressWidget::initUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setMargin(0);
    setLayout(mainLayout);

    progressTimer = new QTimer(this);
    progressTimer->setInterval(1000);

    titleLabel = new DLabel(tr("Changing disk password..."), this);
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    titleLabel->setWordWrap(true);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager *fontManager = DFontSizeManager::instance();
    fontManager->bind(titleLabel, DFontSizeManager::T5, QFont::Medium);

    msgLabel = new DLabel(tr("The window cannot be closed during the process"), this);
    msgLabel->setWordWrap(true);
    msgLabel->setAlignment(Qt::AlignHCenter);
    msgLabel->setMinimumHeight(50);

    changeProgress = new DWaterProgress(this);
    changeProgress->setFixedSize(98, 98);
    changeProgress->setValue(1);

    mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(25);
    mainLayout->addWidget(changeProgress, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(msgLabel, 0, Qt::AlignHCenter);
}

void DPCProgressWidget::initConnect()
{
    Q_ASSERT(accessControlInter);

    connect(progressTimer, &QTimer::timeout, this, &DPCProgressWidget::changeProgressValue);

    accessControlInter->connection().connect(accessControlInter->service(),
                                             accessControlInter->path(),
                                             accessControlInter->interface(),
                                             DaemonServiceIFace::kSigPwdChanged,
                                             this,
                                             SLOT(onDiskPwdChanged(int)));
}

void DPCProgressWidget::start()
{
    progressTimer->start();
    changeProgress->start();
}

void DPCProgressWidget::changeProgressValue()
{
    int value = changeProgress->value();
    if (value + 5 < 95) {
        changeProgress->setValue(value + 5);
    } else {
        progressTimer->stop();
    }
}

void DPCProgressWidget::onDiskPwdChanged(int result)
{
    switch (result) {
    case kNoError:
        if (progressTimer->isActive())
            progressTimer->stop();

        changeProgress->setValue(100);
        QTimer::singleShot(500, [this] {
            emit sigCompleted(true, "");
        });

        break;
    case kPasswordInconsistent:
        emit sigCompleted(false, tr("Passwords of disks are different"));
        break;
    case kAccessDiskFailed:
        emit sigCompleted(false, tr("Unable to get the encrypted disk list"));
        break;
    case kDeviceLoadFailed:
    case kInitFailed:
        emit sigCompleted(false, tr("Initialization failed"));
        break;
    default:
        emit sigCompleted(false, "");
        break;
    }
}

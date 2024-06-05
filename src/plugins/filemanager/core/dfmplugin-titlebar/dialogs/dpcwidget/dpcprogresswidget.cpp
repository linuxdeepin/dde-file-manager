// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    mainLayout->setMargin(0);
#else
    mainLayout->setContentsMargins(0, 0, 0, 0);
#endif
    setLayout(mainLayout);

    progressTimer = new QTimer(this);
    progressTimer->setInterval(1000);

    titleLabel = new DLabel(tr("Changing disk password..."), this);
    titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
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

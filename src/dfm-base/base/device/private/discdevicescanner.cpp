// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "discdevicescanner.h"

#include <dfm-base/utils/sysinfoutils.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <QDebug>
#include <QCoreApplication>
#include <QRunnable>

#include <mutex>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static constexpr int kTimerInterval { 3000 };
static const char kDesktopAppName[] { "dde-desktop" };
static constexpr char kBlockDeviceIdPrefix[] { "/org/freedesktop/UDisks2/block_devices/" };

using namespace dfmbase;
using namespace DiscDevice;

Scanner::Scanner(const QString &dev)
    : device(dev)
{
}

void Scanner::run()
{
    QByteArray devBytes { device.toLatin1() };
    int fd { ::open(devBytes.data(), O_RDWR | O_NONBLOCK) };
    if (fd != -1)
        ::close(fd);
}

DiscDeviceScanner::DiscDeviceScanner(QObject *parent)
    : QObject(parent)
{
}

bool DiscDeviceScanner::startScan()
{
    if (discScanTimer->isActive()) {
        qCWarning(logDFMBase) << "Timer is active, cannot start again";
        return false;
    }

    if (discDevIdGroup.isEmpty()) {
        qCWarning(logDFMBase) << "Current disc dev is empty, cannot start timer";
        return false;
    }

    // only try to open read/write if not root, since it doesn't seem
    // to make a difference for root and can have negative side-effects
    if (SysInfoUtils::isRootUser()) {
        qCWarning(logDFMBase) << "Current user is root, cannot start timer";
        return false;
    }

    // cause a lot of logs in server version
    if (SysInfoUtils::isServerSys()) {
        qCWarning(logDFMBase) << "Current OS version is server, cannot start timer";
        return false;
    }

    // Usually the Community Edition does not require an optical drive to be inserted
    if (SysInfoUtils::isDesktopSys() && !SysInfoUtils::isProfessional()) {
        qCWarning(logDFMBase) << "Current OS version is deepin destkop, does not require start timer";
        return false;
    }

    qCInfo(logDFMBase) << "Start scan disc";
    discScanTimer->start(kTimerInterval);

    return true;
}

void DiscDeviceScanner::stopScan()
{
    if (!discScanTimer->isActive()) {
        qCWarning(logDFMBase) << "Timer is active, canot stop";
        return;
    }
    qCInfo(logDFMBase) << "Stop scan disc";
    discScanTimer->stop();
}

void DiscDeviceScanner::updateScanState()
{
    if (discDevIdGroup.isEmpty())
        stopScan();
    else
        startScan();
}

/*!
 * \brief This is a workaround
 * On some specific hardware devices,
 * `udev` cannot directly trigger the signal that the device has been removed
 */
void DiscDeviceScanner::scanOpticalDisc()
{
    using namespace GlobalServerDefines;
    std::for_each(discDevIdGroup.cbegin(), discDevIdGroup.cend(), [this](const QString &id) {
        const auto &info { DevProxyMng->queryBlockInfo(id) };
        auto &&dev { info.value(DeviceProperty::kDevice).toString() };
        bool optical { info.value(DeviceProperty::kOptical).toBool() };
        if (!optical)
            return;
        if (!dev.startsWith("/dev/sr"))
            return;
        // QThreadPool takes ownership and deletes 'Scanner' automatically
        Scanner *scanner = new Scanner(dev);
        threadPool->start(scanner);
    });
}

void DiscDeviceScanner::onDevicePropertyChangedQVar(const QString &id, const QString &propertyName, const QVariant &var)
{
    onDevicePropertyChangedQDBusVar(id, propertyName, QDBusVariant(var));
}

/**
 * @brief monitor disc eject or insert
 * @param id
 * @param propertyName
 * @param var
 */
void DiscDeviceScanner::onDevicePropertyChangedQDBusVar(const QString &id, const QString &propertyName, const QDBusVariant &var)
{
    using namespace GlobalServerDefines;
    if (!id.startsWith(kBlockDeviceIdPrefix))
        return;
    if (propertyName == DeviceProperty::kOptical) {
        bool ret { var.variant().toBool() };
        if (ret && !discDevIdGroup.contains(id))
            discDevIdGroup.push_back(id);
        else
            discDevIdGroup.removeOne(id);
        updateScanState();
    }
}

void DiscDeviceScanner::onDiscWorkingStateChanged(const QString &id, const QString &dev, bool working)
{
    Q_UNUSED(dev)

    if (working)
        discDevIdGroup.removeOne(id);
    else if (!discDevIdGroup.contains(id))
        discDevIdGroup.push_back(id);

    updateScanState();
}

void DiscDeviceScanner::initialize()
{
    static std::once_flag flag;
    std::call_once(flag, [this]() {
        // ony runing desktop or filemanger, not allowed to run simultaneously
        if (qApp->applicationName() != kDesktopAppName && DevProxyMng->isDBusRuning()) {
            qCInfo(logDFMBase) << "Current app is filemanger and desktop running, don't init";
            return;
        }

        const auto &discIdGroup { DevProxyMng->getAllBlockIds(GlobalServerDefines::DeviceQueryOption::kOptical) };
        discDevIdGroup = discIdGroup;

        connect(discScanTimer.data(), &QTimer::timeout, this, &DiscDeviceScanner::scanOpticalDisc);
        connect(DevProxyMng, &DeviceProxyManager::blockDevPropertyChanged, this, &DiscDeviceScanner::onDevicePropertyChangedQVar);
        connect(DevMngIns, &DeviceManager::opticalDiscWorkStateChanged, this, &DiscDeviceScanner::onDiscWorkingStateChanged);

        threadPool->setMaxThreadCount(4);
        this->startScan();
    });
}

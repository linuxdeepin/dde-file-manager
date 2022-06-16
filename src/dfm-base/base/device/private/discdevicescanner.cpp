/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "discdevicescanner.h"

#include "dfm-base/utils/sysinfoutils.h"
#include "dfm-base/base/device/devicemanager.h"
#include "dfm-base/base/device/deviceproxymanager.h"

#include <QDebug>
#include <QCoreApplication>

#include <mutex>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

static constexpr int kTimerInterval { 3000 };
static const char kDesktopAppName[] { "dde-desktop" };
static constexpr char kBlockDeviceIdPrefix[] { "/org/freedesktop/UDisks2/block_devices/" };

DFMBASE_USE_NAMESPACE

DiscDeviceScanner::DiscDeviceScanner(QObject *parent)
    : QObject(parent)
{
}

bool DiscDeviceScanner::startScan()
{
    if (discScanTimer->isActive()) {
        qWarning() << "Timer is active, cannot start again";
        return false;
    }

    if (discDevIdGroup.isEmpty()) {
        qWarning() << "Current disc dev is empty, cannot start timer";
        return false;
    }

    // only try to open read/write if not root, since it doesn't seem
    // to make a difference for root and can have negative side-effects
    if (SysInfoUtils::isRootUser()) {
        qWarning() << "Current user is root, cannot start timer";
        return false;
    }

    // cause a lot of logs in server version
    if (SysInfoUtils::isServerSys()) {
        qWarning() << "Current OS version is server, cannot start timer";
        return false;
    }

    qInfo() << "Start scan disc";
    discScanTimer->start(kTimerInterval);

    return true;
}

void DiscDeviceScanner::stopScan()
{
    if (!discScanTimer->isActive()) {
        qWarning() << "Timer is active, canot stop";
        return;
    }
    qInfo() << "Stop scan disc";
    discScanTimer->stop();
    return;
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
    std::for_each(discDevIdGroup.cbegin(), discDevIdGroup.cend(), [](const QString &id) {
        const auto &info { DevProxyMng->queryBlockInfo(id) };
        auto &&dev { info.value(DeviceProperty::kDevice).toString() };
        bool optical { info.value(DeviceProperty::kOptical).toBool() };
        if (!optical)
            return;
        if (!dev.startsWith("/dev/sr"))
            return;
        QByteArray devBytes { dev.toLatin1() };
        int fd { ::open(devBytes.data(), O_RDWR | O_NONBLOCK) };
        if (fd != -1)
            ::close(fd);
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

void DiscDeviceScanner::onDiscWoringStateChanged(const QString &id, const QString &dev, bool working)
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
            qInfo() << "Current app is filemanger and desktop running, don't init";
            return;
        }

        const auto &discIdGroup { DevProxyMng->getAllBlockIds(GlobalServerDefines::DeviceQueryOption::kOptical) };
        discDevIdGroup = discIdGroup;

        connect(discScanTimer.data(), &QTimer::timeout, this, &DiscDeviceScanner::scanOpticalDisc);
        connect(DevProxyMng, &DeviceProxyManager::blockDevPropertyChanged, this, &DiscDeviceScanner::onDevicePropertyChangedQVar);
        connect(DevMngIns, &DeviceManager::opticalDiscWorkStateChanged, this, &DiscDeviceScanner::onDiscWoringStateChanged);

        this->startScan();
    });
}

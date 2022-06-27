/*
 * Copyright (C) 2021 Uniontech Software Technology Co., Ltd.
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
#include "defendercontroller.h"

#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QDBusPendingCall>
#include <QApplication>

#include <algorithm>

// Need to consider stopping multiple directory scans at the same time,
// and extend the timeout period appropriately
static const int kMaxDBusTimeout = 1000;

static const char *const kDefenderServiceName = "com.deepin.defender.daemonservice";
static const char *const kDefenderServicePath = "/com/deepin/defender/daemonservice";
static const char *const kDefenderInterfaceName = "com.deepin.defender.daemonservice";

using namespace dfmbase;

DefenderController &DefenderController::instance()
{
    static DefenderController helper;
    return helper;
}

/*!
 * \brief Check if the current scan path contains the url
 * or a subdirectory of the url
 * \param url
 * \return true if url is scanning
 */
bool DefenderController::isScanning(const QUrl &url)
{
    // make sure monitor DBus
    start();

    QList<QUrl> &&paths = getScanningPaths(url);
    return !paths.empty();
}

bool DefenderController::isScanning(const QList<QUrl> &urls)
{
    bool (DefenderController::*f)(const QUrl &) = &DefenderController::isScanning;
    if (std::any_of(urls.constBegin(), urls.constEnd(), std::bind(f, this, std::placeholders::_1)))
        return true;
    return false;
}

bool DefenderController::stopScanning(const QUrl &url)
{
    QList<QUrl> urls;
    urls << url;
    return stopScanning(urls);
}

bool DefenderController::stopScanning(const QList<QUrl> &urls)
{
    qInfo() << "stopScanning:" << urls;
    qInfo() << "current scanning:" << scanningPaths;

    // make sure monitor DBus
    start();

    QList<QUrl> paths;
    foreach (const QUrl &url, urls)
        paths << getScanningPaths(url);

    if (paths.empty())
        return true;

    foreach (const QUrl &path, paths) {
        qInfo() << "send RequestStopUsbScannig:" << path;
        interface->asyncCall("RequestStopUsbScannig", path.toLocalFile());
    }

    // Wait for the scan directory change signal until it times out
    QTime t;
    t.start();
    while (t.elapsed() < kMaxDBusTimeout) {
        qApp->processEvents();
        if (!isScanning(urls))
            return true;
        QThread::msleep(10);
    }
    return false;
}

QList<QUrl> DefenderController::getScanningPaths(const QUrl &url)
{
    QList<QUrl> list;
    for (const QUrl &p : scanningPaths) {
        if (url.isParentOf(p) || url == p)
            list << p;
    }
    return list;
}

void DefenderController::start()
{
    std::call_once(DefenderController::onceFlag(), [this]() {
        qInfo() << "create dbus interface:" << kDefenderServiceName;
        interface.reset(new QDBusInterface(kDefenderServiceName,
                                           kDefenderServicePath,
                                           kDefenderInterfaceName,
                                           QDBusConnection::sessionBus()));

        qInfo() << "create dbus interface done";

        QDBusConnection::sessionBus().connect(
                kDefenderServiceName,
                kDefenderServicePath,
                kDefenderInterfaceName,
                "ScanningUsbPathsChanged",
                this,
                SLOT(scanningUsbPathsChanged(QStringList)));

        qInfo() << "start get usb scanning path";
        QStringList list = interface->property("ScanningUsbPaths").toStringList();
        foreach (const QString &p, list)
            scanningPaths << QUrl::fromLocalFile(p);

        qInfo() << "get usb scanning path done:" << scanningPaths;
    });
}

void DefenderController::scanningUsbPathsChanged(const QStringList &list)
{
    qInfo() << "reveive signal: scanningUsbPathsChanged, " << list;
    scanningPaths.clear();
    foreach (const QString &p, list)
        scanningPaths << QUrl::fromLocalFile(p);
}

DefenderController::DefenderController(QObject *parent)
    : QObject(parent)
{
}

DefenderController::~DefenderController()
{
}

std::once_flag &DefenderController::onceFlag()
{
    static std::once_flag flag;
    return flag;
}

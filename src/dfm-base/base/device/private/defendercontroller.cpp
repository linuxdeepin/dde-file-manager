// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defendercontroller.h"

#include <QDebug>
#include <QThread>
#include <QDateTime>
#include <QDBusPendingCall>
#include <QApplication>
#include <QElapsedTimer>

#include <algorithm>

// Need to consider stopping multiple directory scans at the same time,
// and extend the timeout period appropriately
static const int kMaxDBusTimeout = 25 * 1000;

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
    qCInfo(logDFMBase) << "stopScanning:" << urls;
    qCInfo(logDFMBase) << "current scanning:" << scanningPaths;

    // make sure monitor DBus
    start();

    QList<QUrl> paths;
    foreach (const QUrl &url, urls)
        paths << getScanningPaths(url);

    if (paths.empty())
        return true;

    foreach (const QUrl &path, paths) {
        qCInfo(logDFMBase) << "send RequestStopUsbScannig:" << path;
        interface->asyncCall("RequestStopUsbScannig", path.toLocalFile());
    }

    // Wait for the scan directory change signal until it times out
    QElapsedTimer t;
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
    static std::once_flag flg;
    std::call_once(flg, [this]() {
        qCInfo(logDFMBase) << "create dbus interface:" << kDefenderServiceName;
        interface.reset(new QDBusInterface(kDefenderServiceName,
                                           kDefenderServicePath,
                                           kDefenderInterfaceName,
                                           QDBusConnection::sessionBus()));

        qCInfo(logDFMBase) << "create dbus interface done";

        QDBusConnection::sessionBus().connect(
                kDefenderServiceName,
                kDefenderServicePath,
                kDefenderInterfaceName,
                "ScanningUsbPathsChanged",
                this,
                SLOT(scanningUsbPathsChanged(QStringList)));

        qCInfo(logDFMBase) << "start get usb scanning path";
        QStringList list = interface->property("ScanningUsbPaths").toStringList();
        foreach (const QString &p, list)
            scanningPaths << QUrl::fromLocalFile(p);

        qCInfo(logDFMBase) << "get usb scanning path done:" << scanningPaths;
    });
}

void DefenderController::scanningUsbPathsChanged(const QStringList &list)
{
    qCInfo(logDFMBase) << "reveive signal: scanningUsbPathsChanged, " << list;
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

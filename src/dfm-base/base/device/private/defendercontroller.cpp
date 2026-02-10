// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "defendercontroller.h"

#include <dfm-base/dfm_base_global.h>

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
    if (!url.isValid()) {
        qCWarning(logDFMBase) << "DefenderController::isScanning: Invalid URL provided:" << url;
        return false;
    }

    qCDebug(logDFMBase) << "DefenderController::isScanning: Checking scan status for URL:" << url.toString();
    
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
    if (!url.isValid()) {
        qCWarning(logDFMBase) << "DefenderController::stopScanning: Invalid URL provided:" << url;
        return false;
    }

    qCInfo(logDFMBase) << "DefenderController::stopScanning: Stopping scan for single URL:" << url.toString();
    
    QList<QUrl> urls;
    urls << url;
    return stopScanning(urls);
}

bool DefenderController::stopScanning(const QList<QUrl> &urls)
{
    if (urls.isEmpty()) {
        qCWarning(logDFMBase) << "DefenderController::stopScanning: Empty URL list provided";
        return false;
    }

    qCInfo(logDFMBase) << "DefenderController::stopScanning: Stopping scan for URLs:" << urls;
    qCDebug(logDFMBase) << "DefenderController::stopScanning: Current scanning paths:" << scanningPaths;

    // make sure monitor DBus
    start();

    QList<QUrl> paths;
    foreach (const QUrl &url, urls) {
        if (!url.isValid()) {
            qCWarning(logDFMBase) << "DefenderController::stopScanning: Skipping invalid URL:" << url;
            continue;
        }
        paths << getScanningPaths(url);
    }

    if (paths.empty()) {
        qCInfo(logDFMBase) << "DefenderController::stopScanning: No matching scanning paths found, operation completed";
        return true;
    }

    qCInfo(logDFMBase) << "DefenderController::stopScanning: Found" << paths.size() << "paths to stop scanning";

    foreach (const QUrl &path, paths) {
        qCInfo(logDFMBase) << "DefenderController::stopScanning: Sending RequestStopUsbScannig for path:" << path.toString();
        
        if (!interface) {
            qCCritical(logDFMBase) << "DefenderController::stopScanning: DBus interface is null, cannot send stop request";
            return false;
        }
        
        interface->asyncCall("RequestStopUsbScannig", path.toLocalFile());
    }

    // Wait for the scan directory change signal until it times out
    qCDebug(logDFMBase) << "DefenderController::stopScanning: Waiting for scan stop confirmation, timeout:" << kMaxDBusTimeout << "ms";
    
    QElapsedTimer t;
    t.start();
    while (t.elapsed() < kMaxDBusTimeout) {
        qApp->processEvents();
        if (!isScanning(urls)) {
            qCInfo(logDFMBase) << "DefenderController::stopScanning: Scan stopped successfully after" << t.elapsed() << "ms";
            return true;
        }
        QThread::msleep(10);
    }
    
    qCWarning(logDFMBase) << "DefenderController::stopScanning: Timeout waiting for scan stop confirmation after" 
                          << t.elapsed() << "ms, operation may have failed";
    return false;
}

QList<QUrl> DefenderController::getScanningPaths(const QUrl &url)
{
    if (!url.isValid()) {
        qCWarning(logDFMBase) << "DefenderController::getScanningPaths: Invalid URL provided:" << url;
        return QList<QUrl>();
    }

    QList<QUrl> list;
    for (const QUrl &p : scanningPaths) {
        if (url.isParentOf(p) || url == p) {
            list << p;
        }
    }
    
    qCDebug(logDFMBase) << "DefenderController::getScanningPaths: Found" << list.size() 
                        << "matching paths for URL:" << url.toString();
    
    return list;
}

void DefenderController::start()
{
    static std::once_flag flg;
    std::call_once(flg, [this]() {
        qCInfo(logDFMBase) << "DefenderController::start: Initializing defender controller";
        qCDebug(logDFMBase) << "DefenderController::start: Creating DBus interface for service:" << kDefenderServiceName;
        
        interface.reset(new QDBusInterface(kDefenderServiceName,
                                           kDefenderServicePath,
                                           kDefenderInterfaceName,
                                           QDBusConnection::sessionBus()));

        if (!interface) {
            qCCritical(logDFMBase) << "DefenderController::start: Failed to create DBus interface for defender service";
            return;
        }

        if (!interface->isValid()) {
            qCCritical(logDFMBase) << "DefenderController::start: DBus interface is invalid:" 
                                   << interface->lastError().message();
            return;
        }

        qCInfo(logDFMBase) << "DefenderController::start: DBus interface created successfully";

        bool connected = QDBusConnection::sessionBus().connect(
                kDefenderServiceName,
                kDefenderServicePath,
                kDefenderInterfaceName,
                "ScanningUsbPathsChanged",
                this,
                SLOT(scanningUsbPathsChanged(QStringList)));

        if (!connected) {
            qCWarning(logDFMBase) << "DefenderController::start: Failed to connect to ScanningUsbPathsChanged signal";
        } else {
            qCDebug(logDFMBase) << "DefenderController::start: Successfully connected to ScanningUsbPathsChanged signal";
        }

        qCDebug(logDFMBase) << "DefenderController::start: Retrieving initial USB scanning paths";
        
        QVariant property = interface->property("ScanningUsbPaths");
        if (!property.isValid()) {
            qCWarning(logDFMBase) << "DefenderController::start: Failed to get ScanningUsbPaths property";
            return;
        }
        
        QStringList list = property.toStringList();
        foreach (const QString &p, list) {
            if (!p.isEmpty()) {
                scanningPaths << QUrl::fromLocalFile(p);
            }
        }

        qCInfo(logDFMBase) << "DefenderController::start: Initialization completed, found" 
                           << scanningPaths.size() << "initial scanning paths:" << scanningPaths;
    });
}

void DefenderController::scanningUsbPathsChanged(const QStringList &list)
{
    qCInfo(logDFMBase) << "DefenderController::scanningUsbPathsChanged: Received signal with" 
                       << list.size() << "paths:" << list;
    
    int oldCount = scanningPaths.size();
    scanningPaths.clear();
    
    foreach (const QString &p, list) {
        if (!p.isEmpty()) {
            scanningPaths << QUrl::fromLocalFile(p);
        } else {
            qCWarning(logDFMBase) << "DefenderController::scanningUsbPathsChanged: Skipping empty path";
        }
    }
    
    qCDebug(logDFMBase) << "DefenderController::scanningUsbPathsChanged: Updated scanning paths count from" 
                        << oldCount << "to" << scanningPaths.size();
}

DefenderController::DefenderController(QObject *parent)
    : QObject(parent)
{
}

DefenderController::~DefenderController()
{
}

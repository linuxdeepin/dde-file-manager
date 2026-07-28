// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USBREPAIRPROXY_H
#define USBREPAIRPROXY_H

#include "usbrepair_interface.h"

#include <QObject>
#include <QDBusServiceWatcher>
#include <QScopedPointer>

class UsbRepairProxy : public QObject
{
    Q_OBJECT

public:
    explicit UsbRepairProxy(QObject *parent = nullptr);
    ~UsbRepairProxy();

    void startRepair(const QString &devicePath);
    void cancelRepair(const QString &devicePath);

Q_SIGNALS:
    void fsErrorDetected(const QString &devicePath,
                         const QString &deviceName,
                         const QString &fsType,
                         const QString &errorType,
                         bool canRepair,
                         const QString &message);
    void fsErrorCleared(const QString &devicePath);
    void repairProgress(const QString &devicePath,
                        int percent,
                        const QString &logLine);
    void repairFinished(const QString &devicePath,
                        bool success,
                        const QString &summary);

private Q_SLOTS:
    void onServiceRegistered(const QString &serviceName);
    void onServiceUnregistered(const QString &serviceName);

private:
    void connectSignals();

    using Interface = OrgDeepinFilemanagerUsbRepairInterface;
    QScopedPointer<Interface> m_interface;
    QDBusServiceWatcher *m_watcher { nullptr };

    static constexpr char kServiceName[] { "org.deepin.Filemanager.UsbRepair" };
    static constexpr char kServicePath[] { "/org/deepin/Filemanager/UsbRepair" };
};

#endif   // USBREPAIRPROXY_H

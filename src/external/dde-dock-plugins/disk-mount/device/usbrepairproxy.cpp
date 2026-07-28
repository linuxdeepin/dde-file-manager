// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbrepairproxy.h"

#include <QDBusConnection>
#include <QDBusPendingCallWatcher>

Q_DECLARE_LOGGING_CATEGORY(logAppDock)

UsbRepairProxy::UsbRepairProxy(QObject *parent)
    : QObject(parent)
{
    // NOTE: UsbRepair service is on system bus (not session bus!)
    m_watcher = new QDBusServiceWatcher(
        kServiceName,
        QDBusConnection::systemBus(),
        QDBusServiceWatcher::WatchForOwnerChange,
        this);

    connect(m_watcher, &QDBusServiceWatcher::serviceRegistered,
            this, &UsbRepairProxy::onServiceRegistered);
    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &UsbRepairProxy::onServiceUnregistered);

    // Try to connect immediately if service is already running
    onServiceRegistered(kServiceName);
}

UsbRepairProxy::~UsbRepairProxy()
{
}

void UsbRepairProxy::startRepair(const QString &devicePath)
{
    if (!m_interface || !m_interface->isValid())
        return;

    // Async call — the actual result comes via RepairFinished D-Bus signal
    m_interface->asyncCall("Repair", devicePath);
}

void UsbRepairProxy::cancelRepair(const QString &devicePath)
{
    if (!m_interface || !m_interface->isValid())
        return;

    m_interface->asyncCall("CancelRepair", devicePath);
}

void UsbRepairProxy::onServiceRegistered(const QString &serviceName)
{
    Q_UNUSED(serviceName)
    if (m_interface)
        return;

    m_interface.reset(new Interface(
        kServiceName,
        kServicePath,
        QDBusConnection::systemBus(),
        this));

    if (m_interface->isValid()) {
        connectSignals();
        qCInfo(logAppDock) << "UsbRepairProxy: connected to UsbRepair service on system bus";
    } else {
        qCWarning(logAppDock) << "UsbRepairProxy: failed to connect to UsbRepair service:"
                              << m_interface->lastError().message();
        m_interface.reset();
    }
}

void UsbRepairProxy::onServiceUnregistered(const QString &serviceName)
{
    Q_UNUSED(serviceName)
    qCInfo(logAppDock) << "UsbRepairProxy: UsbRepair service unregistered";
    m_interface.reset();
}

void UsbRepairProxy::connectSignals()
{
    if (!m_interface)
        return;

    connect(m_interface.data(), &Interface::FsErrorDetected,
            this, &UsbRepairProxy::fsErrorDetected);
    connect(m_interface.data(), &Interface::FsErrorCleared,
            this, &UsbRepairProxy::fsErrorCleared);
    connect(m_interface.data(), &Interface::RepairProgress,
            this, &UsbRepairProxy::repairProgress);
    connect(m_interface.data(), &Interface::RepairFinished,
            this, &UsbRepairProxy::repairFinished);
}

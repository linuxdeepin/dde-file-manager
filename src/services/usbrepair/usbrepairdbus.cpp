// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "usbrepairdbus.h"
#include "private/usbrepairdbus_p.h"

SERVICEUSBREPAIR_BEGIN_NAMESPACE
DFM_LOG_REGISTER_CATEGORY(SERVICEUSBREPAIR_NAMESPACE)
SERVICEUSBREPAIR_END_NAMESPACE

SERVICEUSBREPAIR_USE_NAMESPACE

void UsbRepairDBusPrivate::initConnect()
{
    // Monitor signals → forward as D-Bus signals
    QObject::connect(monitor, &UsbRepairMonitor::errorDetected,
                     q, &UsbRepairDBus::FsErrorDetected);
    QObject::connect(monitor, &UsbRepairMonitor::errorCleared,
                     q, &UsbRepairDBus::FsErrorCleared);

    // Worker signals → forward as D-Bus signals
    QObject::connect(worker, &UsbRepairWorker::progress,
                     q, &UsbRepairDBus::RepairProgress);
    QObject::connect(worker, &UsbRepairWorker::finished,
                     q, &UsbRepairDBus::RepairFinished);
}

UsbRepairDBus::UsbRepairDBus(const char *name, QObject *parent)
    : QObject(parent), QDBusContext(), d(new UsbRepairDBusPrivate(this))
{
    QDBusConnection::RegisterOptions opts =
            QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals;

    // NOTE: Use SystemBus (not SessionBus like textindex) because:
    // - fsck requires root privileges
    // - udisks2 events are on system bus
    QDBusConnection bus = QDBusConnection::connectToBus(QDBusConnection::SystemBus, QString(name));
    bus.registerObject(Defines::kServiceObjPath, this, opts);

    // Start monitoring immediately
    d->monitor->startMonitoring();

    fmInfo() << "UsbRepairDBus: service registered on system bus";
}

UsbRepairDBus::~UsbRepairDBus() { }

void UsbRepairDBus::cleanup()
{
    d->monitor->stopMonitoring();
}

bool UsbRepairDBus::Repair(const QString &devicePath, QString &errorMessage)
{
    QString callerBusName = message().service();
    return d->worker->startRepair(devicePath, callerBusName, errorMessage);
}

bool UsbRepairDBus::CancelRepair(const QString &devicePath)
{
    QString callerBusName = message().service();
    return d->worker->cancelRepair(devicePath, callerBusName);
}

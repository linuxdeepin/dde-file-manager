// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef USBREPAIRMONITOR_H
#define USBREPAIRMONITOR_H

#include "service_usbrepair_global.h"

#include <QObject>
#include <QDBusObjectPath>
#include <QMap>

class QDBusInterface;
class QDBusArgument;

SERVICEUSBREPAIR_BEGIN_NAMESPACE

class UsbRepairMonitor : public QObject
{
    Q_OBJECT

public:
    explicit UsbRepairMonitor(QObject *parent = nullptr);
    ~UsbRepairMonitor();

    void startMonitoring();
    void stopMonitoring();

Q_SIGNALS:
    void errorDetected(const QString &devicePath,
                       const QString &deviceName,
                       const QString &fsType,
                       const QString &errorType,
                       bool canRepair,
                       const QString &message);
    void errorCleared(const QString &devicePath);

private Q_SLOTS:
    void onInterfacesAdded(const QDBusObjectPath &objectPath,
                           const QMap<QString, QVariantMap> &interfacesAndProperties);
    void onInterfacesRemoved(const QDBusObjectPath &objectPath,
                             const QStringList &interfaces);

private:
    void checkDeviceHealth(const QString &blockObjPath);
    void checkMissingFilesystem(const QString &blockObjPath);
    bool isUsbDevice(const QString &blockObjPath, QString *deviceName = nullptr);
    QString getDeviceFile(const QString &blockObjPath);
    QString getFsType(const QString &deviceFile);
    bool isDeviceMounted(const QString &deviceFile);
    bool isHardwareReadOnly(const QString &deviceFile);
    bool checkDirtyBit(const QString &deviceFile, const QString &fsType);
    QString getParentDriveObjPath(const QString &blockObjPath);

    QDBusInterface *m_udisks2ObjMgr { nullptr };
};

SERVICEUSBREPAIR_END_NAMESPACE

#endif   // USBREPAIRMONITOR_H

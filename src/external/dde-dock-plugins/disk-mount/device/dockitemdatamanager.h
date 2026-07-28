// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DOCKITEMDATAMANAGER_H
#define DOCKITEMDATAMANAGER_H

#include "devicemanager_interface.h"
#include "typedefines.h"

#include <QObject>
#include <QMap>

typedef QMap<QString, DockItemData> ItemContainer;

using DeviceManager = OrgDeepinFilemanagerDaemonDeviceManagerInterface;
class UsbRepairProxy;
class RepairDialog;

struct PendingErrorInfo {
    QString deviceName;
    QString fsType;
    QString errorType;
    QString message;
};
class DockItemDataManager : public QObject
{
    Q_OBJECT

public:
    static DockItemDataManager *instance();
    void initialize();
    void ejectAll();
    void ejectDevice(const QString &id);

    void subscribeUsageMonitoring();
    void unsubscribeUsageMonitoring();
    void refreshUsage();  // 立即刷新设备容量

Q_SIGNALS:
    void requesetSetDockVisible(bool visible);
    void mountAdded(const DockItemData &item);
    void mountRemoved(const QString &id);
    void usageUpdated(const QString &id, quint64 used);

private Q_SLOTS:
    void onBlockMounted(const QString &id);
    void onBlockUnmounted(const QString &id);
    void onBlockPropertyChanged(const QString &id, const QString &property, const QDBusVariant &value);
    void onProtocolMounted(const QString &id);
    void onProtocolUnmounted(const QString &id);
    void sendNotification(const QString &id, const QString &operation);

    void onServiceRegistered();
    void onServiceUnregistered();

    // USB Repair slots
    void onFsErrorDetected(const QString &devicePath,
                           const QString &deviceName,
                           const QString &fsType,
                           const QString &errorType,
                           bool canRepair,
                           const QString &message);
    void onFsErrorCleared(const QString &devicePath);
    void onNotifyActionInvoked(uint notificationId, const QString &action);
    void onNotifyClosed(uint notificationId, uint reason);
    void onRepairProgress(const QString &devicePath, int percent, const QString &logLine);
    void onRepairFinished(const QString &devicePath, bool success, const QString &summary);

private:
    explicit DockItemDataManager(QObject *parent = nullptr);

    bool blockDeviceFilter(const QVariantMap &data);
    bool protoDeviceFilter(const QVariantMap &data);
    bool isRootDrive(const QString &drivePath);
    bool isMountPointInFstab(const QString &mountPoint);
    void playSoundOnDevPlugInOut(bool in);
    void updateDockVisible();
    void notify(const QString &title, const QString &msg, int timeout = 3000);
    void notifyWithActions(const QString &title, const QString &msg,
                           const QStringList &actions, const QString &devicePath, int timeout = 10000);
    void closeNotification(uint notificationId);

    DockItemData buildBlockItem(const QVariantMap &data);
    DockItemData buildProtocolItem(const QVariantMap &data);

    void connectDeviceManger();
    void watchService();
    void connectRepairService();

private:
    ItemContainer blocks;
    ItemContainer protocols;

    QScopedPointer<DeviceManager> devMng;

    // USB Repair
    UsbRepairProxy *m_repairProxy { nullptr };
    QMap<uint, QString> m_notificationToDevice;   // notification ID → devicePath
    QMap<QString, PendingErrorInfo> m_pendingErrors;   // devicePath → error info
    QMap<QString, RepairDialog *> m_repairDialogs;   // devicePath → repair dialog
};

#endif   // DOCKITEMDATAMANAGER_H

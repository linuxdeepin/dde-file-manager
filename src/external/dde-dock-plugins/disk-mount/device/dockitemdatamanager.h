// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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

private:
    explicit DockItemDataManager(QObject *parent = nullptr);

    bool blockDeviceFilter(const QVariantMap &data);
    bool protoDeviceFilter(const QVariantMap &data);
    bool isRootDrive(const QString &drivePath);
    void playSoundOnDevPlugInOut(bool in);
    void updateDockVisible();
    void notify(const QString &title, const QString &msg);

    DockItemData buildBlockItem(const QVariantMap &data);
    DockItemData buildProtocolItem(const QVariantMap &data);

    void connectDeviceManger();
    void watchService();

private:
    ItemContainer blocks;
    ItemContainer protocols;

    QScopedPointer<DeviceManager> devMng;
};

#endif   // DOCKITEMDATAMANAGER_H

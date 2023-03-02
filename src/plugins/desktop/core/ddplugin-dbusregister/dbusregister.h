// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef INTERFACES_H
#define INTERFACES_H

#include <dfm-framework/lifecycle/plugin.h>

#include <mutex>

class QDBusConnection;
class DeviceManagerDBus;
class OperationsStackManagerDbus;
class VaultManagerDBus;
class FileManager1DBus;
class DBusRegister : public dpf::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.desktop" FILE "dbusregister.json")

public:
    virtual void initialize() override;
    virtual bool start() override;

protected:
    void registerDBus();
private Q_SLOTS:
    void onWindowShowed();

private:
    static std::once_flag &onceFlag();

    void initServiceDBusInterfaces(QDBusConnection *connection);
    void initFreedesktopDBusInterfaces(QDBusConnection *connection);

    void initDeviceDBus(QDBusConnection *connection);
    void initOperationsDBus(QDBusConnection *connection);
    void initVaultDBus(QDBusConnection *connection);
    void initFileManager1DBus(QDBusConnection *connection);

private:
    QScopedPointer<DeviceManagerDBus> deviceManager;
    QScopedPointer<OperationsStackManagerDbus> operationsStackManager;
    QScopedPointer<VaultManagerDBus> vaultManager;
    QScopedPointer<FileManager1DBus> filemanager1;
};

#endif   // INTERFACES_H

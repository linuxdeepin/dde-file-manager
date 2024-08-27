// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CORE_H
#define CORE_H

#include "daemonplugin_core_global.h"

#include <dfm-framework/dpf.h>

class QDBusConnection;
class DeviceManagerDBus;
class OperationsStackManagerDbus;

DAEMONPCORE_BEGIN_NAMESPACE

class Core : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.daemon" FILE "core.json")

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    void initServiceDBusInterfaces(QDBusConnection *connection);
    void initDeviceDBus(QDBusConnection *connection);
    void initOperationsDBus(QDBusConnection *connection);

private Q_SLOTS:
    void exitOnShutdown(bool);

private:
    QScopedPointer<DeviceManagerDBus> deviceManager;
    QScopedPointer<OperationsStackManagerDbus> operationsStackManager;
};

DAEMONPCORE_END_NAMESPACE

#endif   // CORE_H

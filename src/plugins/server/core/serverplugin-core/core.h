// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CORE_H
#define CORE_H

#include "serverplugin_core_global.h"

#include <dfm-framework/dpf.h>

class QDBusConnection;
class DeviceManagerDBus;
class OperationsStackManagerDbus;

SERVERPCORE_BEGIN_NAMESPACE

class Core : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.server" FILE "core.json")

public:
    virtual void initialize() override;
    virtual bool start() override;

private:
    void initServiceDBusInterfaces(QDBusConnection *connection);
    void initDeviceDBus(QDBusConnection *connection);
    void initOperationsDBus(QDBusConnection *connection);

private:
    QScopedPointer<DeviceManagerDBus> deviceManager;
    QScopedPointer<OperationsStackManagerDbus> operationsStackManager;
};

SERVERPCORE_END_NAMESPACE

#endif   // CORE_H

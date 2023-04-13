// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef VAULTDAEMON_H
#define VAULTDAEMON_H

#include "serverplugin_vaultdaemon_global.h"

#include <dfm-framework/dpf.h>

#include <QObject>

class VaultManagerDBus;

SERVERVAULT_BEGIN_NAMESPACE

class VaultManagerDBusWorker : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void launchService();

private:
    QScopedPointer<VaultManagerDBus> vaultManager;
};

class VaultDaemon : public DPF_NAMESPACE::Plugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.deepin.plugin.server" FILE "vaultdaemon.json")

public:
    virtual void initialize() override;
    virtual bool start() override;
    virtual void stop() override;

Q_SIGNALS:
    void requesLaunch();

private:
    QThread workerThread;
};
SERVERVAULT_END_NAMESPACE

#endif // VAULTDAEMON_H

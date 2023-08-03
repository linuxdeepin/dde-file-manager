// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultdaemon.h"
#include "vaultcontrol.h"
#include "dbus/vaultmanagerdbus.h"
#include "vaultmanageradaptor.h"

#include <QDBusConnection>

SERVERVAULT_USE_NAMESPACE

void VaultManagerDBusWorker::launchService()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());

    QDBusConnection conn { QDBusConnection::sessionBus() };
    // register service
    if (!conn.registerService("org.deepin.filemanager.server")) {
        qCritical("Vault Daemon: Cannot register the \"org.deepin.filemanager.server\" service!!!\n");
        ::exit(EXIT_FAILURE);
    }

    qInfo() << "Init DBus VaultManager start";
    // register object
    vaultManager.reset(new VaultManagerDBus);
    Q_UNUSED(new VaultManagerAdaptor(vaultManager.data()));
    if (!conn.registerObject("/org/deepin/filemanager/server/VaultManager",
                                    vaultManager.data())) {
        qWarning("Vault Daemon: Cannot register the \"/org/deepin/filemanager/server/VaultManager\" object.\n");
        vaultManager.reset(nullptr);
    }
    qInfo() << "Vault Daemon: Init DBus VaultManager end";
}

void VaultDaemon::initialize()
{
    VaultManagerDBusWorker *worker { new VaultManagerDBusWorker };
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &VaultDaemon::requesLaunch, worker, &VaultManagerDBusWorker::launchService);
    workerThread.start();
}

bool VaultDaemon::start()
{
    VaultControl::instance()->connectLockScreenDBus();
    VaultControl::instance()->transparentUnlockVault();
    Q_EMIT requesLaunch();
    return true;
}

void VaultDaemon::stop()
{
    workerThread.quit();
    workerThread.wait();
}

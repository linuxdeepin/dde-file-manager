// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "vaultdaemon.h"
#include "vaultcontrol.h"
#include "dbus/vaultmanagerdbus.h"
#include "vaultmanageradaptor.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QDBusConnection>

DFMBASE_USE_NAMESPACE
namespace serverplugin_vaultdaemon {
DFM_LOG_REISGER_CATEGORY(SERVERVAULT_NAMESPCE)

void VaultManagerDBusWorker::launchService()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());

    QDBusConnection conn { QDBusConnection::sessionBus() };
    // register service
    if (!conn.registerService("org.deepin.filemanager.server")) {
        fmCritical("Vault Daemon: Cannot register the \"org.deepin.filemanager.server\" service!!!\n");
        ::exit(EXIT_FAILURE);
    }

    fmInfo() << "Init DBus VaultManager start";
    // register object
    vaultManager.reset(new VaultManagerDBus);
    Q_UNUSED(new VaultManagerAdaptor(vaultManager.data()));
    if (!conn.registerObject("/org/deepin/filemanager/server/VaultManager",
                             vaultManager.data())) {
        fmWarning("Vault Daemon: Cannot register the \"/org/deepin/filemanager/server/VaultManager\" object.\n");
        vaultManager.reset(nullptr);
    }
    fmInfo() << "Vault Daemon: Init DBus VaultManager end";
}

void VaultManagerDBusWorker::sendChangedVaultStateSig(const QVariantMap &map)
{
    Q_EMIT vaultManager->ChangedVaultState(map);
}

void VaultDaemon::initialize()
{
    VaultManagerDBusWorker *worker { new VaultManagerDBusWorker };
    worker->moveToThread(&workerThread);
    connect(&workerThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(this, &VaultDaemon::requesLaunch, worker, &VaultManagerDBusWorker::launchService);
    connect(VaultControl::instance(), &VaultControl::changedVaultState, worker, &VaultManagerDBusWorker::sendChangedVaultStateSig);
    workerThread.start();
}

bool VaultDaemon::start()
{
    QString err;
    if (!DConfigManager::instance()->addConfig(kVaultDConfigName, &err))
        fmWarning() << "Vault: create dconfig failed: " << err;

    VaultControl::instance()->connectLockScreenDBus();
    VaultControl::instance()->transparentUnlockVault();

    const QVariant vRe = DConfigManager::instance()->value(kVaultDConfigName, "enableUnlockVaultInNetwork");
    if (vRe.isValid() && !vRe.toBool()) {
        VaultControl::instance()->MonitorNetworkStatus();
    }

    Q_EMIT requesLaunch();
    return true;
}

void VaultDaemon::stop()
{
    workerThread.quit();
    workerThread.wait();
}

}   // namespace serverplugin_vaultdaemon

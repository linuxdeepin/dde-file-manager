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
namespace daemonplugin_vaultdaemon {
DFM_LOG_REISGER_CATEGORY(DAEMONPVAULT_NAMESPCE)

static constexpr char kDaemonName[] { "org.deepin.Filemanager.Daemon" };
static constexpr char kVaultManagerObjPath[] { "/org/deepin/Filemanager/Daemon/VaultManager" };

void VaultManagerDBusWorker::launchService()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());

    QDBusConnection conn { QDBusConnection::sessionBus() };
    // register service
    if (!conn.registerService(kDaemonName)) {
        fmCritical() << QString("Vault Daemon: Cannot register the \"%1\" service!!!\n").arg(kDaemonName);
        ::exit(EXIT_FAILURE);
    }

    fmInfo() << "Init DBus VaultManager start";
    // register object
    vaultManager.reset(new VaultManagerDBus);
    Q_UNUSED(new VaultManagerAdaptor(vaultManager.data()));
    if (!conn.registerObject(kVaultManagerObjPath,
                             vaultManager.data())) {
        fmWarning() << QString("Vault Daemon: Cannot register the \"%1\" object.\n").arg(kVaultManagerObjPath);
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

}   // namespace daemonplugin_vaultdaemon

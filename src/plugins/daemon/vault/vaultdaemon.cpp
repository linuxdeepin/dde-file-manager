// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
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
DFM_LOG_REGISTER_CATEGORY(DAEMONPVAULT_NAMESPCE)

static constexpr char kDaemonName[] { "org.deepin.Filemanager.Daemon" };
static constexpr char kVaultManagerObjPath[] { "/org/deepin/Filemanager/Daemon/VaultManager" };

void VaultManagerDBusWorker::launchService()
{
    Q_ASSERT(QThread::currentThread() != qApp->thread());

    QDBusConnection conn { QDBusConnection::sessionBus() };
    // register service
    if (!conn.registerService(kDaemonName)) {
        fmCritical() << "[VaultManagerDBusWorker::launchService] Failed to register DBus service:" << kDaemonName;
        ::exit(EXIT_FAILURE);
    }

    fmInfo() << "[VaultManagerDBusWorker::launchService] Initializing DBus VaultManager service";
    // register object
    vaultManager.reset(new VaultManagerDBus);
    Q_UNUSED(new VaultManagerAdaptor(vaultManager.data()));
    if (!conn.registerObject(kVaultManagerObjPath,
                             vaultManager.data())) {
        fmCritical() << "[VaultManagerDBusWorker::launchService] Failed to register DBus object:" << kVaultManagerObjPath;
        vaultManager.reset(nullptr);
    }
    fmInfo() << "[VaultManagerDBusWorker::launchService] DBus VaultManager service initialized successfully";
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
        fmWarning() << "[VaultDaemon::start] Failed to create vault dconfig:" << err;

    VaultControl::instance()->connectLockScreenDBus();
    VaultControl::instance()->transparentUnlockVault();

    const QVariant vRe = DConfigManager::instance()->value(kVaultDConfigName, "enableUnlockVaultInNetwork");
    if (vRe.isValid() && !vRe.toBool()) {
        fmInfo() << "[VaultDaemon::start] Network unlock disabled, starting network monitoring";
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

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage map for src/plugins/daemon/vault/vaultdaemon.cpp:
//   VaultDaemon::initialize                        -> Lifecycle_InitializeStartStop_ThreadCleanShutdown
//   VaultDaemon::start                             -> Lifecycle_InitializeStartStop_ThreadCleanShutdown
//                                                    (also drives VaultControl::connectLockScreenDBus,
//                                                     transparentUnlockVault and MonitorNetworkStatus)
//   VaultDaemon::stop                              -> Lifecycle_InitializeStartStop_ThreadCleanShutdown
//   VaultManagerDBusWorker::launchService          -> reached through requesLaunch with the DBus
//                                                    registration stubbed to succeed
//   daemonplugin_vaultdaemon::__logdaemonplugin_vaultdaemon / __getLogCategoryName
//                                                 -> initialized by any fm* log line in this plugin
// VaultManagerDBusWorker::sendChangedVaultStateSig is NOT covered: its only member (the vault
// manager pointer) is private and null in a fresh worker, emitting through it would crash.
// Branch list for start(): addConfig fails -> warning only; dconfig value invalid/true ->
// skip network monitoring; value false -> MonitorNetworkStatus; then requesLaunch emitted.

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include <QDBusConnection>
#include <QThread>
#include <QTimer>

#include "vaultcontrol.h"
#include "vaultdaemon.h"

DAEMONPVAULT_USE_NAMESPACE

class UT_VaultDaemonCov : public testing::Test
{
protected:
    void SetUp() override
    {
        // keep the DBus registration calls from touching a real bus / calling ::exit on failure
        stub.set_lamda(static_cast<bool (QDBusConnection::*)(const QString &)>(&QDBusConnection::registerService),
                       [](QDBusConnection *, const QString &service) {
                           __DBG_STUB_INVOKE__
                           return service == QStringLiteral("org.deepin.Filemanager.Daemon");
                       });
        stub.set_lamda(static_cast<bool (QDBusConnection::*)(const QString &, QObject *, QDBusConnection::RegisterOptions)>(&QDBusConnection::registerObject),
                       [](QDBusConnection *, const QString &, QObject *, QDBusConnection::RegisterOptions) {
                           __DBG_STUB_INVOKE__
                           return true;
                       });
    }
    void TearDown() override { stub.clear(); }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_VaultDaemonCov, Lifecycle_InitializeStartStop_ThreadCleanShutdown)
{
    // Arrange
    VaultDaemon daemon;

    // Act
    daemon.initialize();
    bool started = daemon.start();
    QThread::msleep(300);   // let the worker thread run launchService()
    daemon.stop();

    // Assert
    EXPECT_TRUE(started);
    EXPECT_TRUE(daemon.workerThread.isFinished());
    EXPECT_NE(VaultControl::instance(), nullptr);   // control singleton survived the lifecycle
}

TEST_F(UT_VaultDaemonCov, Start_AfterStop_StartsAgain)
{
    // Arrange
    VaultDaemon daemon;
    daemon.initialize();
    ASSERT_TRUE(daemon.start());
    daemon.stop();
    ASSERT_TRUE(daemon.workerThread.isFinished());

    // Act
    bool restarted = daemon.start();

    // Assert
    EXPECT_TRUE(restarted);
    EXPECT_NE(VaultControl::instance(), nullptr);   // control singleton still alive
    daemon.stop();
}

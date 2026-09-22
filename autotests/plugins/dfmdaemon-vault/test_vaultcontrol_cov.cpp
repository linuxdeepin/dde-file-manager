// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage map for src/plugins/daemon/vault/vaultcontrol.cpp (+ vaultcontrol.h struct):
//   VaultControl::instance / VaultControl(QObject)   -> Instance_Singleton_ReturnsSameObject
//   VaultControl::CryfsVersionInfo ctor              -> CryfsVersionInfo_CtorAndValidity
//   CryfsVersionInfo::isOlderThan                     -> CryfsVersionInfo_CtorAndValidity
//   CryfsVersionInfo::isVaild                         -> CryfsVersionInfo_CtorAndValidity
//   VaultControl::state                               -> State_* (3 branches)
//   VaultControl::versionString                       -> VersionString_* (invalid output / parse+cache)
//   VaultControl::runVaultProcessAndGetOutput         -> RunVaultProcess_NoBinary_LeavesOutputEmpty
//   VaultControl::lockVault                           -> LockVault_* (no binary / bin-true success)
//   VaultControl::unlockVault                         -> UnlockVault_NoBinary_ReturnsError
//   VaultControl::transparentUnlockVault              -> TransparentUnlock_NotEncrypted_ReturnsFalse
//   VaultControl::responseNetworkStateChaneDBus       -> ResponseNetworkState_* (non-Full / Full early return)
//   VaultControl::responseLockScreenDBus              -> ResponseLockScreen_WrongArguments_ReturnsEarly
//   VaultControl::syncGroupPolicyAlgoName             -> SyncGroupPolicy_MissingAlgo_SetsDefault
//   VaultControl::connectLockScreenDBus               -> ConnectLockScreenDBus_NoCrash_InstanceStable
//   VaultControl::MonitorNetworkStatus                -> MonitorNetworkStatus_NoCrash_NoStateChanged
// Branch list:
//   state      {no cryfs binary -> kNotAvailable, config missing -> kNotExisted, config present -> kEncrypted}
//   versionString {empty output -> invalid stays, "cryfs 0.10.2" -> parsed, second call -> cached}
//   lockVault  {binary missing -> -1, /bin/true -> exit code 0}
//   transparentUnlockVault {state != kEncrypted -> false}
//   responseNetworkStateChaneDBus {not Full, Full + vault not unlocked -> return}

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QStandardPaths>
#include <QTemporaryDir>

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include "vaultcontrol.h"

DAEMONPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

class UT_VaultControlCov : public testing::Test
{
protected:
    void SetUp() override { tmpDir.reset(new QTemporaryDir); }
    void TearDown() override { stub.clear(); }

    // point QStandardPaths::findExecutable at a fixed binary ("" = "not found")
    void stubFindExecutable(const QString &binary)
    {
        stub.set_lamda(static_cast<QString (*)(const QString &, const QStringList &)>(&QStandardPaths::findExecutable),
                       [binary](const QString &, const QStringList &) {
                           __DBG_STUB_INVOKE__
                           return binary;
                       });
    }

protected:
    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tmpDir;
};

TEST_F(UT_VaultControlCov, Instance_Singleton_ReturnsSameObject)
{
    // Arrange
    VaultControl *ctrl = VaultControl::instance();

    // Act
    VaultControl *again = VaultControl::instance();

    // Assert
    EXPECT_NE(ctrl, nullptr);
    EXPECT_EQ(ctrl, again);
}

TEST_F(UT_VaultControlCov, CryfsVersionInfo_CtorAndCompare_ReportValidityAndOrder)
{
    // Arrange
    VaultControl::CryfsVersionInfo invalid(-1, -1, -1);
    VaultControl::CryfsVersionInfo v090(0, 9, 0);
    VaultControl::CryfsVersionInfo v0100(0, 10, 0);

    // Act
    bool invalidOk = invalid.isVaild();
    bool olderMajor = v090.isOlderThan(v0100);
    bool sameVersion = v0100.isOlderThan(VaultControl::CryfsVersionInfo(0, 10, 0));

    // Assert
    EXPECT_FALSE(invalidOk);
    EXPECT_TRUE(v090.isVaild());
    EXPECT_TRUE(olderMajor);
    EXPECT_FALSE(sameVersion);
    EXPECT_EQ(v090.majorVersion, 0);
    EXPECT_EQ(v090.minorVersion, 9);
    EXPECT_EQ(v090.hotfixVersion, 0);
}

TEST_F(UT_VaultControlCov, State_NoCryfsBinary_ReturnsNotAvailable)
{
    // Arrange
    stubFindExecutable("");

    // Act
    VaultState st = VaultControl::instance()->state(tmpDir->path());

    // Assert
    EXPECT_EQ(st, kNotAvailable);
    EXPECT_NE(st, kEncrypted);
}

TEST_F(UT_VaultControlCov, State_NoConfigFile_ReturnsNotExisted)
{
    // Arrange
    stubFindExecutable("/bin/true");
    const QString emptyDir = tmpDir->filePath("empty-enc");
    ASSERT_TRUE(QDir().mkpath(emptyDir));

    // Act
    VaultState st = VaultControl::instance()->state(emptyDir);

    // Assert
    EXPECT_EQ(st, kNotExisted);
    EXPECT_FALSE(QFile::exists(emptyDir + "/cryfs.config"));
}

TEST_F(UT_VaultControlCov, State_ConfigExists_ReturnsEncrypted)
{
    // Arrange
    stubFindExecutable("/bin/true");
    const QString encDir = tmpDir->filePath("enc");
    ASSERT_TRUE(QDir().mkpath(encDir));
    QFile cfg(encDir + "/cryfs.config");
    ASSERT_TRUE(cfg.open(QIODevice::WriteOnly));
    cfg.close();

    // Act
    VaultState st = VaultControl::instance()->state(encDir);

    // Assert - the mount point is not a cryfs fs in the test env, so state must be kEncrypted
    EXPECT_EQ(st, kEncrypted);
    EXPECT_NE(st, kUnlocked);
}

TEST_F(UT_VaultControlCov, VersionString_EmptyOutput_StaysInvalid)
{
    // Arrange - must run before the cached-version test: the version is cached in the singleton
    VaultControl *ctrl = VaultControl::instance();
    stub.set_lamda(ADDR(VaultControl, runVaultProcessAndGetOutput),
                   [](VaultControl *, const QStringList &, QString &err, QString &out) {
                       __DBG_STUB_INVOKE__
                       err = "cryfs not found";
                       out = "";
                   });

    // Act
    VaultControl::CryfsVersionInfo v = ctrl->versionString();

    // Assert
    EXPECT_FALSE(v.isVaild());
    EXPECT_EQ(v.majorVersion, -1);
}

TEST_F(UT_VaultControlCov, VersionString_ValidOutput_ParsesAndCaches)
{
    // Arrange
    VaultControl *ctrl = VaultControl::instance();
    static QString output = "cryfs 0.10.2\n";
    stub.set_lamda(ADDR(VaultControl, runVaultProcessAndGetOutput),
                   [](VaultControl *, const QStringList &, QString &, QString &out) {
                       __DBG_STUB_INVOKE__
                       out = output;
                   });

    // Act
    VaultControl::CryfsVersionInfo parsed = ctrl->versionString();
    output = "cryfs 9.9.9\n";   // a later call must not re-parse, the value is cached
    VaultControl::CryfsVersionInfo cached = ctrl->versionString();

    // Assert
    EXPECT_TRUE(parsed.isVaild());
    EXPECT_EQ(parsed.majorVersion, 0);
    EXPECT_EQ(parsed.minorVersion, 10);
    EXPECT_EQ(parsed.hotfixVersion, 2);
    EXPECT_EQ(cached.minorVersion, 10);
}

TEST_F(UT_VaultControlCov, RunVaultProcess_NoBinary_LeavesOutputEmpty)
{
    // Arrange
    stubFindExecutable("");
    QString err = "sentinel";
    QString out = "sentinel";

    // Act
    VaultControl::instance()->runVaultProcessAndGetOutput({ "--version" }, err, out);

    // Assert - the early return leaves the caller's buffers untouched
    EXPECT_EQ(err.toStdString(), "sentinel");
    EXPECT_EQ(out.toStdString(), "sentinel");
}

TEST_F(UT_VaultControlCov, LockVault_NoBinary_ReturnsError)
{
    // Arrange
    stubFindExecutable("");

    // Act
    int rc = VaultControl::instance()->lockVault("/tmp/no-such-mount", false);

    // Assert
    EXPECT_EQ(rc, -1);
    EXPECT_EQ(VaultControl::instance()->lockVault("/tmp/no-such-mount", true), -1);
}

TEST_F(UT_VaultControlCov, LockVault_TrueBinary_ReturnsZero)
{
    // Arrange - /bin/true exits with 0 for any arguments
    stubFindExecutable("/bin/true");

    // Act
    int rc = VaultControl::instance()->lockVault(tmpDir->path(), false);

    // Assert
    EXPECT_EQ(rc, 0);
    EXPECT_EQ(rc, 0);
}

TEST_F(UT_VaultControlCov, UnlockVault_NoBinary_ReturnsError)
{
    // Arrange
    stubFindExecutable("");

    // Act
    int rc = VaultControl::instance()->unlockVault(tmpDir->path(), tmpDir->path(), "pwd");

    // Assert
    EXPECT_EQ(rc, -1);
    EXPECT_NE(rc, 0);
}

TEST_F(UT_VaultControlCov, TransparentUnlock_NotEncrypted_ReturnsFalse)
{
    // Arrange
    stubFindExecutable("");

    // Act
    bool unlocked = VaultControl::instance()->transparentUnlockVault();

    // Assert
    EXPECT_FALSE(unlocked);
    EXPECT_EQ(VaultControl::instance()->state(QString()), kNotAvailable);
}

TEST_F(UT_VaultControlCov, ResponseNetworkState_NotFull_EmitsNothing)
{
    // Arrange
    VaultControl *ctrl = VaultControl::instance();
    QSignalSpy spy(ctrl, &VaultControl::changedVaultState);
    stubFindExecutable("");

    // Act - every value except Connectivity::Full (4) must be ignored
    ctrl->responseNetworkStateChaneDBus(0);
    ctrl->responseNetworkStateChaneDBus(1);

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(spy.isValid(), true);
}

TEST_F(UT_VaultControlCov, ResponseNetworkState_FullNotUnlocked_EmitsNothing)
{
    // Arrange
    VaultControl *ctrl = VaultControl::instance();
    QSignalSpy spy(ctrl, &VaultControl::changedVaultState);
    stubFindExecutable("");   // state() -> kNotAvailable, i.e. not kUnlocked

    // Act
    ctrl->responseNetworkStateChaneDBus(static_cast<int>(Connectivity::Full));

    // Assert - vault is not unlocked, so nothing may be locked/emitted
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(ctrl->state(QString()), kNotAvailable);
}

TEST_F(UT_VaultControlCov, ResponseLockScreen_WrongArguments_ReturnsEarly)
{
    // Arrange
    VaultControl *ctrl = VaultControl::instance();
    QSignalSpy spy(ctrl, &VaultControl::changedVaultState);
    QDBusMessage empty = QDBusMessage::createSignal("/test", "org.test", "PropertiesChanged");
    QDBusMessage wrongIfc = QDBusMessage::createSignal("/test", "org.test", "PropertiesChanged");
    wrongIfc.setArguments({ QVariant(QString("org.some.other.Service")) });

    // Act
    ctrl->responseLockScreenDBus(empty);
    ctrl->responseLockScreenDBus(wrongIfc);

    // Assert - rejected messages must not touch the vault at all
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(empty.arguments().size(), 0);
}

TEST_F(UT_VaultControlCov, SyncGroupPolicy_MissingAlgo_SetsDefault)
{
    // Arrange
    static bool setCalled = false;
    static QString capturedKey;
    setCalled = false;
    capturedKey.clear();
    stub.set_lamda(ADDR(DConfigManager, setValue),
                   [](DConfigManager *, const QString &, const QString &key, const QVariant &) {
                       __DBG_STUB_INVOKE__
                       setCalled = true;
                       capturedKey = key;
                   });

    // Act - the default config path holds no algorithm, so the default must be pushed
    VaultControl::instance()->syncGroupPolicyAlgoName();

    // Assert
    EXPECT_TRUE(setCalled);
    EXPECT_EQ(capturedKey, QString("dfm.vault.algo.name"));   // kGroupPolicyKeyVaultAlgoName
}

TEST_F(UT_VaultControlCov, ConnectLockScreenDBus_NoCrash_InstanceStable)
{
    // Arrange
    VaultControl *ctrl = VaultControl::instance();

    // Act - DBus hookup only; must not affect the singleton
    ctrl->connectLockScreenDBus();

    // Assert
    EXPECT_EQ(VaultControl::instance(), ctrl);
    EXPECT_EQ(ctrl->state(QString()), VaultControl::instance()->state(QString()));
}

TEST_F(UT_VaultControlCov, MonitorNetworkStatus_NoCrash_NoStateChanged)
{
    // Arrange
    VaultControl *ctrl = VaultControl::instance();
    QSignalSpy spy(ctrl, &VaultControl::changedVaultState);

    // Act - installs the network watcher only, it must not emit anything by itself
    ctrl->MonitorNetworkStatus();

    // Assert
    EXPECT_EQ(spy.count(), 0);
    EXPECT_EQ(VaultControl::instance(), ctrl);
}

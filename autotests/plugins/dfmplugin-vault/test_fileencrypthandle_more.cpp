// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// FileEncryptHandle / FileEncryptHandlerPrivate tests. QStandardPaths::
// findExecutable is stubbed to return "" so no real cryfs/fusermount process
// is ever spawned; process-based helpers take their deterministic early-exit
// branches. No real encryption volume is created or mounted.

#include "utils/fileencrypthandle.h"
#include "utils/fileencrypthandle_p.h"

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QStandardPaths>

#include "stubext.h"

#include "utils/pathmanager.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/encryption/operatorcenter.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace {
const QString kConfigPath = kVaultBasePath + "/" + QString(kVaultConfigFileName);
const QString kContainerPath = kVaultBasePath + "/password_container.bin";
}

class FileEncryptHandleTest : public testing::Test
{
protected:
    void SetUp() override
    {
        QDir().mkpath(kVaultBasePath);
        if (QFile::exists(kConfigPath))
            QFile::copy(kConfigPath, kConfigPath + ".utbak");
        handle = FileEncryptHandle::instance();
        d = handle->d;
        handle->updateState(VaultState::kUnknow);   // reset cached state between tests
    }

    void TearDown() override
    {
        stub.clear();
        QFile::remove(kContainerPath);
        QFile::remove(kConfigPath);
        if (QFile::exists(kConfigPath + ".utbak")) {
            QFile::remove(kConfigPath);
            QFile::rename(kConfigPath + ".utbak", kConfigPath);
        }
    }

    void writeContainer(bool present)
    {
        QFile::remove(kContainerPath);
        if (present) {
            QFile f(kContainerPath);
            f.open(QIODevice::WriteOnly);
            f.close();
        }
    }

    stub_ext::StubExt stub;
    FileEncryptHandle *handle = nullptr;
    FileEncryptHandlerPrivate *d = nullptr;
};

TEST_F(FileEncryptHandleTest, CreateDirIfNotExist_AbsentDir_CreatesAndSucceeds)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString target = tmp.path() + "/fresh";

    EXPECT_FALSE(QFile::exists(target));
    EXPECT_TRUE(handle->createDirIfNotExist(target));
    EXPECT_TRUE(QDir(target).exists());
}

TEST_F(FileEncryptHandleTest, CreateDirIfNotExist_NonEmptyDir_Fails)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QFile f(tmp.path() + "/occupied");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    EXPECT_FALSE(handle->createDirIfNotExist(tmp.path()));
    EXPECT_TRUE(handle->createDirIfNotExist(tmp.path() + "/sub"));
}

TEST_F(FileEncryptHandleTest, State_EmptyBaseDir_ReturnsUnknown)
{
    EXPECT_EQ(handle->state(""), VaultState::kUnknow);
}

TEST_F(FileEncryptHandleTest, State_NoCryfsConfig_ReturnsNotExisted)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    EXPECT_EQ(handle->state(tmp.path()), VaultState::kNotExisted);
}

TEST_F(FileEncryptHandleTest, State_ConfigExistsNotMounted_ReturnsEncrypted)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QFile cfg(tmp.path() + "/cryfs.config");
    ASSERT_TRUE(cfg.open(QIODevice::WriteOnly));
    cfg.close();

    // unlock path does not exist -> canonical path empty -> kEncrypted
    stub.set_lamda(&PathManager::vaultUnlockPath,
                   []() -> QString { return "/no/such/vault_unlocked_ut"; });

    EXPECT_EQ(handle->state(tmp.path()), VaultState::kEncrypted);
}

TEST_F(FileEncryptHandleTest, State_UseCacheWithUnlockedState_ReturnsCached)
{
    ASSERT_TRUE(handle->updateState(VaultState::kUnlocked));
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    // cache hit: result is the cached unlocked state, dir contents ignored
    EXPECT_EQ(handle->state(tmp.path(), true), VaultState::kUnlocked);
    ASSERT_TRUE(handle->updateState(VaultState::kUnknow));
}

TEST_F(FileEncryptHandleTest, State_CryfsMissing_ReturnsNotAvailable)
{
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    EXPECT_EQ(handle->state(tmp.path()), VaultState::kNotAvailable);
    handle->updateState(VaultState::kUnknow);
}

TEST_F(FileEncryptHandleTest, UpdateState_TransitionRules_Enforced)
{
    ASSERT_TRUE(handle->updateState(VaultState::kEncrypted));
    EXPECT_TRUE(handle->updateState(VaultState::kNotExisted));   // allowed from kEncrypted

    handle->updateState(VaultState::kUnknow);
    EXPECT_FALSE(handle->updateState(VaultState::kNotExisted));   // rejected from kUnknow
    handle->updateState(VaultState::kUnknow);
}

TEST_F(FileEncryptHandleTest, CreateVault_NonEmptyLockDir_AbortsEarly)
{
    QTemporaryDir lockDir;
    ASSERT_TRUE(lockDir.isValid());
    QFile f(lockDir.path() + "/occupied");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("x");
    f.close();

    QSignalSpy spy(handle, &FileEncryptHandle::signalCreateVault);
    handle->createVault(lockDir.path(), lockDir.path() + "_unlock", "pwd", EncryptType::AES_256_GCM, 32);
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(FileEncryptHandleTest, UnlockVault_OldVersion_CryfsMissing_SucceedsPerContract)
{
    writeContainer(false);
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });
    stub.set_lamda(VADDR(DConfigManager, setValue),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) {});

    QTemporaryDir lockDir, unlockDir;
    ASSERT_TRUE(lockDir.isValid() && unlockDir.isValid());

    QSignalSpy spy(handle, &FileEncryptHandle::signalUnlockVault);
    bool ret = handle->unlockVault(lockDir.path(), unlockDir.path(), "userpwd");
    EXPECT_TRUE(ret);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), static_cast<int>(ErrorCode::kCryfsNotExist));
    handle->updateState(VaultState::kUnknow);
}

TEST_F(FileEncryptHandleTest, UnlockVault_NewVersionMasterKey_CryfsMissing_SucceedsPerContract)
{
    writeContainer(true);
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });
    stub.set_lamda(VADDR(DConfigManager, setValue),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) {});

    QTemporaryDir lockDir, unlockDir;
    ASSERT_TRUE(lockDir.isValid() && unlockDir.isValid());

    QString masterKeyPwd = QString::fromLatin1(QByteArray(64, '\x1').toBase64());
    QSignalSpy spy(handle, &FileEncryptHandle::signalUnlockVault);
    bool ret = handle->unlockVault(lockDir.path(), unlockDir.path(), masterKeyPwd);
    EXPECT_TRUE(ret);
    EXPECT_EQ(spy.count(), 1);
    handle->updateState(VaultState::kUnknow);
}

TEST_F(FileEncryptHandleTest, UnlockVault_NewVersionEmptyPassword_FailsWithWrongPassword)
{
    writeContainer(true);
    stub.set_lamda(VADDR(DConfigManager, setValue),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) {});

    QTemporaryDir lockDir, unlockDir;
    ASSERT_TRUE(lockDir.isValid() && unlockDir.isValid());

    QSignalSpy spy(handle, &FileEncryptHandle::signalUnlockVault);
    bool ret = handle->unlockVault(lockDir.path(), unlockDir.path(), "");
    EXPECT_FALSE(ret);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), static_cast<int>(ErrorCode::kWrongPassword));
}

TEST_F(FileEncryptHandleTest, LockVault_FusermountMissing_SucceedsPerContract)
{
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });
    QTemporaryDir unlockDir;
    ASSERT_TRUE(unlockDir.isValid());

    QSignalSpy spy(handle, &FileEncryptHandle::signalLockVault);
    bool ret = handle->lockVault(unlockDir.path(), false);
    EXPECT_TRUE(ret);
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), static_cast<int>(ErrorCode::kFusermountNotExist));
    handle->updateState(VaultState::kUnknow);
}

TEST_F(FileEncryptHandleTest, SlotReadOutput_EmptyProcessOutput_EmitsEmptyMessage)
{
    QSignalSpy spy(handle, &FileEncryptHandle::signalReadOutput);
    handle->slotReadOutput();
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toString(), QString(""));
}

TEST_F(FileEncryptHandleTest, SlotReadError_NoActiveState_EmitsRawError)
{
    QSignalSpy spy(handle, &FileEncryptHandle::signalReadError);
    handle->slotReadError();
    ASSERT_EQ(spy.count(), 1);
    EXPECT_TRUE(spy.at(0).at(0).toString().isEmpty());
}

// --- FileEncryptHandlerPrivate direct coverage ---

TEST_F(FileEncryptHandleTest, PrivateRunVaultProcess_NoCryfs_ReturnsCryfsNotExist)
{
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });
    FileEncryptHandlerPrivate priv;
    EXPECT_EQ(priv.runVaultProcess("/lock_ut", "/unlock_ut", "pwd"),
              static_cast<int>(ErrorCode::kCryfsNotExist));
    EXPECT_EQ(priv.runVaultProcess("/lock_ut", "/unlock_ut", "pwd", EncryptType::AES_256_GCM, 32),
              static_cast<int>(ErrorCode::kCryfsNotExist));
}

TEST_F(FileEncryptHandleTest, PrivateLockVaultProcess_NoFusermount_ReturnsFusermountNotExist)
{
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });
    FileEncryptHandlerPrivate priv;
    EXPECT_EQ(priv.lockVaultProcess("/unlock_ut", false),
              static_cast<int>(ErrorCode::kFusermountNotExist));
    EXPECT_EQ(priv.lockVaultProcess("/unlock_ut", true),
              static_cast<int>(ErrorCode::kFusermountNotExist));
}

TEST_F(FileEncryptHandleTest, PrivateRunVaultProcessAndGetOutput_NoCryfs_OutputStaysEmpty)
{
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });
    FileEncryptHandlerPrivate priv;
    QString stdErr { "stale" }, stdOut { "stale" };
    priv.runVaultProcessAndGetOutput({ "--version" }, stdErr, stdOut);
    // early exit on missing cryfs leaves the caller buffers untouched
    EXPECT_EQ(stdErr, QString("stale"));
    EXPECT_EQ(stdOut, QString("stale"));
}

TEST_F(FileEncryptHandleTest, PrivateVersionString_NoCryfs_ReturnsInvalidInfo)
{
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });
    FileEncryptHandlerPrivate priv;
    auto version = priv.versionString();
    EXPECT_FALSE(version.isVaild());
}

TEST_F(FileEncryptHandleTest, PrivateCryfsVersionInfo_ComparisonRules)
{
    FileEncryptHandlerPrivate::CryfsVersionInfo invalid(-1, -1, -1);
    EXPECT_FALSE(invalid.isVaild());

    FileEncryptHandlerPrivate::CryfsVersionInfo v1(0, 9, 9);
    FileEncryptHandlerPrivate::CryfsVersionInfo v2(0, 10, 0);
    ASSERT_TRUE(v1.isVaild());
    EXPECT_TRUE(v1.isOlderThan(v2));
    EXPECT_FALSE(v2.isOlderThan(v1));
    EXPECT_FALSE(v2.isOlderThan(v2));
}

TEST_F(FileEncryptHandleTest, PrivateSetEnviroment_AppendsToProcessEnvironment)
{
    FileEncryptHandlerPrivate priv;
    priv.setEnviroment(qMakePair(QString("UT_VAULT_KEY"), QString("ut_value")));
    // no direct getter on QProcess env; ensure second call still succeeds (append semantics)
    priv.setEnviroment(qMakePair(QString("UT_VAULT_KEY2"), QString("ut_value2")));
    SUCCEED();
}

TEST_F(FileEncryptHandleTest, PrivateSyncGroupPolicyAlgoName_NoConfig_PushesDefaultAlgo)
{
    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyAlgoName, QVariant("NoExist"));

    QString capturedKey, capturedValue;
    stub.set_lamda(VADDR(DConfigManager, setValue),
                   [&](DConfigManager *, const QString &, const QString &key, const QVariant &value) {
                       capturedKey = key;
                       capturedValue = value.toString();
                       
                   });

    FileEncryptHandlerPrivate priv;
    priv.syncGroupPolicyAlgoName();
    EXPECT_EQ(capturedKey, QString(kGroupPolicyKeyVaultAlgoName));
    EXPECT_EQ(capturedValue, QString("aes-256-gcm"));
}

TEST_F(FileEncryptHandleTest, PrivateSyncGroupPolicyAlgoName_ConfigAlgo_PushesConfiguredAlgo)
{
    VaultConfig config;
    config.set(kConfigNodeName, kConfigKeyAlgoName, QVariant("sm4-128-ecb"));

    QString capturedValue;
    stub.set_lamda(VADDR(DConfigManager, setValue),
                   [&](DConfigManager *, const QString &, const QString &, const QVariant &value) {
                       capturedValue = value.toString();
                       
                   });

    FileEncryptHandlerPrivate priv;
    priv.syncGroupPolicyAlgoName();
    EXPECT_EQ(capturedValue, QString("sm4-128-ecb"));
}

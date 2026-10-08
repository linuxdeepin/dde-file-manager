// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// PMS sev-2 regression tests for FileEncryptHandle / FileEncryptHandlerPrivate.
// Covers: BUG-172877 (cryfs >=0.10 needs --allow-replaced-filesystem),
// BUG-310085 (unmount always via fusermount, never cryfs-unmount),
// BUG-278715 (state() canonicalizes unlock path before fsTypeFromUrl),
// BUG-338697 (state re-check bypasses stale cached state),
// BUG-284441 (setEnviroment appends instead of replacing process env),
// BUG-345499 (unlockVault no longer creates the unlock dir in worker thread),
// BUG-347021 (key-mode createVault must use the master key as cryfs password).
// All QProcess spawning is stubbed out; no real cryfs/fusermount is executed.

#include "utils/fileencrypthandle.h"
#include "utils/fileencrypthandle_p.h"

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include "stubext.h"

#include "utils/pathmanager.h"
#include "utils/encryption/vaultconfig.h"
#include "utils/encryption/operatorcenter.h"
#include "utils/encryption/masterkeymanager.h"
#include "utils/encryption/passwordmanager.h"
#include "utils/vaultdefine.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-io/dfmio_utils.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

namespace {
const QString kConfigPath = kVaultBasePath + "/" + QString(kVaultConfigFileName);
const QString kContainerPath = kVaultBasePath + "/password_container.bin";

using StartFunc = void (QProcess::*)(const QString &, const QStringList &, QIODevice::OpenMode);
using WaitFunc = bool (QProcess::*)(int);
using WriteFunc = qint64 (QProcess::*)(const QByteArray &);
using RunVault3 = int (FileEncryptHandlerPrivate::*)(QString, QString, QString);
using RunVault5 = int (FileEncryptHandlerPrivate::*)(QString, QString, QString, EncryptType, int);
}   // namespace

class FileEncryptHandleRegressionTest : public testing::Test
{
protected:
    void SetUp() override
    {
        QDir().mkpath(kVaultBasePath);
        if (QFile::exists(kConfigPath))
            QFile::copy(kConfigPath, kConfigPath + ".utbak");
        handle = FileEncryptHandle::instance();
        d = handle->d;
        handle->updateState(VaultState::kUnknow);
    }

    void TearDown() override
    {
        stub.clear();
        handle->updateState(VaultState::kUnknow);
        QFile::remove(kContainerPath);
        QFile::remove(kConfigPath);
        if (QFile::exists(kConfigPath + ".utbak")) {
            QFile::remove(kConfigPath);
            QFile::rename(kConfigPath + ".utbak", kConfigPath);
        }
    }

    // stub out all QProcess side effects so no real child process is spawned
    void stubProcessSpawning(QString *capturedProg, QStringList *capturedArgs)
    {
        stub.set_lamda(static_cast<StartFunc>(&QProcess::start),
                       [capturedProg, capturedArgs](QProcess *, const QString &prog, const QStringList &args, QIODevice::OpenMode) {
                           *capturedProg = prog;
                           *capturedArgs = args;
                       });
        stub.set_lamda(static_cast<WaitFunc>(&QProcess::waitForStarted),
                       [](QProcess *, int) { return true; });
        stub.set_lamda(static_cast<WaitFunc>(&QProcess::waitForFinished),
                       [](QProcess *, int) { return true; });
        // NOTE: do NOT stub QProcess::waitForBytesWritten - it is virtual (QIODevice
        // override) and cpp-stub cannot patch a vtable-backed member pointer; on a
        // never-started QProcess it returns false immediately anyway.
        stub.set_lamda(static_cast<WriteFunc>(&QProcess::write),
                       [](QProcess *, const QByteArray &) -> qint64 { return 4; });
        stub.set_lamda(&QProcess::closeWriteChannel, [](QProcess *) {});
        stub.set_lamda(&QProcess::terminate, [](QProcess *) {});
    }

    stub_ext::StubExt stub;
    FileEncryptHandle *handle = nullptr;
    FileEncryptHandlerPrivate *d = nullptr;
};

// PMS:172877 cryfs >= 0.10 requires --allow-replaced-filesystem, older versions must not get it
TEST_F(FileEncryptHandleRegressionTest, BUG172877_RunVaultAddsAllowReplacedFilesystemByVersion)
{
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &bin, const QStringList &) -> QString {
                       return bin == "fusermount" ? QString() : QString("/usr/bin/") + bin;
                   });
    QString capturedProg;
    QStringList capturedArgs;
    stubProcessSpawning(&capturedProg, &capturedArgs);

    // new cryfs: flag must be the first argument
    stub.set_lamda(&FileEncryptHandlerPrivate::versionString,
                   [](FileEncryptHandlerPrivate *) {
                       return FileEncryptHandlerPrivate::CryfsVersionInfo(0, 11, 0);
                   });
    FileEncryptHandlerPrivate priv;
    EXPECT_EQ(priv.runVaultProcess("/ut_lock", "/ut_unlock", "pwd"), 0);
    EXPECT_FALSE(capturedArgs.isEmpty());
    EXPECT_TRUE(capturedArgs.contains(QString("--allow-replaced-filesystem")));
    EXPECT_EQ(capturedProg, QString("/usr/bin/cryfs"));
    ASSERT_GE(capturedArgs.size(), 3);
    EXPECT_EQ(capturedArgs.last(), QString("/ut_unlock"));
    EXPECT_EQ(capturedArgs.at(capturedArgs.size() - 2), QString("/ut_lock"));

    // old cryfs (< 0.10): the flag must not be passed
    stub.set_lamda(&FileEncryptHandlerPrivate::versionString,
                   [](FileEncryptHandlerPrivate *) {
                       return FileEncryptHandlerPrivate::CryfsVersionInfo(0, 9, 9);
                   });
    EXPECT_EQ(priv.runVaultProcess("/ut_lock", "/ut_unlock", "pwd"), 0);
    EXPECT_FALSE(capturedArgs.contains(QString("--allow-replaced-filesystem")));

    // 5-arg (create) variant honours the same rule
    stub.set_lamda(&FileEncryptHandlerPrivate::versionString,
                   [](FileEncryptHandlerPrivate *) {
                       return FileEncryptHandlerPrivate::CryfsVersionInfo(0, 11, 0);
                   });
    EXPECT_EQ(priv.runVaultProcess("/ut_lock", "/ut_unlock", "pwd", EncryptType::AES_256_GCM, 32), 0);
    EXPECT_TRUE(capturedArgs.contains(QString("--allow-replaced-filesystem")));

    // 172877 also fixed makeVaultLocalPath returning an empty path
    EXPECT_FALSE(PathManager::makeVaultLocalPath("utdir", "utfile").isEmpty());
}

// PMS:310085 unmount must always go through fusermount (-u / -zu), never cryfs-unmount
TEST_F(FileEncryptHandleRegressionTest, BUG310085_LockVaultAlwaysUsesFusermountArgs)
{
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &bin, const QStringList &) -> QString {
                       return QString("/usr/bin/") + bin;
                   });
    QString capturedProg;
    QStringList capturedArgs;
    stubProcessSpawning(&capturedProg, &capturedArgs);

    FileEncryptHandlerPrivate priv;
    EXPECT_EQ(priv.lockVaultProcess("/ut_unlock", false), 0);
    EXPECT_EQ(capturedProg, QString("/usr/bin/fusermount"));
    EXPECT_EQ(capturedArgs, QStringList({ "-u", "/ut_unlock" }));

    EXPECT_EQ(priv.lockVaultProcess("/ut_unlock", true), 0);
    EXPECT_EQ(capturedProg, QString("/usr/bin/fusermount"));
    EXPECT_EQ(capturedArgs, QStringList({ "-zu", "/ut_unlock" }));
    EXPECT_NE(capturedProg, QString("/usr/bin/cryfs-unmount"));
}

// PMS:278715 state() must canonicalize the unlock path (symlinks) before fsTypeFromUrl
TEST_F(FileEncryptHandleRegressionTest, BUG278715_StateUsesCanonicalUnlockPath)
{
    QTemporaryDir realDir;
    ASSERT_TRUE(realDir.isValid());
    QTemporaryDir stateDir;
    ASSERT_TRUE(stateDir.isValid());
    QFile cfg(stateDir.path() + "/cryfs.config");
    ASSERT_TRUE(cfg.open(QIODevice::WriteOnly));
    cfg.close();

    QString linkPath = stateDir.path() + "/ut_unlock_link";
    ASSERT_TRUE(QFile::link(realDir.path(), linkPath));

    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return "/usr/bin/cryfs"; });
    stub.set_lamda(&PathManager::vaultUnlockPath,
                   [linkPath]() -> QString { return linkPath; });

    QUrl capturedUrl;
    stub.set_lamda(&DFMIO::DFMUtils::fsTypeFromUrl,
                   [&capturedUrl](const QUrl &url) -> QString {
                       capturedUrl = url;
                       return "fuse.cryfs";
                   });

    EXPECT_EQ(handle->state(stateDir.path()), VaultState::kUnlocked);
    // the fsType query must target the canonical (real) path, not the symlink
    QString canonical = QFileInfo(realDir.path()).canonicalFilePath();
    EXPECT_FALSE(canonical.isEmpty());
    EXPECT_EQ(capturedUrl.toLocalFile(), canonical);
    EXPECT_NE(capturedUrl.toLocalFile(), linkPath);
}

// PMS:338697 recovery path must re-check the real state instead of trusting stale cache
TEST_F(FileEncryptHandleRegressionTest, BUG338697_RecheckBypassesStaleCachedState)
{
    // simulate a stale "unlocked" state left in the cache
    ASSERT_TRUE(handle->updateState(VaultState::kUnlocked));

    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());

    // cached read keeps reporting the stale state
    EXPECT_EQ(handle->state(tmp.path(), true), VaultState::kUnlocked);

    // forced re-check with cryfs missing must report the real state (not available)
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });
    EXPECT_EQ(handle->state(tmp.path(), false), VaultState::kNotAvailable);
}

// PMS:284441 setEnviroment must append to the process environment, not replace it
TEST_F(FileEncryptHandleRegressionTest, BUG284441_SetEnviromentKeepsSystemEnvironment)
{
    FileEncryptHandlerPrivate priv;
    priv.setEnviroment(qMakePair(QString("CRYFS_FRONTEND"), QString("noninteractive")));

    QStringList env = priv.process->environment();
    EXPECT_TRUE(env.contains(QString("CRYFS_FRONTEND=noninteractive")));
    // the system environment (e.g. PATH) must survive the custom entry
    bool hasPath = false;
    for (const QString &entry : env) {
        if (entry.startsWith(QString("PATH=")))
            hasPath = true;
    }
    EXPECT_TRUE(hasPath);
}

// PMS:345499 unlockVault runs in a worker thread and must not create the unlock dir
TEST_F(FileEncryptHandleRegressionTest, BUG345499_UnlockVaultDoesNotCreateUnlockDir)
{
    QTemporaryDir lockDir;
    ASSERT_TRUE(lockDir.isValid());
    QString unlockDir = lockDir.path() + "/ut_unlock_not_created";
    ASSERT_FALSE(QDir(unlockDir).exists());

    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });
    stub.set_lamda(ADDR(OperatorCenter, isNewVaultVersion),
                   [](OperatorCenter *) -> bool { return true; });
    stub.set_lamda(&MasterKeyManager::getContainerPath,
                   []() -> QString { return kContainerPath; });

    // the unlock flow may fail on any missing-container branch, but it must NOT
    // create the unlock dir from the worker thread anymore (pre-fix behaviour)
    handle->unlockVault(lockDir.path(), unlockDir, "ut-pwd");
    EXPECT_FALSE(QDir(unlockDir).exists());
}

// PMS:347021 key-mode createVault must feed the master key (not the user password) to cryfs
TEST_F(FileEncryptHandleRegressionTest, BUG347021_CreateVaultKeyModeUsesMasterKeyForCryfs)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString lockDir = tmp.path() + "/ut_lock";
    QString unlockDir = tmp.path() + "/ut_unlock";

    // choose the key-encryption method so createVault takes the master key branch
    VaultConfig cfg;
    cfg.set(kConfigNodeName, kConfigKeyEncryptionMethod, QVariant(QString(kConfigValueMethodKey)));

    stub.set_lamda(VADDR(DConfigManager, setValue),
                   [](DConfigManager *, const QString &, const QString &, const QVariant &) {});
    stub.set_lamda(&QStandardPaths::findExecutable,
                   [](const QString &, const QStringList &) -> QString { return QString(); });

    stub.set_lamda(&MasterKeyManager::generateMasterKey,
                   []() -> QByteArray { return QByteArray(64, 'k'); });
    stub.set_lamda(&MasterKeyManager::getContainerPath,
                   []() -> QString { return kContainerPath; });
    stub.set_lamda(&PasswordManager::createPasswordContainerFile,
                   [](const char *) -> int { return 0; });
    stub.set_lamda(&PasswordManager::createLuksContainer,
                   [](const char *, const char *, size_t, const char *, int &slotID) -> int {
                       slotID = 1;
                       return 0;
                   });
    stub.set_lamda(&PasswordManager::addNewPassword,
                   [](const char *, const char *, const char *, int &newKeySlotId) -> int {
                       newKeySlotId = 2;
                       return 0;
                   });
    stub.set_lamda(ADDR(OperatorCenter, getRecoveryKey),
                   [](OperatorCenter *) -> QString { return QString(32, 'R'); });

    QString capturedCryfsPwd;
    stub.set_lamda(static_cast<RunVault5>(&FileEncryptHandlerPrivate::runVaultProcess),
                   [&capturedCryfsPwd](FileEncryptHandlerPrivate *, QString, QString, QString cryfsPwd, EncryptType, int) -> int {
                       capturedCryfsPwd = cryfsPwd;
                       return 0;
                   });

    QSignalSpy spy(handle, &FileEncryptHandle::signalCreateVault);
    ASSERT_TRUE(spy.isValid());

    const QString kUserPassword = QStringLiteral("ut-user-password");
    handle->createVault(lockDir, unlockDir, kUserPassword, EncryptType::AES_256_GCM, 32);

    // cryfs got the 64-byte master key, not the user password
    EXPECT_EQ(capturedCryfsPwd, QString(64, 'k'));
    EXPECT_NE(capturedCryfsPwd, kUserPassword);

    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toInt(), 0);

    // success path marks the vault as newly created
    VaultConfig verify;
    EXPECT_EQ(verify.getVaultCreationType(), QString(kConfigValueVaultCreationTypeNew));
}

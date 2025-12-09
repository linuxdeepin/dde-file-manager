// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/vaultupgradeunit.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QList>
#include <QMap>
#include <QStorageInfo>
#include <QProcess>
#include <QSqlDatabase>

using namespace dfm_upgrade;

class TestVaultUpgradeUnit : public testing::Test {
protected:
    void SetUp() override {
        testDir = QDir::tempPath() + "/vault_test";
        QDir().mkpath(testDir);

        oldVaultDir = testDir + "/.local/share/applications/vault_encrypted";
        newVaultDir = testDir + "/.config/Vault/vault_encrypted";

        // Create test vault directories and files
        QDir().mkpath(oldVaultDir);
        QDir().mkpath(newVaultDir);

        // Mock QStandardPaths::standardLocations to use test directory
        auto standlocal = static_cast<QList<QString> (*)(QStandardPaths::StandardLocation)>(&QStandardPaths::standardLocations);
        stub.set_lamda(standlocal, [&](QStandardPaths::StandardLocation type) -> QList<QString> {
            if (type == QStandardPaths::ConfigLocation || type == QStandardPaths::HomeLocation) {
                return {testDir};
            }
            return QStandardPaths::standardLocations(type);
        });

        // Mock QDir::homePath
        auto homePathFunc = static_cast<QString (*)()>(&QDir::homePath);
        stub.set_lamda(homePathFunc, [&]() -> QString {
            qDebug() << "Test vault upgrade mock home path" << testDir;
            return testDir;
        });
    }

    void TearDown() override {
        QDir(testDir).removeRecursively();
        stub.clear();
    }

    QString testDir;
    QString oldVaultDir;
    QString newVaultDir;
    stub_ext::StubExt stub;
};

TEST_F(TestVaultUpgradeUnit, name)
{
    VaultUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("VaultUpgradeUnit"));
}

TEST_F(TestVaultUpgradeUnit, initialize_NoOldVault)
{
    VaultUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_FALSE(result);
}

// Add tests for VaultUpgradeUnit methods
TEST_F(TestVaultUpgradeUnit, initialize_WithOldVault)
{
    // Create old vault config file to trigger initialization success
    QString oldConfigPath = oldVaultDir + "/cryfs.config";
    QFile oldConfig(oldConfigPath);
    if (oldConfig.open(QIODevice::WriteOnly)) {
        oldConfig.write("test config content");
        oldConfig.close();
    }

    // Ensure new config doesn't exist
    QString newConfigPath = newVaultDir + "/cryfs.config";
    QFile::remove(newConfigPath);

    VaultUpgradeUnit unit;
    QMap<QString, QString> args;
    bool result = unit.initialize(args);
    EXPECT_TRUE(result);
}

TEST_F(TestVaultUpgradeUnit, upgrade)
{
    // Mock QProcess methods
    auto processStart = static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODeviceBase::OpenMode)>(&QProcess::start);
    stub.set_lamda(processStart, [](QProcess *process, const QString &program, const QStringList &arguments, QIODeviceBase::OpenMode mode) {
        Q_UNUSED(process);
        Q_UNUSED(program);
        Q_UNUSED(arguments);
        Q_UNUSED(mode);
        // Do nothing in test
    });

    auto processWaitForStarted = static_cast<bool (QProcess::*)(int)>(&QProcess::waitForStarted);
    stub.set_lamda(processWaitForStarted, [](QProcess *process, int msecs) -> bool {
        Q_UNUSED(process);
        Q_UNUSED(msecs);
        return true;  // Return true to indicate process started successfully
    });

    auto processWaitForFinished = static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished);
    stub.set_lamda(processWaitForFinished, [](QProcess *process, int msecs) -> bool {
        Q_UNUSED(process);
        Q_UNUSED(msecs);
        return true;  // Return true to indicate process finished successfully
    });

    auto processTerminate = static_cast<void (QProcess::*)()>(&QProcess::terminate);
    stub.set_lamda(processTerminate, [](QProcess *process) {
        Q_UNUSED(process);
        // Do nothing in test
    });

    // Create old vault config file to trigger initialization success
    QString oldConfigPath = oldVaultDir + "/cryfs.config";
    QFile oldConfig(oldConfigPath);
    if (oldConfig.open(QIODevice::WriteOnly)) {
        oldConfig.write("test config content");
        oldConfig.close();
    }

    // Ensure new vault directory doesn't exist
    auto vaultDir = testDir + "/.config/Vault";
    QDir(vaultDir).removeRecursively();

    VaultUpgradeUnit unit;
    QMap<QString, QString> args;
    bool initResult = unit.initialize(args);
    if (initResult) {
        bool result = unit.upgrade();
        // Since we're mocking, the result should be true if all operations succeed
        EXPECT_TRUE(result);
    } else {
        // SUCCEED();  // Expected in test environment where old vault doesn't exist
        EXPECT_TRUE(initResult);
    }
}

TEST_F(TestVaultUpgradeUnit, completed)
{
    VaultUpgradeUnit unit;
    unit.completed();  // Should not crash or throw
    SUCCEED();  // Just verify that the method can be called without issues
}

// Test VaultUpgradeUnit::isLockState method
TEST_F(TestVaultUpgradeUnit, isLockState)
{
    QString testPath = testDir + "/test_mount";
    QDir().mkpath(testPath);

    // Mock QStorageInfo methods
    auto storageInfoIsValid = static_cast<bool (QStorageInfo::*)() const>(&QStorageInfo::isValid);
    stub.set_lamda(storageInfoIsValid, [](QStorageInfo *info) -> bool {
        Q_UNUSED(info);
        return true; // Return true to simulate valid storage
    });

    auto storageInfoFileSystemType = static_cast<QByteArray (QStorageInfo::*)() const>(&QStorageInfo::fileSystemType);
    stub.set_lamda(storageInfoFileSystemType, [](QStorageInfo *info) -> QByteArray {
        Q_UNUSED(info);
        return QByteArray("fuse.cryfs");  // Return fuse.cryfs to simulate vault mount
    });

    VaultUpgradeUnit unit;
    bool result = unit.isLockState(testPath);
    EXPECT_TRUE(result);
}

// Test VaultUpgradeUnit::isLockState method with non-fuse filesystem
TEST_F(TestVaultUpgradeUnit, isLockState_NotFuse)
{
    QString testPath = testDir + "/test_mount";
    QDir().mkpath(testPath);

    // Mock QStorageInfo methods to simulate non-fuse filesystem
    auto storageInfoIsValid = static_cast<bool (QStorageInfo::*)() const>(&QStorageInfo::isValid);
    stub.set_lamda(storageInfoIsValid, [](QStorageInfo *info) -> bool {
        Q_UNUSED(info);
        return true; // Return true to simulate valid storage
    });

    auto storageInfoFileSystemType = static_cast<QByteArray (QStorageInfo::*)() const>(&QStorageInfo::fileSystemType);
    stub.set_lamda(storageInfoFileSystemType, [](QStorageInfo *info) -> QByteArray {
        Q_UNUSED(info);
        return QByteArray("ext4");  // Return different fs type to simulate non-vault mount
    });

    VaultUpgradeUnit unit;
    bool result = unit.isLockState(testPath);
    EXPECT_FALSE(result);
}

// Test VaultUpgradeUnit::lockVault method
TEST_F(TestVaultUpgradeUnit, lockVault)
{
    QString testPath = testDir + "/test_mount";
    QDir().mkpath(testPath);

    // Mock QStandardPaths::findExecutable to simulate fusermount exists
    auto findExecutableFunc = static_cast<QString (*)(const QString &, const QStringList &)>(&QStandardPaths::findExecutable);
    stub.set_lamda(findExecutableFunc, [&](const QString &command, const QStringList &paths) -> QString {
        Q_UNUSED(paths);
        if (command == "fusermount") {
            return "/usr/bin/fusermount";  // Return a valid path to simulate fusermount exists
        }
        return QString();  // Return empty string for other commands
    });

    // Mock QProcess methods
    auto processStart = static_cast<void (QProcess::*)(const QString &, const QStringList &, QIODeviceBase::OpenMode)>(&QProcess::start);
    stub.set_lamda(processStart, [](QProcess *process, const QString &program, const QStringList &arguments, QIODeviceBase::OpenMode mode) {
        Q_UNUSED(process);
        Q_UNUSED(program);
        Q_UNUSED(arguments);
        Q_UNUSED(mode);
        // Do nothing in test
    });

    auto processWaitForStarted = static_cast<bool (QProcess::*)(int)>(&QProcess::waitForStarted);
    stub.set_lamda(processWaitForStarted, [](QProcess *process, int msecs) -> bool {
        Q_UNUSED(process);
        Q_UNUSED(msecs);
        return true;  // Return true to indicate process started successfully
    });

    auto processWaitForFinished = static_cast<bool (QProcess::*)(int)>(&QProcess::waitForFinished);
    stub.set_lamda(processWaitForFinished, [](QProcess *process, int msecs) -> bool {
        Q_UNUSED(process);
        Q_UNUSED(msecs);
        return true;  // Return true to indicate process finished successfully
    });

    auto processTerminate = static_cast<void (QProcess::*)()>(&QProcess::terminate);
    stub.set_lamda(processTerminate, [](QProcess *process) {
        Q_UNUSED(process);
        // Do nothing in test
    });

    // Mock isLockState to return false after lock operation
    auto isLockStateFunc = static_cast<bool (VaultUpgradeUnit::*)(const QString &)>(&VaultUpgradeUnit::isLockState);
    stub.set_lamda(isLockStateFunc, [](VaultUpgradeUnit *unit, const QString &mountPath) -> bool {
        Q_UNUSED(unit);
        Q_UNUSED(mountPath);
        return false; // Return false to simulate successful lock
    });

    VaultUpgradeUnit unit;
    bool result = unit.lockVault(testPath);
    EXPECT_TRUE(result);
}

// Test VaultUpgradeUnit::lockVault method when fusermount not found
TEST_F(TestVaultUpgradeUnit, lockVault_FusermountNotFound)
{
    QString testPath = testDir + "/test_mount";
    QDir().mkpath(testPath);

    // Mock QStandardPaths::findExecutable to simulate fusermount not found
    auto findExecutableFunc = static_cast<QString (*)(const QString &, const QStringList &)>(&QStandardPaths::findExecutable);
    stub.set_lamda(findExecutableFunc, [&](const QString &command, const QStringList &paths) -> QString {
        Q_UNUSED(command);
        Q_UNUSED(paths);
        return QString();  // Return empty string to simulate fusermount not found
    });

    VaultUpgradeUnit unit;
    bool result = unit.lockVault(testPath);
    EXPECT_FALSE(result);  // Should return false when fusermount is not found
}
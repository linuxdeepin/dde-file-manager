// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/fileencrypthandle.h"
#include "utils/fileencrypthandle_p.h"
#include "utils/encryption/vaultconfig.h"

#include <gtest/gtest.h>

#include <DDialog>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QProcess>
#include <QDir>
#include <QStandardPaths>

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/dialogmanager.h>

#include <dfm-io/dfm-io/dfmio_utils.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
USING_IO_NAMESPACE

TEST(UT_FileEncryptHandle, init)
{
    stub_ext::StubExt stub;
    typedef bool(QDBusConnection::*FuncType2)(const QString &, const QString &, const QString &, const QString &, const QString &, QObject *, const char *);
    stub.set_lamda(static_cast<FuncType2>(&QDBusConnection::connect), []{
        return true;
    });

    FileEncryptHandle::instance();
}

TEST(UT_FileEncryptHandle, connectLockScreenToUpdateVaultState_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QDBusConnection::isConnected, [ &isOk ]{
        isOk = true;
        return false;
    });

    FileEncryptHandle::instance()->connectLockScreenToUpdateVaultState();

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, connectLockScreenToUpdateVaultState_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QDBusConnection::isConnected, []{
        return true;
    });
    stub.set_lamda(&QDBusConnectionInterface::isServiceRegistered, [ &isOk ]{
        isOk = true;
        QDBusMessage me;
        return QDBusReply<bool>(me);
    });

    FileEncryptHandle::instance()->connectLockScreenToUpdateVaultState();

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, createVault_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandle::createDirIfNotExist, [ &isOk ]{
        isOk = true;
        return false;
    });

    FileEncryptHandle::instance()->createVault("/UT_TEST1", "/UT_TEST2", "TU_PASSWORD");

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, createVault_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandle::createDirIfNotExist, []{
        return true;
    });
    stub.set_lamda(&DConfigManager::setValue, []{});
    stub.set_lamda(&VaultConfig::set, []{});
    typedef void(QProcess::*FuncType)(const QString &, const QStringList &, QIODevice::OpenMode);
    stub.set_lamda(static_cast<FuncType>(&QProcess::start), [ &isOk ]{
        isOk = true;
    });

    FileEncryptHandle::instance()->createVault("/UT_TEST1", "/UT_TEST2", "TU_PASSWORD");

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, createVault_three)
{
    bool isOk { false };
    FileEncryptHandle *handle = FileEncryptHandle::instance();

    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandle::createDirIfNotExist, []{
        return true;
    });
    stub.set_lamda(&DConfigManager::setValue, []{});
    stub.set_lamda(&VaultConfig::set, []{});
    typedef int(FileEncryptHandlerPrivate::*FuncType)(QString, QString, QString, EncryptType, int);
    stub.set_lamda(static_cast<FuncType>(&FileEncryptHandlerPrivate::runVaultProcess), [ &handle, &isOk ]{
        isOk = true;
        handle->d->activeState.insert(1, static_cast<int>(ErrorCode::kUnspecifiedError));
        return 0;
    });

    FileEncryptHandle::instance()->createVault("/UT_TEST1", "/UT_TEST2", "TU_PASSWORD");

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, unlockVault_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandle::createDirIfNotExist, []{
        return false;
    });
    stub.set_lamda(&DialogManager::showErrorDialog, []{});

    bool isOk = FileEncryptHandle::instance()->unlockVault("/UT_TEST1", "UT_TEST2", "UT_PASSWORD");

    EXPECT_FALSE(isOk);
}

TEST(UT_FileEncryptHandle, unlockVault_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandle::createDirIfNotExist, []{
        return true;
    });
    stub.set_lamda(&FileEncryptHandlerPrivate::syncGroupPolicyAlgoName, []{});
    typedef void(QProcess::*FuncType)(const QString &, const QStringList &, QIODevice::OpenMode);
    stub.set_lamda(static_cast<FuncType>(&QProcess::start), [ &isOk ]{
        isOk = true;
    });

    FileEncryptHandle::instance()->unlockVault("/UT_TEST1", "UT_TEST2", "UT_PASSWORD");

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, unlockVault_three)
{
    FileEncryptHandle *handle = FileEncryptHandle::instance();

    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandle::createDirIfNotExist, []{
        return true;
    });
    stub.set_lamda(&FileEncryptHandlerPrivate::syncGroupPolicyAlgoName, []{});
    typedef int(FileEncryptHandlerPrivate::*FuncType)(QString, QString, QString);
    stub.set_lamda(static_cast<FuncType>(&FileEncryptHandlerPrivate::runVaultProcess), [ &handle ]{
        handle->d->activeState.insert(3, static_cast<int>(ErrorCode::kUnspecifiedError));
        return 0;
    });

    bool isOk = FileEncryptHandle::instance()->unlockVault("/UT_TEST1", "UT_TEST2", "UT_PASSWORD");

    EXPECT_FALSE(isOk);
}

TEST(UT_FileEncryptHandle, lockVault_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef void(QProcess::*FuncType)(const QString &, const QStringList &, QIODevice::OpenMode);
    stub.set_lamda(static_cast<FuncType>(&QProcess::start), [ &isOk ]{
        isOk = true;
    });

    FileEncryptHandle::instance()->lockVault("/UT_TEST1", false);

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, lockVault_two)
{
    bool isOk { false };
    FileEncryptHandle *handle = FileEncryptHandle::instance();

    stub_ext::StubExt stub;
    stub.set_lamda(&FileEncryptHandlerPrivate::lockVaultProcess, [ &handle, &isOk ]{
        isOk = true;
        handle->d->activeState.insert(7, static_cast<int>(ErrorCode::kUnspecifiedError));
        return 0;
    });

    FileEncryptHandle::instance()->lockVault("/UT_TEST1", false);

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, createDirIfNotExist_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QDir::mkpath, []{
        return true;
    });

    bool isOk = FileEncryptHandle::instance()->createDirIfNotExist("/UT_TEST");

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, createDirIfNotExist_two)
{
    stub_ext::StubExt stub;
    typedef  bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), []{
        return true;
    });
    stub.set_lamda(&QDir::isEmpty, []{
        return true;
    });

    bool isOk = FileEncryptHandle::instance()->createDirIfNotExist("/UT_TEST");

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, createDirIfNotExist_three)
{
    stub_ext::StubExt stub;
    typedef  bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), []{
        return true;
    });
    stub.set_lamda(&QDir::isEmpty, []{
        return false;
    });

    bool isOk = FileEncryptHandle::instance()->createDirIfNotExist("/UT_TEST");

    EXPECT_FALSE(isOk);
}

TEST(UT_FileEncryptHandle, state_one)
{
    VaultState state = FileEncryptHandle::instance()->state("");

    EXPECT_TRUE(state == VaultState::kUnknow);
}

TEST(UT_FileEncryptHandle, state_two)
{
    FileEncryptHandle::instance()->d->curState = kUnlocked;
    VaultState state = FileEncryptHandle::instance()->state("/UT_TEST");

    EXPECT_TRUE(state == kUnlocked);
}

TEST(UT_FileEncryptHandle, state_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QStandardPaths::findExecutable, []{
        return "";
    });

    FileEncryptHandle::instance()->d->curState = kUnknow;
    VaultState state = FileEncryptHandle::instance()->state("/UT_TEST");

    EXPECT_TRUE(state == kNotAvailable);
}

TEST(UT_FileEncryptHandle, state_four)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QStandardPaths::findExecutable, []{
        return "/UT_TEST";
    });

    FileEncryptHandle::instance()->d->curState = kUnknow;
    VaultState state = FileEncryptHandle::instance()->state("/UT_TEST");

    EXPECT_TRUE(state == kNotExisted);
}

TEST(UT_FileEncryptHandle, state_five)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QStandardPaths::findExecutable, []{
        return "/UT_TEST/";
    });
    typedef bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), []{
        return true;
    });
    stub.set_lamda(&DFMUtils::fsTypeFromUrl, []{
        return "fuse.cryfs";
    });

    FileEncryptHandle::instance()->d->curState = kUnknow;
    VaultState state = FileEncryptHandle::instance()->state("/UT_TEST");

    EXPECT_TRUE(state == kUnlocked);
}

TEST(UT_FileEncryptHandle, state_six)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QStandardPaths::findExecutable, []{
        return "/UT_TEST/";
    });
    typedef bool(*FuncType)(const QString &);
    stub.set_lamda(static_cast<FuncType>(&QFile::exists), []{
        return true;
    });
    stub.set_lamda(&DFMUtils::fsTypeFromUrl, []{
        return "UT_TEST";
    });

    FileEncryptHandle::instance()->d->curState = kUnknow;
    VaultState state = FileEncryptHandle::instance()->state("/UT_TEST");

    EXPECT_TRUE(state == kEncrypted);
}

TEST(UT_FileEncryptHandle, updateState_one)
{
    FileEncryptHandle::instance()->d->curState = kUnknow;
    bool isOk = FileEncryptHandle::instance()->updateState(kNotExisted);

    EXPECT_FALSE(isOk);
}

TEST(UT_FileEncryptHandle, updateState_two)
{
    FileEncryptHandle::instance()->updateState(kUnknow);

    EXPECT_TRUE(FileEncryptHandle::instance()->d->curState == kUnknow);
}

TEST(UT_FileEncryptHandle, encryptAlgoTypeOfGroupPolicy)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef void(QProcess::*FuncType)(const QString &, const QStringList &, QIODevice::OpenMode);
    stub.set_lamda(static_cast<FuncType>(&QProcess::start), [ &isOk ]{
        isOk = true;
    });

    FileEncryptHandle::instance()->encryptAlgoTypeOfGroupPolicy();

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, slotReadError_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QProcess::readAllStandardError, []{
        return QByteArray("mountpoint is not empty");
    });

    FileEncryptHandle::instance()->d->activeState.clear();
    FileEncryptHandle::instance()->d->activeState.insert(1, -1);
    FileEncryptHandle::instance()->slotReadError();

    EXPECT_TRUE(FileEncryptHandle::instance()->d->activeState[1] == static_cast<int>(ErrorCode::kMountpointNotEmpty));
}

TEST(UT_FileEncryptHandle, slotReadError_two)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QProcess::readAllStandardError, []{
        return QByteArray("Permission denied");
    });

    FileEncryptHandle::instance()->d->activeState.clear();
    FileEncryptHandle::instance()->d->activeState.insert(1, -1);
    FileEncryptHandle::instance()->slotReadError();

    EXPECT_TRUE(FileEncryptHandle::instance()->d->activeState[1] == static_cast<int>(ErrorCode::kPermissionDenied));
}

TEST(UT_FileEncryptHandle, slotReadError_three)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QProcess::readAllStandardError, []{
        return QByteArray("mountpoint is not empty");
    });

    FileEncryptHandle::instance()->d->activeState.clear();
    FileEncryptHandle::instance()->d->activeState.insert(3, -1);
    FileEncryptHandle::instance()->slotReadError();

    EXPECT_TRUE(FileEncryptHandle::instance()->d->activeState[3] == static_cast<int>(ErrorCode::kMountpointNotEmpty));
}

TEST(UT_FileEncryptHandle, slotReadError_four)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QProcess::readAllStandardError, []{
        return QByteArray("mountpoint is not empty");
    });

    FileEncryptHandle::instance()->d->activeState.clear();
    FileEncryptHandle::instance()->d->activeState.insert(3, -1);
    FileEncryptHandle::instance()->slotReadError();

    EXPECT_TRUE(FileEncryptHandle::instance()->d->activeState[3] == static_cast<int>(ErrorCode::kMountpointNotEmpty));
}

TEST(UT_FileEncryptHandle, slotReadError_five)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QProcess::readAllStandardError, []{
        return QByteArray("Permission denied");
    });

    FileEncryptHandle::instance()->d->activeState.clear();
    FileEncryptHandle::instance()->d->activeState.insert(3, -1);
    FileEncryptHandle::instance()->slotReadError();

    EXPECT_TRUE(FileEncryptHandle::instance()->d->activeState[3] == static_cast<int>(ErrorCode::kPermissionDenied));
}

TEST(UT_FileEncryptHandle, slotReadError_six)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&QProcess::readAllStandardError, []{
        return QByteArray("Device or resource busy");
    });
    stub.set_lamda(&DialogManager::showErrorDialog, []{});

    FileEncryptHandle::instance()->d->activeState.clear();
    FileEncryptHandle::instance()->d->activeState.insert(7, -1);
    FileEncryptHandle::instance()->slotReadError();

    EXPECT_TRUE(FileEncryptHandle::instance()->d->activeState[7] == static_cast<int>(ErrorCode::kResourceBusy));
}

TEST(UT_FileEncryptHandle, slotReadOutput)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QProcess::readAllStandardOutput, [ &isOk ]{
        isOk = true;
        return "";
    });
    stub.set_lamda(&DialogManager::showErrorDialog, []{});

    FileEncryptHandle::instance()->slotReadOutput();

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, responseLockScreenDBus)
{
    bool isOk {false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QDBusMessage::arguments, [ &isOk ]{
        isOk = true;
        return QList<QVariant> {QVariant(1)};
    });

    QDBusMessage msg;
    FileEncryptHandle::instance()->responseLockScreenDBus(msg);

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, syncGroupPolicyAlgoName_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef  QVariant(VaultConfig::*FuncType)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>(kConfigKeyNotExist);
    });
    stub.set_lamda(&DConfigManager::setValue, [ &isOk ]{
        isOk = true;
    });

    FileEncryptHandle::instance()->d->syncGroupPolicyAlgoName();

    EXPECT_TRUE(isOk);
}

TEST(UT_FileEncryptHandle, syncGroupPolicyAlgoName_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef  QVariant(VaultConfig::*FuncType)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType>(&VaultConfig::get), []{
        return QVariant::fromValue<QString>("UT_TEST");
    });
    stub.set_lamda(&DConfigManager::setValue, [ &isOk ]{
        isOk = true;
    });

    FileEncryptHandle::instance()->d->syncGroupPolicyAlgoName();

    EXPECT_TRUE(isOk);
}













// SPDX-FileCopyrightText: 2020 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/fileencrypthandle.h"
#include "utils/encryption/vaultconfig.h"

#include <gtest/gtest.h>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QProcess>

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

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

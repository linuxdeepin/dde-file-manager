// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/vaultautolock.h"
#include "utils/vaulthelper.h"
#include "dbus/vaultdbusutils.h"

#include <gtest/gtest.h>

#include <dfm-base/base/application/settings.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(UT_VaultAutoLock, getAutoLockState_one)
{
    stub_ext::StubExt stub;
    typedef  void(QTimer::*FuncType)();
    stub.set_lamda(static_cast<FuncType>(&QTimer::start), []{});
    typedef void(Settings::*FuncType1)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType1>(&Settings::setValue), []{});
    typedef QVariant(Settings::*FuncType2)(const QString &, const QString &, const QVariant &)const;
    stub.set_lamda(static_cast<FuncType2>(&Settings::value), []{
        return QVariant::fromValue<int>(static_cast<int>(VaultAutoLock::kNever));
    });

    VaultAutoLock::AutoLockState state = VaultAutoLock::instance()->getAutoLockState();

    EXPECT_TRUE(state == VaultAutoLock::kNever);
}

TEST(UT_VaultAutoLock, getAutoLockState_two)
{
    stub_ext::StubExt stub;
    typedef  void(QTimer::*FuncType)();
    stub.set_lamda(static_cast<FuncType>(&QTimer::start), []{});
    typedef void(Settings::*FuncType1)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType1>(&Settings::setValue), []{});
    typedef QVariant(Settings::*FuncType2)(const QString &, const QString &, const QVariant &)const;

    VaultAutoLock::instance()->autoLockState = VaultAutoLock::kFiveMinutes;
    VaultAutoLock::AutoLockState state = VaultAutoLock::instance()->getAutoLockState();

    EXPECT_TRUE(state == VaultAutoLock::kFiveMinutes);
}

TEST(UT_VaultAutoLock, autoLock)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultAutoLock::refreshAccessTime, []{});
    typedef  void(QTimer::*FuncType)();
    stub.set_lamda(static_cast<FuncType>(&QTimer::start), []{});
    typedef void(Settings::*FuncType1)(const QString &, const QString &, const QVariant &);
    stub.set_lamda(static_cast<FuncType1>(&Settings::setValue), []{});

    VaultAutoLock::instance()->autoLock(VaultAutoLock::kFiveMinutes);
}

TEST(UT_VaultAutoLock, isValid)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultDBusUtils::vaultManagerDBusCall, []{
        return QVariant::fromValue<QString>("UT_TEST");
    });

    bool isOk = VaultAutoLock::instance()->isValid();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultAutoLock, refreshAccessTime)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultAutoLock::isValid, []{
        return true;
    });
    stub.set_lamda(&VaultDBusUtils::vaultManagerDBusCall, []{
        return QVariant::fromValue<quint64>(123);
    });
    stub.set_lamda(&VaultAutoLock::dbusSetRefreshTime, [ &isOk ]{
        isOk = true;
    });

    VaultAutoLock::instance()->refreshAccessTime();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultAutoLock, resetConfig)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultAutoLock::autoLock, [ &isOk ]{
        isOk = true;
        return true;
    });

    VaultAutoLock::instance()->resetConfig();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultAutoLock, processAutoLock_one)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnknow;
    });

    VaultAutoLock::instance()->processAutoLock();
}

TEST(UT_VaultAutoLock, processAutoLock_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::state, []{
        return VaultState::kUnlocked;
    });
    stub.set_lamda(&VaultAutoLock::dbusGetLastestTime, []{
        return 0;
    });
    stub.set_lamda(&VaultAutoLock::dbusGetSelfTime, []{
        return (5*60+1);
    });
    stub.set_lamda(&VaultHelper::lockVault, [ &isOk ]{
        isOk = true;
    });

    VaultAutoLock::instance()->autoLockState = VaultAutoLock::kFiveMinutes;
    VaultAutoLock::instance()->processAutoLock();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultAutoLock, slotUnlockVault)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultAutoLock::autoLock, [ &isOk ]{
        isOk = true;
        return true;
    });

    VaultAutoLock::instance()->slotUnlockVault(0);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultAutoLock, slotLockVault)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&QTimer::stop, [ &isOk ]{
        isOk = true;
    });

    VaultAutoLock::instance()->slotLockVault(0);
    VaultAutoLock::instance()->slotLockVault(1);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultAutoLock, processLockEvent)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultHelper::lockVault, [ &isOk ]{
        isOk = true;
    });

    VaultAutoLock::instance()->processLockEvent();

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultAutoLock, dbusSetRefreshTime)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    stub.set_lamda(&VaultDBusUtils::vaultManagerDBusCall, [ &isOk ]{
        isOk = true;
        return QVariant::fromValue<quint64>(1);
    });

    VaultAutoLock::instance()->dbusSetRefreshTime(1);

    EXPECT_TRUE(isOk);
}

TEST(UT_VaultAutoLock, dbusGetLastestTime)
{
    stub_ext::StubExt stub;
    stub.set_lamda(&VaultDBusUtils::vaultManagerDBusCall, []{
        return QVariant::fromValue<quint64>(1);
    });

    quint64 time = VaultAutoLock::instance()->dbusGetLastestTime();

    EXPECT_TRUE(time == 1);
}

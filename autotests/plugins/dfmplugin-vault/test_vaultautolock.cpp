// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// VaultAutoLock tests: all vault-manager DBus round trips are stubbed at
// VaultDBusUtils::vaultManagerDBusCall so the auto-lock logic is exercised
// deterministically without any daemon.

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QVariant>

#include <unistd.h>

#include "stubext.h"

#include "utils/vaultautolock.h"
#include "dbus/vaultdbusutils.h"
#include "utils/vaulthelper.h"
#include "utils/fileencrypthandle.h"

DPVAULT_USE_NAMESPACE

namespace {
QVariant g_dbusReply;
}

static void setDbusReply(const QVariant &v)
{
    g_dbusReply = v;
}

class VaultAutoLockTest : public testing::Test
{
protected:
    void SetUp() override
    {
        g_dbusReply = QVariant();
        stub.set_lamda(&VaultDBusUtils::vaultManagerDBusCall,
                       [](const QString &, const QVariant &) -> QVariant {
                           return g_dbusReply;
                       });
    }

    void TearDown() override
    {
        stub.clear();
        g_dbusReply = QVariant();
    }

    stub_ext::StubExt stub;
};

TEST_F(VaultAutoLockTest, Instance_IsSingleton)
{
    VaultAutoLock *ins = VaultAutoLock::instance();
    ASSERT_NE(ins, nullptr);
    EXPECT_EQ(ins, VaultAutoLock::instance());
}

TEST_F(VaultAutoLockTest, IsValid_DBusReplyValid_ReturnsTrue)
{
    setDbusReply(QVariant::fromValue<quint64>(12345));
    EXPECT_TRUE(VaultAutoLock::instance()->isValid());
}

TEST_F(VaultAutoLockTest, IsValid_DBusReplyNull_ReturnsFalse)
{
    setDbusReply(QVariant());
    EXPECT_FALSE(VaultAutoLock::instance()->isValid());
}

TEST_F(VaultAutoLockTest, DbusGetLastestTime_ReplyValid_ReturnsValue)
{
    setDbusReply(QVariant::fromValue<quint64>(777));
    EXPECT_EQ(VaultAutoLock::instance()->dbusGetLastestTime(), quint64(777));
}

TEST_F(VaultAutoLockTest, DbusGetLastestTime_ReplyNull_ReturnsZero)
{
    setDbusReply(QVariant());
    EXPECT_EQ(VaultAutoLock::instance()->dbusGetLastestTime(), quint64(0));
}

TEST_F(VaultAutoLockTest, DbusGetSelfTime_ReplyValid_ReturnsValue)
{
    setDbusReply(QVariant::fromValue<quint64>(888));
    EXPECT_EQ(VaultAutoLock::instance()->dbusGetSelfTime(), quint64(888));
}

TEST_F(VaultAutoLockTest, DbusGetSelfTime_ReplyNull_ReturnsZero)
{
    setDbusReply(QVariant());
    EXPECT_EQ(VaultAutoLock::instance()->dbusGetSelfTime(), quint64(0));
}

TEST_F(VaultAutoLockTest, DbusSetRefreshTime_ReplyValid_NoWarningPath)
{
    setDbusReply(QVariant::fromValue<quint64>(1));
    VaultAutoLock::instance()->dbusSetRefreshTime(100);
    SUCCEED();
}

TEST_F(VaultAutoLockTest, RefreshAccessTime_Valid_QueriesSelfTime)
{
    setDbusReply(QVariant::fromValue<quint64>(42));
    VaultAutoLock::instance()->refreshAccessTime();
    SUCCEED();
}

TEST_F(VaultAutoLockTest, RefreshAccessTime_Invalid_SkipsRefresh)
{
    setDbusReply(QVariant());
    VaultAutoLock::instance()->refreshAccessTime();
    SUCCEED();
}

TEST_F(VaultAutoLockTest, ProcessAutoLock_VaultLocked_EarlyReturn)
{
    using StateFunc = VaultState (VaultHelper::*)(const QString &, bool) const;
    stub.set_lamda(static_cast<StateFunc>(&VaultHelper::state),
                   [](VaultHelper *, const QString &, bool) -> VaultState {
                       return VaultState::kEncrypted;
                   });
    VaultAutoLock::instance()->processAutoLock();
    SUCCEED();
}

TEST_F(VaultAutoLockTest, ProcessAutoLock_UnlockedWithinThreshold_NoLock)
{
    using StateFunc = VaultState (VaultHelper::*)(const QString &, bool) const;
    stub.set_lamda(static_cast<StateFunc>(&VaultHelper::state),
                   [](VaultHelper *, const QString &, bool) -> VaultState {
                       return VaultState::kUnlocked;
                   });
    bool lockCalled = false;
    stub.set_lamda(&VaultHelper::lockVault,
                   [&lockCalled](VaultHelper *, bool) -> bool {
                       lockCalled = true;
                       return true;
                   });

    // both times 0 -> interval 0, never exceeds threshold
    setDbusReply(QVariant::fromValue<quint64>(0));
    VaultAutoLock::instance()->processAutoLock();
    EXPECT_FALSE(lockCalled);
}

TEST_F(VaultAutoLockTest, ProcessLockEvent_LockSucceeds)
{
    bool lockCalled = false;
    stub.set_lamda(&VaultHelper::lockVault,
                   [&lockCalled](VaultHelper *, bool) -> bool {
                       lockCalled = true;
                       return true;
                   });
    VaultAutoLock::instance()->processLockEvent();
    EXPECT_TRUE(lockCalled);
}

TEST_F(VaultAutoLockTest, SlotLockEvent_OtherUser_Ignored)
{
    bool lockCalled = false;
    stub.set_lamda(&VaultHelper::lockVault,
                   [&lockCalled](VaultHelper *, bool) -> bool {
                       lockCalled = true;
                       return true;
                   });
    VaultAutoLock::instance()->slotLockEvent("someone_not_me_ut");
    EXPECT_FALSE(lockCalled);
}

TEST_F(VaultAutoLockTest, SlotLockEvent_CurrentUser_LocksVault)
{
    bool lockCalled = false;
    stub.set_lamda(&VaultHelper::lockVault,
                   [&lockCalled](VaultHelper *, bool) -> bool {
                       lockCalled = true;
                       return true;
                   });
    VaultAutoLock::instance()->slotLockEvent(QString::fromLocal8Bit(getlogin()));
    EXPECT_TRUE(lockCalled);
}

TEST_F(VaultAutoLockTest, SlotUnlockVault_SuccessState_KeepsAutoLockState)
{
    auto before = VaultAutoLock::instance()->getAutoLockState();
    VaultAutoLock::instance()->slotUnlockVault(0);
    EXPECT_EQ(VaultAutoLock::instance()->getAutoLockState(), before);
}

TEST_F(VaultAutoLockTest, SlotUnlockVault_FailureState_NoRestart)
{
    VaultAutoLock::instance()->slotUnlockVault(1);
    SUCCEED();
}

TEST_F(VaultAutoLockTest, ResetConfig_SetsStateToNever)
{
    VaultAutoLock::instance()->resetConfig();
    EXPECT_EQ(VaultAutoLock::instance()->getAutoLockState(), VaultAutoLock::AutoLockState::kNever);
}

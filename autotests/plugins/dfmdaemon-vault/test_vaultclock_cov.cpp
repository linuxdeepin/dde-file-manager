// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage map for src/plugins/daemon/vault/dbus/private/vaultclock.cpp:
//   VaultClock::VaultClock / ~VaultClock        -> Ctor_FreshState_AllZeroAndNoLockEvent (fixture lifecycle)
//   setRefreshTime / getLastestTime            -> RefreshTime_SetThenGet_RoundTrips
//   getSelfTime / addTickTime                  -> SelfTime_AddTickTime_AccumulatesSeconds
//   tick (protected slot)                      -> Tick_InvokedByName_IncrementsSelfTime
//   triggerLockEvent / isLockEventTriggered /
//   clearLockEvent                             -> LockEvent_TriggerThenClear_TogglesState
//   vaultBasePath (static)                     -> VaultBasePath_UnderHomeConfigVault_MatchesHomePath
// Branch list (all trivial setters/getters, no deep branches).

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "daemonplugin_vaultdaemon_global.h"
#include "dbus/private/vaultclock.h"

#include <QDir>
#include <QMetaObject>
#include <QTimer>

DAEMONPVAULT_USE_NAMESPACE

class UT_VaultClockCov : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override { stub.clear(); }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_VaultClockCov, Ctor_FreshState_AllZeroAndNoLockEvent)
{
    // Arrange
    VaultClock clock;

    // Act - values are read straight after construction
    quint64 lastest = clock.getLastestTime();
    quint64 self = clock.getSelfTime();
    bool locked = clock.isLockEventTriggered();

    // Assert
    EXPECT_EQ(lastest, quint64(0));
    EXPECT_EQ(self, quint64(0));
    EXPECT_FALSE(locked);
}

TEST_F(UT_VaultClockCov, RefreshTime_SetThenGet_RoundTrips)
{
    // Arrange
    VaultClock clock;
    const quint64 kExpected = 123456ULL;

    // Act
    clock.setRefreshTime(kExpected);

    // Assert
    EXPECT_EQ(clock.getLastestTime(), kExpected);
    EXPECT_EQ(clock.getSelfTime(), quint64(0));   // refresh time must not touch the self timer
}

TEST_F(UT_VaultClockCov, SelfTime_AddTickTime_AccumulatesSeconds)
{
    // Arrange
    VaultClock clock;

    // Act
    clock.addTickTime(10);
    clock.addTickTime(5);

    // Assert
    EXPECT_EQ(clock.getSelfTime(), quint64(15));
    EXPECT_EQ(clock.getLastestTime(), quint64(0));   // tick time must not touch the refresh time
}

TEST_F(UT_VaultClockCov, Tick_InvokedByName_IncrementsSelfTime)
{
    // Arrange
    VaultClock clock;

    // Act - tick() is a protected slot, invoke it through the meta object
    bool invoked = QMetaObject::invokeMethod(&clock, "tick");

    // Assert
    EXPECT_TRUE(invoked);
    EXPECT_EQ(clock.getSelfTime(), quint64(1));
}

TEST_F(UT_VaultClockCov, LockEvent_TriggerThenClear_TogglesState)
{
    // Arrange
    VaultClock clock;
    ASSERT_FALSE(clock.isLockEventTriggered());

    // Act
    clock.triggerLockEvent();
    bool afterTrigger = clock.isLockEventTriggered();
    clock.clearLockEvent();
    bool afterClear = clock.isLockEventTriggered();

    // Assert
    EXPECT_TRUE(afterTrigger);
    EXPECT_FALSE(afterClear);
    EXPECT_EQ(clock.getLastestTime(), quint64(0));   // lock events never touch the clock value
}

TEST_F(UT_VaultClockCov, VaultBasePath_UnderHomeConfigVault_MatchesHomePath)
{
    // Arrange
    QString expected = QDir::homePath() + QStringLiteral("/.config/Vault");

    // Act
    QString base = VaultClock::vaultBasePath();

    // Assert
    EXPECT_EQ(base, expected);
    EXPECT_TRUE(base.endsWith(QStringLiteral("Vault")));
}

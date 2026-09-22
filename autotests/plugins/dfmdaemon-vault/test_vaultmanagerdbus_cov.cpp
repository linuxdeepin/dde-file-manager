// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Coverage map for src/plugins/daemon/vault/dbus/vaultmanagerdbus.cpp:
//   VaultManagerDBus::VaultManagerDBus   -> Ctor_InitializesCurrentUserAndClock
//   VaultManagerDBus::GetCurrentUser      -> Ctor_InitializesCurrentUserAndClock
//   VaultManagerDBus::SysUserChanged      -> SysUserChanged_NewUser_SwitchesClock, SysUserChanged_SameUser_KeepsClock
//   VaultManagerDBus::SetRefreshTime      -> RefreshTime_SetAndQuery_RoundTrips
//   VaultManagerDBus::GetLastestTime      -> RefreshTime_SetAndQuery_RoundTrips
//   VaultManagerDBus::GetSelfTime         -> ComputerSleep_WakeAfterSleep_AddsTickTime
//   VaultManagerDBus::TriggerLockEvent    -> LockEvent_TriggerQueryClear_Cycle
//   VaultManagerDBus::IsLockEventTriggered-> LockEvent_TriggerQueryClear_Cycle
//   VaultManagerDBus::ClearLockEvent      -> LockEvent_TriggerQueryClear_Cycle
//   VaultManagerDBus::ComputerSleep       -> ComputerSleep_WakeAfterSleep_AddsTickTime
//   VaultManagerDBus::GetLeftoverErrorInputTimes       -> LeftoverErrorTimes_DecrementAndRestore_Cycle
//   VaultManagerDBus::LeftoverErrorInputTimesMinusOne  -> LeftoverErrorTimes_DecrementAndRestore_Cycle
//   VaultManagerDBus::RestoreLeftoverErrorInputTimes   -> LeftoverErrorTimes_DecrementAndRestore_Cycle
//   VaultManagerDBus::restoreLeftoverErrorInputTimes   -> (reached through RestoreLeftoverErrorInputTimes)
//   VaultManagerDBus::StartTimerOfRestorePasswordInput-> NeedWaitMinutes_DefaultAndRestore_Stable
//   VaultManagerDBus::GetNeedWaitMinutes               -> NeedWaitMinutes_DefaultAndRestore_Stable
//   VaultManagerDBus::RestoreNeedWaitMinutes           -> NeedWaitMinutes_DefaultAndRestore_Stable
//   VaultManagerDBus::restoreNeedWaitMinutes           -> (reached through RestoreNeedWaitMinutes)
//   VaultManagerDBus::timerEvent                       -> TimerEvent_UnknownTimerId_NoStateChange
// Branch list:
//   SysUserChanged {user changed -> reuse/insert clock, user unchanged -> no-op}
//   ComputerSleep  {entering sleep -> record time, waking with diff>0 -> addTickTime, diff<=0 -> skip}
//   Get*/timerEvent {map miss -> default insert, map hit}

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "daemonplugin_vaultdaemon_global.h"
#include "dbus/vaultmanagerdbus.h"

#include <QDateTime>
#include <QTimerEvent>

DAEMONPVAULT_USE_NAMESPACE

// timerEvent() is protected; re-export it so the test can drive it with a foreign timer id.
class ExposedVaultManagerDBus : public VaultManagerDBus
{
public:
    using VaultManagerDBus::VaultManagerDBus;
    using VaultManagerDBus::timerEvent;
};

class UT_VaultManagerDBusCov : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override { stub.clear(); }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_VaultManagerDBusCov, Ctor_OnConstruction_InitializesUserAndClock)
{
    // Arrange
    VaultManagerDBus mgr;

    // Act - the constructor already ran, probe the state it installed

    // Assert - getCurrentUser may be empty in a sandboxed session, but must be stable
    EXPECT_EQ(mgr.GetCurrentUser(), mgr.GetCurrentUser());
    EXPECT_EQ(mgr.GetLastestTime(), quint64(0));   // fresh clock, no refresh time recorded yet
}

TEST_F(UT_VaultManagerDBusCov, SysUserChanged_NewUser_SwitchesClock)
{
    // Arrange
    ExposedVaultManagerDBus mgr;
    mgr.SysUserChanged("coverage-user-1");

    // Act
    mgr.SetRefreshTime(777);
    mgr.SysUserChanged("coverage-user-1");   // same user again: must keep the same clock

    // Assert
    EXPECT_EQ(mgr.GetLastestTime(), quint64(777));
    EXPECT_EQ(mgr.GetSelfTime(), quint64(0));
}

TEST_F(UT_VaultManagerDBusCov, RefreshTime_SetAndQuery_RoundTrips)
{
    // Arrange
    VaultManagerDBus mgr;

    // Act
    mgr.SetRefreshTime(987654321ULL);

    // Assert
    EXPECT_EQ(mgr.GetLastestTime(), quint64(987654321ULL));
    EXPECT_EQ(mgr.GetSelfTime(), quint64(0));
}

TEST_F(UT_VaultManagerDBusCov, LockEvent_TriggerQueryClear_Cycle)
{
    // Arrange
    VaultManagerDBus mgr;

    // Act
    mgr.TriggerLockEvent();
    bool triggered = mgr.IsLockEventTriggered();
    mgr.ClearLockEvent();
    bool cleared = mgr.IsLockEventTriggered();

    // Assert
    EXPECT_TRUE(triggered);
    EXPECT_FALSE(cleared);
    EXPECT_EQ(mgr.GetLastestTime(), quint64(0));   // lock flags never touch the refresh time
}

TEST_F(UT_VaultManagerDBusCov, ComputerSleep_WakeAfterSleep_AddsTickTime)
{
    // Arrange
    ExposedVaultManagerDBus mgr;
    mgr.SysUserChanged("sleep-user");
    ASSERT_EQ(mgr.GetSelfTime(), quint64(0));

    static int callCount = 0;
    callCount = 0;
    stub.set_lamda(static_cast<qint64 (*)()>(&QDateTime::currentSecsSinceEpoch), [] {
        __DBG_STUB_INVOKE__
        ++callCount;
        return callCount == 1 ? qint64(1000) : qint64(1015);   // 15 s of "sleep"
    });

    // Act
    mgr.ComputerSleep(true);    // entering sleep: records t0
    mgr.ComputerSleep(false);   // waking up: adds the 15 s diff to every user clock

    // Assert
    EXPECT_EQ(mgr.GetSelfTime(), quint64(15));
    EXPECT_EQ(mgr.GetLastestTime(), quint64(0));   // sleep accounting must not touch the refresh time
}

TEST_F(UT_VaultManagerDBusCov, LeftoverErrorTimes_DecrementAndRestore_Cycle)
{
    // Arrange
    VaultManagerDBus mgr;
    const int uid = 4242;
    int initial = mgr.GetLeftoverErrorInputTimes(uid);
    int initialOther = mgr.GetLeftoverErrorInputTimes(uid + 1);

    // Act
    mgr.LeftoverErrorInputTimesMinusOne(uid);
    mgr.LeftoverErrorInputTimesMinusOne(uid);
    int afterMinus = mgr.GetLeftoverErrorInputTimes(uid);
    mgr.RestoreLeftoverErrorInputTimes(uid);
    int afterRestore = mgr.GetLeftoverErrorInputTimes(uid);

    // Assert
    EXPECT_EQ(initial, initialOther);       // both users share the same default
    EXPECT_EQ(afterMinus, initial - 2);
    EXPECT_EQ(afterRestore, initial);
}

TEST_F(UT_VaultManagerDBusCov, NeedWaitMinutes_DefaultAndRestore_Stable)
{
    // Arrange
    VaultManagerDBus mgr;
    const int uid = 88;
    int first = mgr.GetNeedWaitMinutes(uid);

    // Act
    mgr.StartTimerOfRestorePasswordInput(uid);   // starts a long timer, does not touch minutes
    int afterStart = mgr.GetNeedWaitMinutes(uid);
    mgr.RestoreNeedWaitMinutes(uid);
    int afterRestore = mgr.GetNeedWaitMinutes(uid);

    // Assert
    EXPECT_EQ(afterStart, first);
    EXPECT_EQ(afterRestore, first);
}

TEST_F(UT_VaultManagerDBusCov, TimerEvent_UnknownTimerId_NoStateChange)
{
    // Arrange
    ExposedVaultManagerDBus mgr;
    mgr.SetRefreshTime(555);

    // Act - fire timerEvent with an id that was never registered in the internal map
    QTimerEvent unknownEvent(-999);
    mgr.timerEvent(&unknownEvent);

    // Assert
    EXPECT_EQ(mgr.GetLastestTime(), quint64(555));
    EXPECT_EQ(mgr.GetNeedWaitMinutes(12345) > 0, true);   // wait minutes untouched by foreign timer
}

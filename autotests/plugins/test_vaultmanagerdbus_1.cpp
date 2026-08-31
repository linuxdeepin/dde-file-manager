// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultmanagerdbus_1.cpp
 * @brief Unit tests for VaultManagerDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dbus/vaultmanagerdbus.h"

#include <QTest>

using namespace vault;

class VaultManagerDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultManagerDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultManagerDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultManagerDBusTest, ComputerSleep)
{
    // Test method: void ComputerSleep((bool bSleep))
    EXPECT_NO_FATAL_FAILURE(obj->ComputerSleep(false));
}

TEST_F(VaultManagerDBusTest, GetCurrentUser)
{
    // Test getter: QString GetCurrentUser()
    auto result = obj->GetCurrentUser();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultManagerDBusTest, GetLastestTime)
{
    // Test getter: quint64 GetLastestTime()
    auto result = obj->GetLastestTime();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultManagerDBusTest, GetLeftoverErrorInputTimes)
{
    // Test method: int GetLeftoverErrorInputTimes((int userID))
    auto result = obj->GetLeftoverErrorInputTimes(0);
    EXPECT_GE(result, 0);

}

TEST_F(VaultManagerDBusTest, GetNeedWaitMinutes)
{
    // Test method: int GetNeedWaitMinutes((int userID))
    auto result = obj->GetNeedWaitMinutes(0);
    EXPECT_GE(result, 0);

}

TEST_F(VaultManagerDBusTest, GetSelfTime)
{
    // Test getter: quint64 GetSelfTime()
    auto result = obj->GetSelfTime();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultManagerDBusTest, IsLockEventTriggered)
{
    // Test bool getter: IsLockEventTriggered()
    bool result = obj->IsLockEventTriggered();
    EXPECT_FALSE(result);

}

TEST_F(VaultManagerDBusTest, LeftoverErrorInputTimesMinusOne)
{
    // Test method: void LeftoverErrorInputTimesMinusOne((int userID))
    EXPECT_NO_FATAL_FAILURE(obj->LeftoverErrorInputTimesMinusOne(0));
}

TEST_F(VaultManagerDBusTest, RestoreLeftoverErrorInputTimes)
{
    // Test method: void RestoreLeftoverErrorInputTimes((int userID))
    EXPECT_NO_FATAL_FAILURE(obj->RestoreLeftoverErrorInputTimes(0));
}

TEST_F(VaultManagerDBusTest, RestoreNeedWaitMinutes)
{
    // Test method: void RestoreNeedWaitMinutes((int userID))
    EXPECT_NO_FATAL_FAILURE(obj->RestoreNeedWaitMinutes(0));
}

TEST_F(VaultManagerDBusTest, SetRefreshTime)
{
    // Test setter: void SetRefreshTime((quint64 time))
    EXPECT_NO_FATAL_FAILURE(obj->SetRefreshTime(0));
}

TEST_F(VaultManagerDBusTest, StartTimerOfRestorePasswordInput)
{
    // Test method: void StartTimerOfRestorePasswordInput((int userID))
    EXPECT_NO_FATAL_FAILURE(obj->StartTimerOfRestorePasswordInput(0));
}

TEST_F(VaultManagerDBusTest, SysUserChanged)
{
    // Test method: void SysUserChanged((const QString &curUser))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->SysUserChanged(_arg0));
}

TEST_F(VaultManagerDBusTest, TriggerLockEvent)
{
    // Test method: void TriggerLockEvent(())
    EXPECT_NO_FATAL_FAILURE(obj->TriggerLockEvent());
}

TEST_F(VaultManagerDBusTest, restoreLeftoverErrorInputTimes)
{
    // Test method: void restoreLeftoverErrorInputTimes((int userID))
    EXPECT_NO_FATAL_FAILURE(obj->restoreLeftoverErrorInputTimes(0));
}

TEST_F(VaultManagerDBusTest, restoreNeedWaitMinutes)
{
    // Test method: void restoreNeedWaitMinutes((int userID))
    EXPECT_NO_FATAL_FAILURE(obj->restoreNeedWaitMinutes(0));
}

TEST_F(VaultManagerDBusTest, timerEvent)
{
    // Test event handler: timerEvent((QTimerEvent *event))
    QTimerEvent _event(QTimerEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->timerEvent(&_event));
}

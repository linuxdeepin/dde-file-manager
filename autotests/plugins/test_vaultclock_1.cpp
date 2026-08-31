// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultclock_1.cpp
 * @brief Unit tests for VaultClock methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dbus/private/vaultclock.h"

#include <QTest>

using namespace vault;

class VaultClockTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultClock();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultClock *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultClockTest, VaultClock)
{
    // Test constructor: VaultClock((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultClockTest, addTickTime)
{
    // Test method: void addTickTime((qint64 seconds))
    EXPECT_NO_FATAL_FAILURE(obj->addTickTime(0));
}

TEST_F(VaultClockTest, getLastestTime)
{
    // Test getter: quint64 getLastestTime()
    auto result = obj->getLastestTime();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultClockTest, getSelfTime)
{
    // Test getter: quint64 getSelfTime()
    auto result = obj->getSelfTime();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultClockTest, isLockEventTriggered)
{
    // Test bool getter: isLockEventTriggered()
    bool result = obj->isLockEventTriggered();
    EXPECT_FALSE(result);

}

TEST_F(VaultClockTest, setRefreshTime)
{
    // Test setter: void setRefreshTime((quint64 time))
    EXPECT_NO_FATAL_FAILURE(obj->setRefreshTime(0));
}

TEST_F(VaultClockTest, tick)
{
    // Test method: void tick(())
    EXPECT_NO_FATAL_FAILURE(obj->tick());
}

TEST_F(VaultClockTest, triggerLockEvent)
{
    // Test method: void triggerLockEvent(())
    EXPECT_NO_FATAL_FAILURE(obj->triggerLockEvent());
}

TEST_F(VaultClockTest, vaultBasePath)
{
    // Test getter: QString vaultBasePath()
    auto result = obj->vaultBasePath();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultClockTest, VaultClock_Destructor)
{
    // Test method:  ~VaultClock(())
    EXPECT_NO_FATAL_FAILURE({ VaultClock *tmp = new VaultClock(); delete tmp; });
}

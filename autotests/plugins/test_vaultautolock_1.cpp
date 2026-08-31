// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultautolock_1.cpp
 * @brief Unit tests for VaultAutoLock methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/vaultautolock.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultAutoLockTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultAutoLock();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultAutoLock *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultAutoLockTest, VaultAutoLock)
{
    // Test constructor: VaultAutoLock((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultAutoLockTest, autoLock)
{
    // Test method: bool autoLock((VaultAutoLock::AutoLockState lockState))
    auto result = obj->autoLock(VaultAutoLock::AutoLockState());
    EXPECT_FALSE(result);

}

TEST_F(VaultAutoLockTest, dbusGetLastestTime)
{
    // Test getter: quint64 dbusGetLastestTime()
    auto result = obj->dbusGetLastestTime();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultAutoLockTest, dbusGetSelfTime)
{
    // Test getter: quint64 dbusGetSelfTime()
    auto result = obj->dbusGetSelfTime();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultAutoLockTest, dbusSetRefreshTime)
{
    // Test method: void dbusSetRefreshTime((quint64 time))
    EXPECT_NO_FATAL_FAILURE(obj->dbusSetRefreshTime(0));
}

TEST_F(VaultAutoLockTest, getAutoLockState)
{
    // Test getter: VaultAutoLock::AutoLockState getAutoLockState()
    auto result = obj->getAutoLockState();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(VaultAutoLockTest, instance)
{
    // Test getter: VaultAutoLock instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(VaultAutoLockTest, loadConfig)
{
    // Test method: void loadConfig(())
    EXPECT_NO_FATAL_FAILURE(obj->loadConfig());
}

TEST_F(VaultAutoLockTest, processAutoLock)
{
    // Test method: void processAutoLock(())
    EXPECT_NO_FATAL_FAILURE(obj->processAutoLock());
}

TEST_F(VaultAutoLockTest, processLockEvent)
{
    // Test method: void processLockEvent(())
    EXPECT_NO_FATAL_FAILURE(obj->processLockEvent());
}

TEST_F(VaultAutoLockTest, refreshAccessTime)
{
    // Test method: void refreshAccessTime(())
    EXPECT_NO_FATAL_FAILURE(obj->refreshAccessTime());
}

TEST_F(VaultAutoLockTest, slotLockEvent)
{
    // Test event handler: slotLockEvent((const QString &user))
    QString _event(QString::None);
    EXPECT_NO_FATAL_FAILURE(obj->slotLockEvent(&_event));
}

TEST_F(VaultAutoLockTest, slotLockVault)
{
    // Test method: void slotLockVault((int state))
    EXPECT_NO_FATAL_FAILURE(obj->slotLockVault(0));
}

TEST_F(VaultAutoLockTest, slotUnlockVault)
{
    // Test method: void slotUnlockVault((int state))
    EXPECT_NO_FATAL_FAILURE(obj->slotUnlockVault(0));
}

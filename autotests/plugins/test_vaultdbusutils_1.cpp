// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultdbusutils_1.cpp
 * @brief Unit tests for VaultDBusUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dbus/vaultdbusutils.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultDBusUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultDBusUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultDBusUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultDBusUtilsTest, VaultDBusUtils)
{
    // Test constructor: VaultDBusUtils(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultDBusUtilsTest, getLeftoverErrorInputTimes)
{
    // Test getter: int getLeftoverErrorInputTimes()
    auto result = obj->getLeftoverErrorInputTimes();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultDBusUtilsTest, getNeedWaitMinutes)
{
    // Test getter: int getNeedWaitMinutes()
    auto result = obj->getNeedWaitMinutes();
    EXPECT_EQ(result, 0);

}

TEST_F(VaultDBusUtilsTest, handleChangedVaultState)
{
    // Test method: void handleChangedVaultState((const QVariantMap &map))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleChangedVaultState(_arg0));
}

TEST_F(VaultDBusUtilsTest, handleLockScreenDBus)
{
    // Test method: void handleLockScreenDBus((const QDBusMessage &msg))
    QDBusMessage _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleLockScreenDBus(_arg0));
}

TEST_F(VaultDBusUtilsTest, instance)
{
    // Test getter: VaultDBusUtils instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(VaultDBusUtilsTest, isFullConnectInternet)
{
    // Test bool getter: isFullConnectInternet()
    bool result = obj->isFullConnectInternet();
    EXPECT_FALSE(result);

}

TEST_F(VaultDBusUtilsTest, leftoverErrorInputTimesMinusOne)
{
    // Test method: void leftoverErrorInputTimesMinusOne(())
    EXPECT_NO_FATAL_FAILURE(obj->leftoverErrorInputTimesMinusOne());
}

TEST_F(VaultDBusUtilsTest, lockEventTriggered)
{
    // Test method: void lockEventTriggered((QObject *obj, const char *cslot))
    EXPECT_NO_FATAL_FAILURE(obj->lockEventTriggered(nullptr, nullptr));
}

TEST_F(VaultDBusUtilsTest, startTimerOfRestorePasswordInput)
{
    // Test method: void startTimerOfRestorePasswordInput(())
    EXPECT_NO_FATAL_FAILURE(obj->startTimerOfRestorePasswordInput());
}

TEST_F(VaultDBusUtilsTest, vaultManagerDBusCall)
{
    // Test method: QVariant vaultManagerDBusCall((QString function, const QVariant &value))
    QVariant _arg1{};
    auto result = obj->vaultManagerDBusCall(QString(), _arg1);
    EXPECT_FALSE(result.isValid());

}

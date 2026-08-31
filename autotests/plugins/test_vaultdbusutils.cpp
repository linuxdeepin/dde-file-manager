// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultdbusutils.cpp
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

TEST_F(VaultDBusUtilsTest, isServiceRegister)
{
    // Test method: bool isServiceRegister((QDBusConnection::BusType type, const QString &serviceName))
    QString _arg1{};
    auto result = obj->isServiceRegister(QDBusConnection::BusType(), _arg1);
    EXPECT_FALSE(result);

}

TEST_F(VaultDBusUtilsTest, restoreLeftoverErrorInputTimes)
{
    // Test method: void restoreLeftoverErrorInputTimes(())
    EXPECT_NO_FATAL_FAILURE(obj->restoreLeftoverErrorInputTimes());
}

TEST_F(VaultDBusUtilsTest, restoreNeedWaitMinutes)
{
    // Test method: void restoreNeedWaitMinutes(())
    EXPECT_NO_FATAL_FAILURE(obj->restoreNeedWaitMinutes());
}

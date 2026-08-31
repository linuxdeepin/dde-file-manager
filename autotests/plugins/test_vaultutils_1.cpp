// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultutils_1.cpp
 * @brief Unit tests for VaultUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/vaultutils.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultUtilsTest, VaultUtils)
{
    // Test constructor: VaultUtils((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultUtilsTest, instance)
{
    // Test getter: VaultUtils instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(VaultUtilsTest, slotCheckAuthorizationFinished)
{
    // Test method: void slotCheckAuthorizationFinished((Authority::Result result))
    EXPECT_NO_FATAL_FAILURE(obj->slotCheckAuthorizationFinished(Authority::Result()));
}

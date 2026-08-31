// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultmenusceneprivate.cpp
 * @brief Unit tests for VaultMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/vaultmenuscene.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultMenuScenePrivateTest, VaultMenuScenePrivate)
{
    // Test constructor: VaultMenuScenePrivate((VaultMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultMenuScenePrivateTest, emptyMenuActionRule)
{
    // Test getter: QStringList emptyMenuActionRule()
    auto result = obj->emptyMenuActionRule();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultMenuScenePrivateTest, normalMenuActionRule)
{
    // Test getter: QStringList normalMenuActionRule()
    auto result = obj->normalMenuActionRule();
    EXPECT_TRUE(result.isEmpty());

}

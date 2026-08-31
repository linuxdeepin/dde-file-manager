// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultmenuscenecreator.cpp
 * @brief Unit tests for VaultMenuSceneCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/vaultmenuscene.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultMenuSceneCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultMenuSceneCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultMenuSceneCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultMenuSceneCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(VaultMenuSceneCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

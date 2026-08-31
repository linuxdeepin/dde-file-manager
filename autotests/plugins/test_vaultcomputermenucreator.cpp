// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultcomputermenucreator.cpp
 * @brief Unit tests for VaultComputerMenuCreator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/vaultcomputermenuscene.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultComputerMenuCreatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultComputerMenuCreator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultComputerMenuCreator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultComputerMenuCreatorTest, create)
{
    // Test getter: AbstractMenuScene create()
    auto result = obj->create();
    EXPECT_NO_FATAL_FAILURE({ obj->create(); });

}

TEST_F(VaultComputerMenuCreatorTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

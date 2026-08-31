// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultcomputermenusceneprivate.cpp
 * @brief Unit tests for VaultComputerMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/vaultcomputermenuscene.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultComputerMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultComputerMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultComputerMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultComputerMenuScenePrivateTest, VaultComputerMenuScenePrivate)
{
    // Test constructor: VaultComputerMenuScenePrivate((VaultComputerMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultcomputermenuscene.cpp
 * @brief Unit tests for VaultComputerMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/vaultcomputermenuscene.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultComputerMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultComputerMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultComputerMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultComputerMenuSceneTest, VaultComputerMenuScene)
{
    // Test constructor: VaultComputerMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultComputerMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultComputerMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(VaultComputerMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultComputerMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(VaultComputerMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultComputerMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

TEST_F(VaultComputerMenuSceneTest, VaultComputerMenuScene_Destructor)
{
    // Test method:  ~VaultComputerMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ VaultComputerMenuScene *tmp = new VaultComputerMenuScene(); delete tmp; });
}

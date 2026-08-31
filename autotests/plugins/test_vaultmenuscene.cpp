// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vaultmenuscene.cpp
 * @brief Unit tests for VaultMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/vaultmenuscene.h"

#include <QTest>

using namespace dfmplugin_vault;

class VaultMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VaultMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VaultMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VaultMenuSceneTest, VaultMenuScene)
{
    // Test constructor: VaultMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VaultMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(VaultMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(VaultMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(VaultMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(VaultMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

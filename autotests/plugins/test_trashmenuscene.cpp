// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashmenuscene.cpp
 * @brief Unit tests for TrashMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/trashmenuscene.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashMenuSceneTest, TrashMenuScene)
{
    // Test constructor: TrashMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TrashMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(TrashMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TrashMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(TrashMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

TEST_F(TrashMenuSceneTest, TrashMenuScene_Destructor)
{
    // Test method:  ~TrashMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ TrashMenuScene *tmp = new TrashMenuScene(); delete tmp; });
}

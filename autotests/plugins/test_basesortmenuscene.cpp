// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basesortmenuscene.cpp
 * @brief Unit tests for BaseSortMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/basesortmenuscene.h"

#include <QTest>

using namespace dfmplugin_workspace;

class BaseSortMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BaseSortMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BaseSortMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BaseSortMenuSceneTest, BaseSortMenuScene)
{
    // Test constructor: BaseSortMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BaseSortMenuSceneTest, create)
{
    // Test method: bool create((QMenu *parent))
    auto result = obj->create(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(BaseSortMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(BaseSortMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseSortMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(BaseSortMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(BaseSortMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

TEST_F(BaseSortMenuSceneTest, BaseSortMenuScene_Destructor)
{
    // Test method:  ~BaseSortMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ BaseSortMenuScene *tmp = new BaseSortMenuScene(); delete tmp; });
}

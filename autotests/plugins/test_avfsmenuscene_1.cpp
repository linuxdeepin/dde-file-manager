// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsmenuscene_1.cpp
 * @brief Unit tests for AvfsMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu/avfsmenuscene.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsMenuSceneTest, AvfsMenuScene)
{
    // Test constructor: AvfsMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AvfsMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(AvfsMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(AvfsMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(AvfsMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(AvfsMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

TEST_F(AvfsMenuSceneTest, AvfsMenuScene_Destructor)
{
    // Test method:  ~AvfsMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ AvfsMenuScene *tmp = new AvfsMenuScene(); delete tmp; });
}

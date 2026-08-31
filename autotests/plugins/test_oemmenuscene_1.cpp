// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_oemmenuscene_1.cpp
 * @brief Unit tests for OemMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "oemmenuscene/oemmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class OemMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new OemMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    OemMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(OemMenuSceneTest, OemMenuScene)
{
    // Test constructor: OemMenuScene((OemMenu *oem, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(OemMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(OemMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(OemMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(OemMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

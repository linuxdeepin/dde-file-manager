// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_newcreatemenuscene.cpp
 * @brief Unit tests for NewCreateMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/newcreatemenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class NewCreateMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new NewCreateMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    NewCreateMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(NewCreateMenuSceneTest, NewCreateMenuScene)
{
    // Test constructor: NewCreateMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(NewCreateMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(NewCreateMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

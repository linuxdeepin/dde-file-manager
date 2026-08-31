// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_actioniconmenuscene_1.cpp
 * @brief Unit tests for ActionIconMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/actioniconmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ActionIconMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ActionIconMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ActionIconMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ActionIconMenuSceneTest, ActionIconMenuScene)
{
    // Test constructor: ActionIconMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ActionIconMenuSceneTest, actionIconVisible)
{
    // Test bool getter: actionIconVisible()
    bool result = obj->actionIconVisible();
    EXPECT_FALSE(result);

}

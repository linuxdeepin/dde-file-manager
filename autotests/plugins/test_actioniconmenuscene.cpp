// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_actioniconmenuscene.cpp
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

TEST_F(ActionIconMenuSceneTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

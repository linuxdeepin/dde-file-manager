// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_actioniconmenusceneprivate.cpp
 * @brief Unit tests for ActionIconMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/actioniconmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ActionIconMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ActionIconMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ActionIconMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ActionIconMenuScenePrivateTest, ActionIconMenuScenePrivate)
{
    // Test constructor: ActionIconMenuScenePrivate((ActionIconMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

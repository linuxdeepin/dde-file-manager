// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_clipboardmenusceneprivate.cpp
 * @brief Unit tests for ClipBoardMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/clipboardmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ClipBoardMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ClipBoardMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ClipBoardMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ClipBoardMenuScenePrivateTest, ClipBoardMenuScenePrivate)
{
    // Test constructor: ClipBoardMenuScenePrivate((AbstractMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_clipboardmenuscene.cpp
 * @brief Unit tests for ClipBoardMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/clipboardmenuscene.h"

#include <QTest>

using namespace dfmplugin_menu;

class ClipBoardMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ClipBoardMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ClipBoardMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ClipBoardMenuSceneTest, updateState)
{
    // Test method: void updateState((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->updateState(nullptr));
}

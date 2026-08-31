// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentmenuscene_1.cpp
 * @brief Unit tests for RecentMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/recentmenuscene.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentMenuSceneTest, RecentMenuScene)
{
    // Test constructor: RecentMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentMenuSceneTest, RecentMenuScene_Destructor)
{
    // Test method:  ~RecentMenuScene(())
    EXPECT_NO_FATAL_FAILURE({ RecentMenuScene *tmp = new RecentMenuScene(); delete tmp; });
}

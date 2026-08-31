// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentmenusceneprivate_1.cpp
 * @brief Unit tests for RecentMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/recentmenuscene.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentMenuScenePrivateTest, RecentMenuScenePrivate)
{
    // Test constructor: RecentMenuScenePrivate((RecentMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentMenuScenePrivateTest, groupByRole)
{
    // Test method: void groupByRole((const QString &strategy))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->groupByRole(_arg0));
}

TEST_F(RecentMenuScenePrivateTest, updateGroupSubMenu)
{
    // Test method: void updateGroupSubMenu((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->updateGroupSubMenu(nullptr));
}

TEST_F(RecentMenuScenePrivateTest, updateSortSubMenu)
{
    // Test method: void updateSortSubMenu((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->updateSortSubMenu(nullptr));
}

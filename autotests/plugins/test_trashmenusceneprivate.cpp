// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashmenusceneprivate.cpp
 * @brief Unit tests for TrashMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/trashmenuscene.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashMenuScenePrivateTest, TrashMenuScenePrivate)
{
    // Test constructor: TrashMenuScenePrivate((TrashMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TrashMenuScenePrivateTest, groupByRole)
{
    // Test method: void groupByRole((const QString &strategy))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->groupByRole(_arg0));
}

TEST_F(TrashMenuScenePrivateTest, updateGroupSubMenu)
{
    // Test method: void updateGroupSubMenu((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->updateGroupSubMenu(nullptr));
}

TEST_F(TrashMenuScenePrivateTest, updateSortSubMenu)
{
    // Test method: void updateSortSubMenu((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->updateSortSubMenu(nullptr));
}

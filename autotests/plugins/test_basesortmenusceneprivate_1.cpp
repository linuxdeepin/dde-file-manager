// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basesortmenusceneprivate_1.cpp
 * @brief Unit tests for BaseSortMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/basesortmenuscene.h"

#include <QTest>

using namespace dfmplugin_workspace;

class BaseSortMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BaseSortMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BaseSortMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BaseSortMenuScenePrivateTest, BaseSortMenuScenePrivate)
{
    // Test constructor: BaseSortMenuScenePrivate((BaseSortMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(BaseSortMenuScenePrivateTest, sendToRule)
{
    // Test getter: QStringList sendToRule()
    auto result = obj->sendToRule();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseSortMenuScenePrivateTest, sortPrimaryMenu)
{
    // Test method: void sortPrimaryMenu((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->sortPrimaryMenu(nullptr));
}

TEST_F(BaseSortMenuScenePrivateTest, stageToRule)
{
    // Test getter: QStringList stageToRule()
    auto result = obj->stageToRule();
    EXPECT_TRUE(result.isEmpty());

}

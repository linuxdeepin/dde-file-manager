// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_basesortmenusceneprivate.cpp
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

TEST_F(BaseSortMenuScenePrivateTest, primaryMenuRule)
{
    // Test getter: QStringList primaryMenuRule()
    auto result = obj->primaryMenuRule();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseSortMenuScenePrivateTest, secondaryMenuRule)
{
    // Test getter: QMap<QString, QStringList> secondaryMenuRule()
    auto result = obj->secondaryMenuRule();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BaseSortMenuScenePrivateTest, sortSecondaryMenu)
{
    // Test method: void sortSecondaryMenu((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->sortSecondaryMenu(nullptr));
}

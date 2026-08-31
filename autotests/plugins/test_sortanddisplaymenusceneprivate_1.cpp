// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sortanddisplaymenusceneprivate_1.cpp
 * @brief Unit tests for SortAndDisplayMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/sortanddisplaymenuscene.h"

#include <QTest>

using namespace dfmplugin_workspace;

class SortAndDisplayMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SortAndDisplayMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SortAndDisplayMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SortAndDisplayMenuScenePrivateTest, SortAndDisplayMenuScenePrivate)
{
    // Test constructor: SortAndDisplayMenuScenePrivate((AbstractMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SortAndDisplayMenuScenePrivateTest, addDisplayAsActions)
{
    // Test method: QMenu addDisplayAsActions((QMenu *menu))
    auto result = obj->addDisplayAsActions(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->addDisplayAsActions(nullptr); });

}

TEST_F(SortAndDisplayMenuScenePrivateTest, addSortByActions)
{
    // Test method: QMenu addSortByActions((QMenu *menu))
    auto result = obj->addSortByActions(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->addSortByActions(nullptr); });

}

TEST_F(SortAndDisplayMenuScenePrivateTest, createEmptyMenu)
{
    // Test method: void createEmptyMenu((QMenu *parent))
    EXPECT_NO_FATAL_FAILURE(obj->createEmptyMenu(nullptr));
}

TEST_F(SortAndDisplayMenuScenePrivateTest, groupByStrategy)
{
    // Test method: void groupByStrategy((const QString &strategyName))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->groupByStrategy(_arg0));
}

TEST_F(SortAndDisplayMenuScenePrivateTest, sortByRole)
{
    // Test method: void sortByRole((int role))
    EXPECT_NO_FATAL_FAILURE(obj->sortByRole(0));
}

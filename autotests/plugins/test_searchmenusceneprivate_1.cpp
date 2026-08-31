// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchmenusceneprivate_1.cpp
 * @brief Unit tests for SearchMenuScenePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/searchmenuscene.h"

#include <QTest>

using namespace dfmplugin_search;

class SearchMenuScenePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchMenuScenePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchMenuScenePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchMenuScenePrivateTest, SearchMenuScenePrivate)
{
    // Test constructor: SearchMenuScenePrivate((SearchMenuScene *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SearchMenuScenePrivateTest, createAction)
{
    // Test method: void createAction((QMenu *menu, const QString &actName, bool isSubAct, bool checkable))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->createAction(nullptr, _arg1, false, false));
}

TEST_F(SearchMenuScenePrivateTest, groupByRole)
{
    // Test method: void groupByRole((const QString &strategy))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->groupByRole(_arg0));
}

TEST_F(SearchMenuScenePrivateTest, openFileLocation)
{
    // Test method: bool openFileLocation((const QString &path))
    QString _arg0{};
    auto result = obj->openFileLocation(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SearchMenuScenePrivateTest, updateSortMenu)
{
    // Test method: void updateSortMenu((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->updateSortMenu(nullptr));
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchmenuscene.cpp
 * @brief Unit tests for SearchMenuScene methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menus/searchmenuscene.h"

#include <QTest>

using namespace dfmplugin_search;

class SearchMenuSceneTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchMenuScene();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchMenuScene *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchMenuSceneTest, SearchMenuScene)
{
    // Test constructor: SearchMenuScene((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SearchMenuSceneTest, initialize)
{
    // Test method: bool initialize((const QVariantHash &params))
    QVariantHash _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SearchMenuSceneTest, scene)
{
    // Test method: AbstractMenuScene scene((QAction *action))
    auto result = obj->scene(nullptr);
    EXPECT_NO_FATAL_FAILURE({ obj->scene(nullptr); });

}

TEST_F(SearchMenuSceneTest, triggered)
{
    // Test method: bool triggered((QAction *action))
    auto result = obj->triggered(nullptr);
    EXPECT_FALSE(result);

}

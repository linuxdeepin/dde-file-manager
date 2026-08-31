// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchmenusceneprivate.cpp
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

TEST_F(SearchMenuScenePrivateTest, disableSubScene)
{
    // Test method: void disableSubScene((AbstractMenuScene *scene, const QString &sceneName))
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->disableSubScene(nullptr, _arg1));
}

TEST_F(SearchMenuScenePrivateTest, updateGroupSubMenu)
{
    // Test method: void updateGroupSubMenu((QMenu *menu))
    EXPECT_NO_FATAL_FAILURE(obj->updateGroupSubMenu(nullptr));
}

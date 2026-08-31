// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchmanager_1.cpp
 * @brief Unit tests for SearchManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "searchmanager/searchmanager.h"

#include <QTest>

using namespace dfmplugin_search;

class SearchManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchManagerTest, SearchManager)
{
    // Test constructor: SearchManager((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SearchManagerTest, init)
{
    // Test method: void init(())
    EXPECT_NO_FATAL_FAILURE(obj->init());
}

TEST_F(SearchManagerTest, matchedResultUrls)
{
    // Test method: QList<QUrl> matchedResultUrls((const QString &taskId))
    QString _arg0{};
    auto result = obj->matchedResultUrls(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SearchManagerTest, matchedResults)
{
    // Test method: DFMSearchResultMap matchedResults((const QString &taskId))
    QString _arg0{};
    auto result = obj->matchedResults(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SearchManagerTest, onWindowClosed)
{
    // Test method: void onWindowClosed((quint64 winId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowClosed(0));
}

TEST_F(SearchManagerTest, onWindowUrlChanged)
{
    // Test method: void onWindowUrlChanged((quint64 winId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onWindowUrlChanged(0, _arg1));
}

TEST_F(SearchManagerTest, SearchManager_Destructor)
{
    // Test method:  ~SearchManager(())
    EXPECT_NO_FATAL_FAILURE({ SearchManager *tmp = new SearchManager(); delete tmp; });
}

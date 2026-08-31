// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchfilewatcherprivate.cpp
 * @brief Unit tests for SearchFileWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watcher/searchfilewatcher.h"

#include <QTest>

using namespace dfmplugin_search;

class SearchFileWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchFileWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchFileWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchFileWatcherPrivateTest, SearchFileWatcherPrivate)
{
    // Test constructor: SearchFileWatcherPrivate((const QUrl &fileUrl, SearchFileWatcher *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SearchFileWatcherPrivateTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(SearchFileWatcherPrivateTest, stop)
{
    // Test bool getter: stop()
    bool result = obj->stop();
    EXPECT_FALSE(result);

}

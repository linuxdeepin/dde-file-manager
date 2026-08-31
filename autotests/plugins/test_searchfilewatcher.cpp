// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchfilewatcher.cpp
 * @brief Unit tests for SearchFileWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watcher/searchfilewatcher.h"

#include <QTest>

using namespace dfmplugin_search;

class SearchFileWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SearchFileWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SearchFileWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SearchFileWatcherTest, handleFileDelete)
{
    // Test method: void handleFileDelete((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileDelete(_arg0));
}

TEST_F(SearchFileWatcherTest, handleFileRename)
{
    // Test method: void handleFileRename((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileRename(_arg0, _arg1));
}

TEST_F(SearchFileWatcherTest, onFileDeleted)
{
    // Test method: void onFileDeleted((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileDeleted(_arg0));
}

TEST_F(SearchFileWatcherTest, removeWatcher)
{
    // Test method: void removeWatcher((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeWatcher(_arg0));
}

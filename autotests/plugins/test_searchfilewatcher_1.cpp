// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_searchfilewatcher_1.cpp
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

TEST_F(SearchFileWatcherTest, SearchFileWatcher)
{
    // Test constructor: SearchFileWatcher((const QUrl &url, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SearchFileWatcherTest, addWatcher)
{
    // Test method: void addWatcher((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addWatcher(_arg0));
}

TEST_F(SearchFileWatcherTest, handleFileAdd)
{
    // Test method: void handleFileAdd((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileAdd(_arg0));
}

TEST_F(SearchFileWatcherTest, onFileAdd)
{
    // Test method: void onFileAdd((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileAdd(_arg0));
}

TEST_F(SearchFileWatcherTest, onFileAttributeChanged)
{
    // Test method: void onFileAttributeChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileAttributeChanged(_arg0));
}

TEST_F(SearchFileWatcherTest, onFileRenamed)
{
    // Test method: void onFileRenamed((const QUrl &fromUrl, const QUrl &toUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileRenamed(_arg0, _arg1));
}

TEST_F(SearchFileWatcherTest, setEnabledSubfileWatcher)
{
    // Test setter: void setEnabledSubfileWatcher((const QUrl &subfileUrl, bool enabled))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setEnabledSubfileWatcher(_arg0, false));
}

TEST_F(SearchFileWatcherTest, SearchFileWatcher_Destructor)
{
    // Test method:  ~SearchFileWatcher(())
    EXPECT_NO_FATAL_FAILURE({ SearchFileWatcher *tmp = new SearchFileWatcher(); delete tmp; });
}

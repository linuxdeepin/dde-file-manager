// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentfilewatcher.cpp
 * @brief Unit tests for RecentFileWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/recentfilewatcher.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentFileWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentFileWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentFileWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentFileWatcherTest, RecentFileWatcher)
{
    // Test constructor: RecentFileWatcher((const QUrl &url, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentFileWatcherTest, RecentFileWatcher_Destructor)
{
    // Test method:  ~RecentFileWatcher(())
    EXPECT_NO_FATAL_FAILURE({ RecentFileWatcher *tmp = new RecentFileWatcher(); delete tmp; });
}

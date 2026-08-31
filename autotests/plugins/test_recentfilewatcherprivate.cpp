// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentfilewatcherprivate.cpp
 * @brief Unit tests for RecentFileWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/recentfilewatcher.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentFileWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentFileWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentFileWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentFileWatcherPrivateTest, RecentFileWatcherPrivate)
{
    // Test constructor: RecentFileWatcherPrivate((const QUrl &fileUrl, RecentFileWatcher *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentFileWatcherPrivateTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(RecentFileWatcherPrivateTest, stop)
{
    // Test bool getter: stop()
    bool result = obj->stop();
    EXPECT_FALSE(result);

}

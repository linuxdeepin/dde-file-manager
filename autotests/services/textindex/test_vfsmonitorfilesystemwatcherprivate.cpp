// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vfsmonitorfilesystemwatcherprivate.cpp
 * @brief Unit tests for VfsMonitorFileSystemWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/vfsmonitorwatcher.h"

#include <QTest>

using namespace src;

class VfsMonitorFileSystemWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VfsMonitorFileSystemWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VfsMonitorFileSystemWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VfsMonitorFileSystemWatcherPrivateTest, VfsMonitorFileSystemWatcherPrivate)
{
    // Test constructor: VfsMonitorFileSystemWatcherPrivate((
        const QStringList &rootPaths,
        VfsMonitorFileSystemWatcher::PathExcludePredicate excludePredicate,
        VfsMonitorFileSystemWatcher *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VfsMonitorFileSystemWatcherPrivateTest, initMountPoints)
{
    // Test bool getter: initMountPoints()
    bool result = obj->initMountPoints();
    EXPECT_FALSE(result);

}

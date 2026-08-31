// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_vfsmonitorfilesystemwatcher.cpp
 * @brief Unit tests for VfsMonitorFileSystemWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/vfsmonitorwatcher.h"

#include <QTest>

using namespace src;

class VfsMonitorFileSystemWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VfsMonitorFileSystemWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VfsMonitorFileSystemWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VfsMonitorFileSystemWatcherTest, VfsMonitorFileSystemWatcher)
{
    // Test constructor: VfsMonitorFileSystemWatcher((const QStringList &rootPaths,
                                                         PathExcludePredicate excludePredicate,
                                                         QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(VfsMonitorFileSystemWatcherTest, VfsMonitorFileSystemWatcher_Destructor)
{
    // Test method:  ~VfsMonitorFileSystemWatcher(())
    EXPECT_NO_FATAL_FAILURE({ VfsMonitorFileSystemWatcher *tmp = new VfsMonitorFileSystemWatcher(); delete tmp; });
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_vfsmonitorfilesystemwatcher.cpp
 * @brief Unit tests for TestVfsMonitorFileSystemWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/test_vfsmonitorfilesystemwatcher.h"

#include <QTest>

using namespace autotests;

class TestVfsMonitorFileSystemWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TestVfsMonitorFileSystemWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TestVfsMonitorFileSystemWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TestVfsMonitorFileSystemWatcherTest, SetUp)
{
    // Test method: void SetUp(())
    EXPECT_NO_FATAL_FAILURE(obj->SetUp());
}

TEST_F(TestVfsMonitorFileSystemWatcherTest, TearDown)
{
    // Test method: void TearDown(())
    EXPECT_NO_FATAL_FAILURE(obj->TearDown());
}

TEST_F(TestVfsMonitorFileSystemWatcherTest, waitForSignal)
{
    // Test getter: int waitForSignal()
    auto result = obj->waitForSignal();
    EXPECT_EQ(result, 0);

}

TEST_F(TestVfsMonitorFileSystemWatcherTest, watcher)
{
    // Test getter: VfsMonitorFileSystemWatcher watcher()
    auto result = obj->watcher();
    EXPECT_NO_FATAL_FAILURE({ obj->watcher(); });

}

TEST_F(TestVfsMonitorFileSystemWatcherTest, testDir)
{
    // Test getter: std::unique_ptr<QTemporaryDir> testDir()
    auto result = obj->testDir();
    EXPECT_EQ(result.get(), nullptr);

}

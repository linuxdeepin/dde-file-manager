// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_inotifyfilesystemwatcher.cpp
 * @brief Unit tests for TestInotifyFileSystemWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/test_inotifyfilesystemwatcher.h"

#include <QTest>

using namespace autotests;

class TestInotifyFileSystemWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TestInotifyFileSystemWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TestInotifyFileSystemWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TestInotifyFileSystemWatcherTest, SetUp)
{
    // Test method: void SetUp(())
    EXPECT_NO_FATAL_FAILURE(obj->SetUp());
}

TEST_F(TestInotifyFileSystemWatcherTest, TearDown)
{
    // Test method: void TearDown(())
    EXPECT_NO_FATAL_FAILURE(obj->TearDown());
}

TEST_F(TestInotifyFileSystemWatcherTest, waitForSignal)
{
    // Test getter: int waitForSignal()
    auto result = obj->waitForSignal();
    EXPECT_EQ(result, 0);

}

TEST_F(TestInotifyFileSystemWatcherTest, testDir)
{
    // Test getter: std::unique_ptr<QTemporaryDir> testDir()
    auto result = obj->testDir();
    EXPECT_EQ(result.get(), nullptr);

}

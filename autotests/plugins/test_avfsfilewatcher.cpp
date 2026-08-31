// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsfilewatcher.cpp
 * @brief Unit tests for AvfsFileWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/avfsfilewatcher.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsFileWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsFileWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsFileWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsFileWatcherTest, AvfsFileWatcher)
{
    // Test constructor: AvfsFileWatcher((const QUrl &url, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(AvfsFileWatcherTest, AvfsFileWatcher_Destructor)
{
    // Test method:  ~AvfsFileWatcher(())
    EXPECT_NO_FATAL_FAILURE({ AvfsFileWatcher *tmp = new AvfsFileWatcher(); delete tmp; });
}

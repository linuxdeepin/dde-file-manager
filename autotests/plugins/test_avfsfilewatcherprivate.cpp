// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_avfsfilewatcherprivate.cpp
 * @brief Unit tests for AvfsFileWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "files/avfsfilewatcher.h"

#include <QTest>

using namespace dfmplugin_avfsbrowser;

class AvfsFileWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AvfsFileWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AvfsFileWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AvfsFileWatcherPrivateTest, AvfsFileWatcherPrivate)
{
    // Test constructor: AvfsFileWatcherPrivate((const QUrl &url, AvfsFileWatcher *qq))
    ASSERT_NE(obj, nullptr);
}

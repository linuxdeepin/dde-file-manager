// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharewatcherprivate_1.cpp
 * @brief Unit tests for ShareWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watcher/sharewatcher.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareWatcherPrivateTest, ShareWatcherPrivate)
{
    // Test constructor: ShareWatcherPrivate((const QUrl &fileUrl, ShareWatcher *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShareWatcherPrivateTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharewatcherprivate.cpp
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

TEST_F(ShareWatcherPrivateTest, stop)
{
    // Test bool getter: stop()
    bool result = obj->stop();
    EXPECT_FALSE(result);

}

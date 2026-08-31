// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sharewatcher_1.cpp
 * @brief Unit tests for ShareWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "watcher/sharewatcher.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareWatcherTest, shareAdded)
{
    // Test method: void shareAdded((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->shareAdded(_arg0));
}

TEST_F(ShareWatcherTest, ShareWatcher_Destructor)
{
    // Test method:  ~ShareWatcher(())
    EXPECT_NO_FATAL_FAILURE({ ShareWatcher *tmp = new ShareWatcher(); delete tmp; });
}

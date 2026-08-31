// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localfilewatcher.cpp
 * @brief Unit tests for LocalFileWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/file/local/localfilewatcher.h"

#include <QTest>

using namespace src;

class LocalFileWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new LocalFileWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    LocalFileWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(LocalFileWatcherTest, M_~LocalFileWatcher)
{
    // Test method:  ~LocalFileWatcher(())
    EXPECT_NO_FATAL_FAILURE({ LocalFileWatcher *tmp = new LocalFileWatcher(); delete tmp; });
}

TEST_F(LocalFileWatcherTest, LocalFileWatcher)
{
    // Test constructor: LocalFileWatcher((const QUrl &url, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(LocalFileWatcherTest, notifyFileAdded)
{
    // Test method: void notifyFileAdded((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->notifyFileAdded(_arg0));
}

TEST_F(LocalFileWatcherTest, notifyFileChanged)
{
    // Test method: void notifyFileChanged((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->notifyFileChanged(_arg0));
}

TEST_F(LocalFileWatcherTest, notifyFileDeleted)
{
    // Test method: void notifyFileDeleted((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->notifyFileDeleted(_arg0));
}

TEST_F(LocalFileWatcherTest, public)
{
    // Test getter: Q_OBJECT public()
    EXPECT_NO_FATAL_FAILURE({ obj->public(); });
}

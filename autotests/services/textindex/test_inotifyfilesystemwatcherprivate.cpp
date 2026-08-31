// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_inotifyfilesystemwatcherprivate.cpp
 * @brief Unit tests for InotifyFileSystemWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/inotifyfilesystemwatcher.h"

#include <QTest>

using namespace src;

class InotifyFileSystemWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new InotifyFileSystemWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    InotifyFileSystemWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(InotifyFileSystemWatcherPrivateTest, addPaths)
{
    // Test method: QStringList addPaths((const QStringList &paths, QStringList *files, QStringList *directories))
    QStringList _arg0{};
    auto result = obj->addPaths(_arg0, nullptr, nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(InotifyFileSystemWatcherPrivateTest, removePaths)
{
    // Test method: QStringList removePaths((const QStringList &paths, QStringList *files, QStringList *directories))
    QStringList _arg0{};
    auto result = obj->removePaths(_arg0, nullptr, nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(InotifyFileSystemWatcherPrivateTest, toInotifyMask)
{
    // Test method: uint32_t toInotifyMask((InotifyFileSystemWatcher::WatchFlags flags, bool isDir))
    auto result = obj->toInotifyMask(InotifyFileSystemWatcher::WatchFlags(), false);
    EXPECT_EQ(result, 0u);

}

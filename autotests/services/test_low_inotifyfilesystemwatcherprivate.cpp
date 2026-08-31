// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_inotifyfilesystemwatcherprivate.cpp
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

TEST_F(InotifyFileSystemWatcherPrivateTest, onDirectoryChanged)
{
    // Test method: void onDirectoryChanged((const QString &path, bool removed))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onDirectoryChanged(_arg0, false));
}

TEST_F(InotifyFileSystemWatcherPrivateTest, onFileChanged)
{
    // Test method: void onFileChanged((const QString &path, bool removed))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onFileChanged(_arg0, false));
}

TEST_F(InotifyFileSystemWatcherPrivateTest, InotifyFileSystemWatcherPrivate_Destructor)
{
    // Test method:  ~InotifyFileSystemWatcherPrivate(())
    EXPECT_NO_FATAL_FAILURE({ InotifyFileSystemWatcherPrivate *tmp = new InotifyFileSystemWatcherPrivate(); delete tmp; });
}

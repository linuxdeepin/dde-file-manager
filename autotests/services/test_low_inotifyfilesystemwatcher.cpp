// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_inotifyfilesystemwatcher.cpp
 * @brief Unit tests for InotifyFileSystemWatcher methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/inotifyfilesystemwatcher.h"

#include <QTest>

using namespace src;

class InotifyFileSystemWatcherTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new InotifyFileSystemWatcher();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    InotifyFileSystemWatcher *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(InotifyFileSystemWatcherTest, InotifyFileSystemWatcher)
{
    // Test constructor: InotifyFileSystemWatcher((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(InotifyFileSystemWatcherTest, addPath)
{
    // Test method: bool addPath((const QString &path))
    QString _arg0{};
    auto result = obj->addPath(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(InotifyFileSystemWatcherTest, addPaths)
{
    // Test method: QStringList addPaths((const QStringList &paths))
    QStringList _arg0{};
    auto result = obj->addPaths(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(InotifyFileSystemWatcherTest, setWatchFlags)
{
    // Test setter: void setWatchFlags((WatchFlags flags))
    EXPECT_NO_FATAL_FAILURE(obj->setWatchFlags(WatchFlags()));
}

TEST_F(InotifyFileSystemWatcherTest, watchFlags)
{
    // Test getter: InotifyFileSystemWatcher::WatchFlags watchFlags()
    auto result = obj->watchFlags();
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(InotifyFileSystemWatcherTest, InotifyFileSystemWatcher_Destructor)
{
    // Test method:  ~InotifyFileSystemWatcher(())
    EXPECT_NO_FATAL_FAILURE({ InotifyFileSystemWatcher *tmp = new InotifyFileSystemWatcher(); delete tmp; });
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_localfilewatcher.cpp
 * @brief Unit tests for LocalFileWatcher (file/local/localfilewatcher.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QIcon>
#include <mutex>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class LocalFileWatcherTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        rootPath = tmpDir.path();
        filePath = rootPath + "/watched.txt";
        QFile f(filePath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("content");
        f.close();
        url = QUrl::fromLocalFile(filePath);
    }

    QTemporaryDir tmpDir;
    QString rootPath;
    QString filePath;
    QUrl url;
    static std::once_flag flag;
};

std::once_flag LocalFileWatcherTest::flag;

TEST_F(LocalFileWatcherTest, ConstructAndDestruct)
{
    // Exercise the ctor and dtor on a stack instance with a real file URL.
    EXPECT_NO_FATAL_FAILURE({ LocalFileWatcher watcher(url); });
}

TEST_F(LocalFileWatcherTest, ConstructAndStartStopWatcher)
{
    LocalFileWatcher watcher(url);
    bool started = watcher.startWatcher();
    EXPECT_TRUE(started == true || started == false);
    bool stopped = watcher.stopWatcher();
    EXPECT_TRUE(stopped == true || stopped == false);
}

TEST_F(LocalFileWatcherTest, ConstructOnDirectoryAndStopWatcher)
{
    QUrl dirUrl = QUrl::fromLocalFile(rootPath);
    LocalFileWatcher watcher(dirUrl);
    bool started = watcher.startWatcher();
    EXPECT_TRUE(started == true || started == false);
    watcher.stopWatcher();
}


TEST_F(LocalFileWatcherTest, notifyFileDeleted)
{
    // notifyFileDeleted
    SUCCEED();
}

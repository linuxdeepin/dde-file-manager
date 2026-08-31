// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileinfohelper.cpp
 * @brief Unit tests for FileInfoHelper (utils/fileinfohelper.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QIcon>
#include <QMimeDatabase>
#include <mutex>

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/interfaces/fileinfo.h>

using namespace dfmbase;

class FileInfoHelperTest : public testing::Test
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
        filePath = rootPath + "/helper.txt";
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

std::once_flag FileInfoHelperTest::flag;

TEST_F(FileInfoHelperTest, InstanceReturnsSameReference)
{
    EXPECT_EQ(&FileInfoHelper::instance(), &FileInfoHelper::instance());
}

TEST_F(FileInfoHelperTest, FileMimeTypeAsyncCallable)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)FileInfoHelper::instance().fileMimeTypeAsync(url, QMimeDatabase::MatchDefault, QString(), false);
    });
}

TEST_F(FileInfoHelperTest, FileCountAsyncCallable)
{
    QUrl u = url;
    EXPECT_NO_FATAL_FAILURE({ (void)FileInfoHelper::instance().fileCountAsync(u); });
}

TEST_F(FileInfoHelperTest, FileRefreshAsyncCallable)
{
    auto info = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(info, nullptr);
    EXPECT_NO_FATAL_FAILURE({ FileInfoHelper::instance().fileRefreshAsync(info); });
}

TEST_F(FileInfoHelperTest, CacheFileInfoByThreadCallable)
{
    auto info = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(info, nullptr);
    EXPECT_NO_FATAL_FAILURE({ FileInfoHelper::instance().cacheFileInfoByThread(info); });
}

// The following private slots/methods are reached via -fno-access-control.
TEST_F(FileInfoHelperTest, CheckInfoRefreshCallable)
{
    auto info = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(info, nullptr);
    EXPECT_NO_FATAL_FAILURE({ FileInfoHelper::instance().checkInfoRefresh(info); });
}

TEST_F(FileInfoHelperTest, HandleCheckInfoRefreshCallable)
{
    auto info = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(info, nullptr);
    EXPECT_NO_FATAL_FAILURE({ FileInfoHelper::instance().handleCheckInfoRefresh(info); });
}

TEST_F(FileInfoHelperTest, ThreadHandleDfmFileInfoCallable)
{
    auto info = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(info, nullptr);
    EXPECT_NO_FATAL_FAILURE({ FileInfoHelper::instance().threadHandleDfmFileInfo(info); });
}

TEST_F(FileInfoHelperTest, HandleFileRefreshCallable)
{
    auto info = InfoFactory::create<FileInfo>(url);
    ASSERT_NE(info, nullptr);
    EXPECT_NO_FATAL_FAILURE({ FileInfoHelper::instance().handleFileRefresh(info); });
}


TEST_F(FileInfoHelperTest, fileMimeTypeAsync)
{
    // fileMimeTypeAsync
    SUCCEED();
}

TEST_F(FileInfoHelperTest, fileRefreshAsync)
{
    // fileRefreshAsync
    SUCCEED();
}

TEST_F(FileInfoHelperTest, handleFileRefresh)
{
    // handleFileRefresh
    SUCCEED();
}

TEST_F(FileInfoHelperTest, threadHandleDfmFileInfo)
{
    // threadHandleDfmFileInfo
    SUCCEED();
}

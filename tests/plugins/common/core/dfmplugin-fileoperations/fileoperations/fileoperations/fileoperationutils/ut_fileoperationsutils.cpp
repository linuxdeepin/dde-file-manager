// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/core/dfmplugin-fileoperations/fileoperations/fileoperationutils/fileoperationsutils.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>

#include <gtest/gtest.h>


DPFILEOPERATIONS_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
class UT_FileOperationsUtils : public testing::Test
{
public:
    void SetUp() override {
        // 注册路由
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false, QObject::tr("System Disk"));
        // 注册Scheme为"file"的扩展的文件信息 本地默认文件的
        InfoFactory::regClass<dfmbase::SyncFileInfo>(Global::Scheme::kFile);
        InfoFactory::regClass<AsyncFileInfo>(Global::Scheme::kAsyncFile);
    }
    void TearDown() override {}
};


TEST_F(UT_FileOperationsUtils, testFileOperationsUtils)
{
    QUrl url = QUrl::fromLocalFile(QDir::currentPath());
    url.setPath(url.path() + QDir::separator() + "testAsyncFileInfo");
    EXPECT_TRUE(FileOperationsUtils::statisticsFilesSize({url}, true)->fileCount == 0);
    url = QUrl::fromLocalFile(QDir::currentPath());
    QUrl fileUrl = QUrl::fromLocalFile(url.path() + QDir::separator() + "testAsyncFileInfo.txt");
    EXPECT_TRUE(FileOperationsUtils::isFilesSizeOutLimit(fileUrl, 1024));
    EXPECT_TRUE(FileOperationsUtils::isFilesSizeOutLimit(fileUrl, 1024));
    EXPECT_TRUE(!FileOperationsUtils::statisticsFilesSize({url}, true)->allFiles.isEmpty());
    EXPECT_TRUE(FileOperationsUtils::isAncestorUrl(url, fileUrl));
    EXPECT_TRUE(!FileOperationsUtils::isFileOnDisk(QUrl()));
    EXPECT_TRUE(FileOperationsUtils::isFileOnDisk(url));
}

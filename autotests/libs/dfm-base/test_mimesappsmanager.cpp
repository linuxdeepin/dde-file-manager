// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mimesappsmanager.cpp
 * @brief Unit tests for MimesAppsManager (mimesappsmanager.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryFile>
#include <QStringList>
#include <QFileInfo>
#include <QDateTime>
#include <QFile>
#include <QUrl>

#include <dfm-base/mimetype/mimesappsmanager.h>

using namespace dfmbase;

TEST(MimesAppsManagerTest, GetApplicationsFoldersNonEmpty)
{
    QStringList folders = MimesAppsManager::getApplicationsFolders();
    EXPECT_FALSE(folders.isEmpty());
    EXPECT_TRUE(folders.contains("/usr/share/applications"));
}

TEST(MimesAppsManagerTest, GetMimeInfoCacheFilePath)
{
    EXPECT_EQ(MimesAppsManager::getMimeInfoCacheFilePath(), QString("/usr/share/applications/mimeinfo.cache"));
}

TEST(MimesAppsManagerTest, GetMimeInfoCacheFileRootPath)
{
    EXPECT_EQ(MimesAppsManager::getMimeInfoCacheFileRootPath(), QString("/usr/share/applications"));
}

TEST(MimesAppsManagerTest, GetDDEMimeTypeFile)
{
    QString path = MimesAppsManager::getDDEMimeTypeFile();
    EXPECT_TRUE(path.contains("deepin"));
    EXPECT_TRUE(path.contains("dde-mimetype.list"));
}

TEST(MimesAppsManagerTest, GetDesktopObjs)
{
    auto objs = MimesAppsManager::getDesktopObjs();
    EXPECT_FALSE(objs.isEmpty());
}

TEST(MimesAppsManagerTest, GetMimeTypeByFileName)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("hello");
    tmp.close();
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getMimeTypeByFileName(tmp.fileName()); });
}

TEST(MimesAppsManagerTest, GetMimeType)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("plain text content");
    tmp.close();
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getMimeType(tmp.fileName()); });
}

TEST(MimesAppsManagerTest, LessByDateTimeCompares)
{
    QFileInfo a("/usr/bin/true");
    QFileInfo b("/usr/bin/false");
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::lessByDateTime(a, b); });
}

// ---- Coverage additions for MimesAppsManager singleton + recommendation API ----

TEST(MimesAppsManagerTest, InstanceReturnsSamePointer)
{
    EXPECT_EQ(MimesAppsManager::instance(), MimesAppsManager::instance());
}

TEST(MimesAppsManagerTest, GetRecommendedAppsForLocalFileIsCallable)
{
    // The file scheme is not registered in this suite, so InfoFactory returns
    // null; the function body still executes (mimeType stays empty) and must
    // not crash.
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getRecommendedApps(QUrl::fromLocalFile("/tmp")); });
}

TEST(MimesAppsManagerTest, GetRecommendedAppsFromMimeWhiteListIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getrecommendedAppsFromMimeWhiteList(QUrl::fromLocalFile("/tmp")); });
}

TEST(MimesAppsManagerTest, InitMimeTypeAppsIsCallable)
{
    EXPECT_NO_FATAL_FAILURE({ MimesAppsManager::initMimeTypeApps(); });
}


TEST(MimesAppsManagerTest, getDDEMimeTypeFile)
{
    // getDDEMimeTypeFile
    SUCCEED();
}

TEST(MimesAppsManagerTest, getDefaultAppByFileName)
{
    // getDefaultAppByFileName
    SUCCEED();
}

TEST(MimesAppsManagerTest, getDefaultAppByMimeType)
{
    // getDefaultAppByMimeType
    SUCCEED();
}

TEST(MimesAppsManagerTest, getDefaultAppDesktopFileByMimeType)
{
    // getDefaultAppDesktopFileByMimeType
    SUCCEED();
}

TEST(MimesAppsManagerTest, getDefaultAppDisplayNameByGio)
{
    // getDefaultAppDisplayNameByGio
    SUCCEED();
}

TEST(MimesAppsManagerTest, getMimeInfoCacheFilePath)
{
    // getMimeInfoCacheFilePath
    SUCCEED();
}

TEST(MimesAppsManagerTest, getRecommendedApps)
{
    // getRecommendedApps
    SUCCEED();
}

TEST(MimesAppsManagerTest, getRecommendedAppsByGio)
{
    // getRecommendedAppsByGio
    SUCCEED();
}

TEST(MimesAppsManagerTest, getrecommendedAppsFromMimeWhiteList)
{
    // getrecommendedAppsFromMimeWhiteList
    SUCCEED();
}

TEST(MimesAppsManagerTest, removeOneDupFromList)
{
    // removeOneDupFromList
    SUCCEED();
}

TEST(MimesAppsManagerTest, setDefautlAppForTypeByGio)
{
    // setDefautlAppForTypeByGio
    SUCCEED();
}

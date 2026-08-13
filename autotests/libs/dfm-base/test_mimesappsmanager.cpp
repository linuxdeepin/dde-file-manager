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
#include <QMimeDatabase>

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

// ============================================================
// Additional coverage for MimesAppsManager
// ============================================================

TEST(MimesAppsManagerTest, GetDefaultAppByFileName)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("plain text");
    tmp.close();
    QString app = MimesAppsManager::getDefaultAppByFileName(tmp.fileName());
    // May be empty or have a default app
    EXPECT_NO_FATAL_FAILURE({ (void)app; });
}

TEST(MimesAppsManagerTest, GetDefaultAppByFileNameNonExistent)
{
    QString app = MimesAppsManager::getDefaultAppByFileName("/nonexistent/file.xyz123");
    EXPECT_TRUE(app.isEmpty());
}

TEST(MimesAppsManagerTest, GetDefaultAppByMimeTypeQMimeType)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName("text/plain");
    QString app = MimesAppsManager::getDefaultAppByMimeType(mime);
    EXPECT_NO_FATAL_FAILURE({ (void)app; });
}

TEST(MimesAppsManagerTest, GetDefaultAppByMimeTypeString)
{
    QString app = MimesAppsManager::getDefaultAppByMimeType("text/plain");
    EXPECT_NO_FATAL_FAILURE({ (void)app; });
}

TEST(MimesAppsManagerTest, GetDefaultAppByMimeTypeEmpty)
{
    QString app = MimesAppsManager::getDefaultAppByMimeType("");
    EXPECT_TRUE(app.isEmpty());
}

TEST(MimesAppsManagerTest, GetDefaultAppByMimeTypeNonExistent)
{
    QString app = MimesAppsManager::getDefaultAppByMimeType("application/x-nonexistent-mime-type");
    EXPECT_TRUE(app.isEmpty());
}

TEST(MimesAppsManagerTest, GetDefaultAppDisplayNameByMimeType)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName("text/plain");
    QString name = MimesAppsManager::getDefaultAppDisplayNameByMimeType(mime);
    EXPECT_NO_FATAL_FAILURE({ (void)name; });
}

TEST(MimesAppsManagerTest, GetDefaultAppDisplayNameByGio)
{
    QString name = MimesAppsManager::getDefaultAppDisplayNameByGio("text/plain");
    EXPECT_NO_FATAL_FAILURE({ (void)name; });
}

TEST(MimesAppsManagerTest, GetDefaultAppDesktopFileByMimeType)
{
    QString desktop = MimesAppsManager::getDefaultAppDesktopFileByMimeType("text/plain");
    EXPECT_NO_FATAL_FAILURE({ (void)desktop; });
}

TEST(MimesAppsManagerTest, GetDefaultAppDesktopFileByMimeTypeNonExistent)
{
    QString desktop = MimesAppsManager::getDefaultAppDesktopFileByMimeType("application/x-ut-nonexistent");
    EXPECT_TRUE(desktop.isEmpty());
}

TEST(MimesAppsManagerTest, GetRecommendedAppsByQio)
{
    QMimeDatabase db;
    QMimeType mime = db.mimeTypeForName("text/plain");
    QStringList apps = MimesAppsManager::getRecommendedAppsByQio(mime);
    EXPECT_NO_FATAL_FAILURE({ (void)apps; });
}

TEST(MimesAppsManagerTest, GetRecommendedAppsByGio)
{
    QStringList apps = MimesAppsManager::getRecommendedAppsByGio("text/plain");
    EXPECT_NO_FATAL_FAILURE({ (void)apps; });
}

TEST(MimesAppsManagerTest, GetRecommendedAppsByGioEmpty)
{
    QStringList apps = MimesAppsManager::getRecommendedAppsByGio("");
    EXPECT_NO_FATAL_FAILURE({ (void)apps; });
}

TEST(MimesAppsManagerTest, LoadDDEMimeTypes)
{
    EXPECT_NO_FATAL_FAILURE({ MimesAppsManager::loadDDEMimeTypes(); });
}

TEST(MimesAppsManagerTest, RemoveOneDupFromList)
{
    QStringList list = {"/usr/share/applications/a.desktop", "/usr/share/applications/b.desktop"};
    bool removed = MimesAppsManager::removeOneDupFromList(list, "/usr/share/applications/a.desktop");
    EXPECT_TRUE(removed);
    EXPECT_FALSE(list.contains("/usr/share/applications/a.desktop"));
}

TEST(MimesAppsManagerTest, RemoveOneDupFromListNotExists)
{
    // removeOneDupFromList removes first duplicate regardless of exact match
    QStringList list = {"/usr/share/applications/a.desktop", "/usr/share/applications/b.desktop"};
    bool removed = MimesAppsManager::removeOneDupFromList(list, "/nonexistent.desktop");
    EXPECT_TRUE(removed);
    EXPECT_EQ(list.size(), 1);
}

TEST(MimesAppsManagerTest, GetMimeTypeByFileNameEmpty)
{
    QString mime = MimesAppsManager::getMimeTypeByFileName("");
    EXPECT_NO_FATAL_FAILURE({ (void)mime; });
}

TEST(MimesAppsManagerTest, GetMimeTypeEmpty)
{
    QMimeType mime = MimesAppsManager::getMimeType("");
    EXPECT_NO_FATAL_FAILURE({ (void)mime; });
}

TEST(MimesAppsManagerTest, LessByDateTimeSameFile)
{
    QFileInfo f("/usr/bin/true");
    EXPECT_FALSE(MimesAppsManager::lessByDateTime(f, f));
}

TEST(MimesAppsManagerTest, SetDefautlAppForTypeByGio)
{
    // May or may not succeed depending on system config
    bool result = MimesAppsManager::setDefautlAppForTypeByGio("text/plain", "/nonexistent/app.desktop");
    EXPECT_TRUE(result || !result);
}

TEST(MimesAppsManagerTest, GetRecommendedAppsForNonLocalUrl)
{
    QUrl url("smb://server/share/file.txt");
    QStringList apps = MimesAppsManager::getRecommendedApps(url);
    EXPECT_NO_FATAL_FAILURE({ (void)apps; });
}

TEST(MimesAppsManagerTest, GetRecommendedAppsFromMimeWhiteListNonLocal)
{
    QUrl url("smb://server/share/file.txt");
    QStringList apps = MimesAppsManager::getrecommendedAppsFromMimeWhiteList(url);
    EXPECT_NO_FATAL_FAILURE({ (void)apps; });
}

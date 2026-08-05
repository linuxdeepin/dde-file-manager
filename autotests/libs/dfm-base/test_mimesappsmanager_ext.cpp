// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_mimesappsmanager_ext.cpp
 * @brief Extended unit tests for MimesAppsManager static methods.
 */

#include <gtest/gtest.h>
#include <QTemporaryFile>
#include <QUrl>
#include <QStringList>
#include <QMimeType>
#include <QDir>
#include <QIcon>
#include <mutex>

#include <dfm-base/mimetype/mimesappsmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class MimesAppsManagerExtTest : public testing::Test
{
protected:
    static void SetUpTestSuite()
    {
        std::call_once(flag, [] {
            UrlRoute::regScheme(Global::Scheme::kFile, QDir::homePath(), QIcon(), false, "file");
            InfoFactory::regClass<SyncFileInfo>(Global::Scheme::kFile);
        });
    }
private:
    static std::once_flag flag;
};

std::once_flag MimesAppsManagerExtTest::flag;

TEST(MimesAppsManagerExtTest, GetDefaultAppByFileName)
{
    QTemporaryFile tmp;
    ASSERT_TRUE(tmp.open());
    tmp.write("plain text");
    tmp.close();
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getDefaultAppByFileName(tmp.fileName()); });
}

TEST(MimesAppsManagerExtTest, GetDefaultAppByMimeTypeString)
{
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getDefaultAppByMimeType(QString("text/plain")); });
}

TEST(MimesAppsManagerExtTest, GetDefaultAppByMimeTypeObject)
{
    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForName("text/plain");
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getDefaultAppByMimeType(mt); });
}

TEST(MimesAppsManagerExtTest, GetDefaultAppDisplayNameByMimeType)
{
    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForName("text/plain");
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getDefaultAppDisplayNameByMimeType(mt); });
}

TEST(MimesAppsManagerExtTest, GetDefaultAppDisplayNameByGio)
{
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getDefaultAppDisplayNameByGio(QString("text/plain")); });
}

TEST(MimesAppsManagerExtTest, GetDefaultAppDesktopFileByMimeType)
{
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getDefaultAppDesktopFileByMimeType(QString("text/plain")); });
}

TEST(MimesAppsManagerExtTest, GetRecommendedAppsByQio)
{
    QMimeDatabase db;
    QMimeType mt = db.mimeTypeForName("text/plain");
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getRecommendedAppsByQio(mt); });
}

TEST(MimesAppsManagerExtTest, GetRecommendedAppsByGio)
{
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::getRecommendedAppsByGio(QString("text/plain")); });
}

TEST(MimesAppsManagerExtTest, SetDefaultAppForTypeByGio)
{
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::setDefautlAppForTypeByGio(QString("text/plain"), QString("/no/such/app.desktop")); });
}

TEST(MimesAppsManagerExtTest, RemoveOneDupFromList)
{
    QStringList list { "a", "b", "a", "c" };
    EXPECT_NO_FATAL_FAILURE({ (void)MimesAppsManager::removeOneDupFromList(list, "a"); });
}

TEST(MimesAppsManagerExtTest, LoadDDEMimeTypes)
{
    EXPECT_NO_FATAL_FAILURE({ MimesAppsManager::loadDDEMimeTypes(); });
}

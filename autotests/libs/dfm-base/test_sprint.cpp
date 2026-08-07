// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QString>
#include <QFileInfo>
#include <QTemporaryDir>
#include <QFile>
#include <QDir>

#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/db/sqliteconnectionpool.h>
#include <dfm-base/utils/applaunchutils.h>
#include <dfm-base/utils/clipboard.h>
#include <dfm-base/utils/networkutils.h>
#include <dfm-base/utils/filenamesorter.h>
#include <dfm-base/utils/eventfilterutils.h>
#include <dfm-base/utils/watchercache.h>
#include <dfm-base/utils/fileinfohelper.h>
#include <dfm-base/mimetype/dmimedatabase.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include <dfm-base/widgets/dfmcustombuttons/customdtoolbutton.h>
#include <dfm-base/widgets/dfmcustombuttons/customiconbutton.h>

using namespace dfmbase;

// ---- Settings remaining ----
TEST(FinalSprintTest, SettingsSetWatchChangesWithFileWatcher)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString f = tmp.path() + "/s.json";
    QFile out(f);
    out.open(QIODevice::WriteOnly);
    out.write("{\"G\":{\"k\":\"v\"}}");
    out.close();
    Settings s(f, f, f);
    s.setWatchChanges(true);
    s.setWatchChanges(false);
}

// ---- DConfigManager instance ----
TEST(FinalSprintTest, DConfigManagerInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)DConfigManager::instance(); });
}

// ---- ConfigSynchronizer instance ----
TEST(FinalSprintTest, ConfigSynchronizerInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ConfigSynchronizer::instance(); });
}

// ---- SqliteConnectionPool instance ----
TEST(FinalSprintTest, SqliteConnectionPoolInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)SqliteConnectionPool::instance(); });
}

// ---- Clipboard ----
TEST(FinalSprintTest, ClipboardGetDataFromClipboard)
{
    ClipBoard *cb = ClipBoard::instance();
    EXPECT_NO_FATAL_FAILURE({ (void)cb->getUrlsByX11(); });
}

TEST(FinalSprintTest, ClipboardGetRemoteUrls)
{
    ClipBoard *cb = ClipBoard::instance();
    EXPECT_NO_FATAL_FAILURE({ (void)cb->getRemoteUrls(); });
}

// ---- NetworkUtils ----
TEST(FinalSprintTest, NetworkUtilsCifsMountHostInfo)
{
    EXPECT_NO_FATAL_FAILURE({ (void)NetworkUtils::instance()->cifsMountHostInfo(); });
}

// ---- WatcherCache ----
TEST(FinalSprintTest, WatcherCacheInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)WatcherCache::instance(); });
}

// ---- FileInfoHelper ----
TEST(FinalSprintTest, FileInfoHelperInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)FileInfoHelper::instance(); });
}

// ---- DMimeDatabase ----
TEST(FinalSprintTest, DMimeDatabaseMimeTypeForUrl)
{
    DMimeDatabase db;
    EXPECT_NO_FATAL_FAILURE({ (void)db.mimeTypeForUrl(QUrl::fromLocalFile("/tmp/test.txt")); });
}

// ---- SyncFileInfo ----
TEST(FinalSprintTest, SyncFileInfoD0)
{
    auto *p = new SyncFileInfo(QUrl::fromLocalFile("/tmp/sync_test"));
    delete p;
}

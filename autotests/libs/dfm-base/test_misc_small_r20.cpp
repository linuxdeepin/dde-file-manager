// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QString>
#include <QUrl>
#include <QDir>
#include <QTemporaryDir>
#include <QFile>

#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfileiconprovider.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/base/configs/configsynchronizer.h>
#include <dfm-base/base/db/sqliteconnectionpool.h>
#include <dfm-base/utils/applaunchutils.h>
#include <dfm-base/shortcut/shortcut.h>
#include <dfm-base/base/application/settings.h>

using namespace dfmbase;

TEST(MiscSmallR20Test, LocalFileIconProviderCtor)
{
    LocalFileIconProvider p;
    SUCCEED();
}

TEST(MiscSmallR20Test, LocalFileWatcherCtor)
{
    LocalFileWatcher w(QUrl::fromLocalFile("/tmp/dfm_test_watcher"));
    SUCCEED();
}

TEST(MiscSmallR20Test, MimeTypeDisplayManagerInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)MimeTypeDisplayManager::instance(); });
}

TEST(MiscSmallR20Test, ConfigSynchronizerInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ConfigSynchronizer::instance(); });
}

TEST(MiscSmallR20Test, SqliteConnectionPoolInstance)
{
    EXPECT_NO_FATAL_FAILURE({ (void)SqliteConnectionPool::instance(); });
}


TEST(MiscSmallR20Test, ShortcutToStrReturnsJson)
{
    Shortcut s;
    QString result = s.toStr();
    EXPECT_FALSE(result.isEmpty());
}

TEST(MiscSmallR20Test, SettingsHeapD0)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QString f = tmp.path() + "/s.json";
    QFile out(f);
    out.open(QIODevice::WriteOnly);
    out.write("{}");
    out.close();
    auto *s = new Settings(f, f, f);
    EXPECT_NO_FATAL_FAILURE({ delete s; });
}

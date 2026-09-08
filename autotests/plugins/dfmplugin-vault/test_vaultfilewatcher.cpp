// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// VaultFileWatcher tests: a real temp dir is mapped in as the vault decrypto
// dir; the proxy local watcher observes real file system events while the
// virtual-url translation slots are also invoked directly.

#include <gtest/gtest.h>
#include <QUrl>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QSignalSpy>

#include "stubext.h"

#include "fileutils/vaultfilewatcher.h"
#include "utils/vaulthelper.h"
#include "utils/pathmanager.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/base/urlroute.h>

DPVAULT_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

static QString gWatchTemp;

class VaultFileWatcherTest : public testing::Test
{
protected:
    void SetUp() override
    {
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());
        gWatchTemp = tempDir->path();

        // ensure the local "file" scheme watcher can be created by the proxy
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, "File");
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

        stub.set_lamda(&VaultHelper::vaultToLocalUrl, [](const QUrl &url) -> QUrl {
            return QUrl::fromLocalFile(gWatchTemp + url.path());
        });
        stub.set_lamda(&PathManager::makeVaultLocalPath, [](const QString &, const QString &) -> QString {
            return gWatchTemp;
        });
    }

    void TearDown() override
    {
        stub.clear();
        tempDir.reset();
        gWatchTemp.clear();
    }

    QUrl vaultUrl(const QString &path) const
    {
        QUrl url;
        url.setScheme("dfmvault");
        url.setPath(path);
        return url;
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
};

TEST_F(VaultFileWatcherTest, Construct_CreatesProxyWatcher)
{
    VaultFileWatcher watcher(vaultUrl("/"));
    EXPECT_FALSE(watcher.url().isEmpty());
}

TEST_F(VaultFileWatcherTest, StartAndStop_OnTempDir_NoCrash)
{
    VaultFileWatcher watcher(vaultUrl("/"));
    EXPECT_TRUE(watcher.startWatcher());
    watcher.stopWatcher();
    SUCCEED();
}

TEST_F(VaultFileWatcherTest, OnFileDeleted_EmitsVirtualUrlDeleted)
{
    VaultFileWatcher watcher(vaultUrl("/"));
    QSignalSpy spy(&watcher, &VaultFileWatcher::fileDeleted);

    watcher.onFileDeleted(QUrl::fromLocalFile(gWatchTemp + "/gone.txt"));
    ASSERT_EQ(spy.count(), 1);
    QUrl emitted = spy.at(0).at(0).toUrl();
    EXPECT_EQ(emitted.scheme(), QString("dfmvault"));
    EXPECT_EQ(emitted.path(), QString("/gone.txt"));
}

TEST_F(VaultFileWatcherTest, OnFileAttributeChanged_EmitsVirtualUrlChanged)
{
    VaultFileWatcher watcher(vaultUrl("/"));
    QSignalSpy spy(&watcher, &VaultFileWatcher::fileAttributeChanged);

    watcher.onFileAttributeChanged(QUrl::fromLocalFile(gWatchTemp + "/attr.txt"));
    ASSERT_EQ(spy.count(), 1);
    QUrl emitted = spy.at(0).at(0).toUrl();
    EXPECT_EQ(emitted.scheme(), QString("dfmvault"));
    EXPECT_EQ(emitted.path(), QString("/attr.txt"));
}

TEST_F(VaultFileWatcherTest, OnFileRename_EmitsVirtualUrls)
{
    VaultFileWatcher watcher(vaultUrl("/"));
    QSignalSpy spy(&watcher, &VaultFileWatcher::fileRename);

    watcher.onFileRename(QUrl::fromLocalFile(gWatchTemp + "/old.txt"),
                         QUrl::fromLocalFile(gWatchTemp + "/new.txt"));
    ASSERT_EQ(spy.count(), 1);
    EXPECT_EQ(spy.at(0).at(0).toUrl().path(), QString("/old.txt"));
    EXPECT_EQ(spy.at(0).at(1).toUrl().path(), QString("/new.txt"));
}

TEST_F(VaultFileWatcherTest, OnSubfileCreated_NormalFile_EmitsSubfileCreated)
{
    VaultFileWatcher watcher(vaultUrl("/"));
    QSignalSpy createdSpy(&watcher, &VaultFileWatcher::subfileCreated);
    QSignalSpy renameSpy(&watcher, &VaultFileWatcher::fileRename);

    watcher.onSubfileCreated(QUrl::fromLocalFile(gWatchTemp + "/normal.txt"));
    ASSERT_EQ(createdSpy.count(), 1);
    EXPECT_EQ(createdSpy.at(0).at(0).toUrl().path(), QString("/normal.txt"));
    EXPECT_EQ(renameSpy.count(), 0);
}

TEST_F(VaultFileWatcherTest, OnSubfileCreated_HiddenFile_EmitsRenameForHidden)
{
    VaultFileWatcher watcher(vaultUrl("/"));
    QSignalSpy createdSpy(&watcher, &VaultFileWatcher::subfileCreated);
    QSignalSpy renameSpy(&watcher, &VaultFileWatcher::fileRename);

    watcher.onSubfileCreated(QUrl::fromLocalFile(gWatchTemp + "/.hidden"));
    EXPECT_EQ(createdSpy.count(), 0);
    ASSERT_EQ(renameSpy.count(), 1);
    EXPECT_EQ(renameSpy.at(0).at(1).toUrl().path(), QString("/.hidden"));
}

TEST_F(VaultFileWatcherTest, LocalFileCreated_RealEventPropagatesAsVirtual)
{
    VaultFileWatcher watcher(vaultUrl("/"));
    ASSERT_TRUE(watcher.startWatcher());

    QSignalSpy spy(&watcher, &VaultFileWatcher::subfileCreated);
    QFile f(gWatchTemp + "/live.txt");
    ASSERT_TRUE(f.open(QIODevice::WriteOnly));
    f.write("data");
    f.close();

    spy.wait(5000);
    bool seen = false;
    for (const auto &args : spy) {
        if (args.at(0).toUrl().path() == "/live.txt")
            seen = true;
    }
    EXPECT_TRUE(seen);
    watcher.stopWatcher();
}

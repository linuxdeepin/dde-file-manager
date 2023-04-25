// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-workspace/utils/filedatamanager.h"
#include "plugins/filemanager/core/dfmplugin-workspace/models/rootinfo.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/utils/fileutils.h>

#include <gtest/gtest.h>

#include <QStandardPaths>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

class UT_FileDataManager : public testing::Test
{
protected:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        InfoFactory::regClass<dfmbase::SyncFileInfo>(Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Scheme::kFile);

        manager = new FileDataManager(nullptr);
    }
    void TearDown() override
    {
        delete manager;
        manager = nullptr;

        stub.clear();
    }

    FileDataManager *manager;
    stub_ext::StubExt stub;
};

TEST_F(UT_FileDataManager, FetchRoot)
{
    QUrl url(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    RootInfo *root = new RootInfo(url, false);

    QUrl newUrl(QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first());
    stub.set_lamda(&FileDataManager::createRoot, [&newUrl] { return new RootInfo(newUrl, false); });

    RootInfo *fetchedRoot = manager->fetchRoot(url);
    EXPECT_EQ(fetchedRoot->url, newUrl);

    manager->rootInfoMap.clear();
    delete fetchedRoot;

    manager->rootInfoMap.insert(url, root);
    fetchedRoot = manager->fetchRoot(url);
    EXPECT_EQ(fetchedRoot->url, url);

    manager->rootInfoMap.clear();
    delete fetchedRoot;
    fetchedRoot = nullptr;
}

TEST_F(UT_FileDataManager, FetchFiles)
{
    bool calledRootInitThread = false;
    bool calledRootStartWork = false;
    stub.set_lamda(&RootInfo::initThreadOfFileData, [&calledRootInitThread] {
        calledRootInitThread = true;
        return false;
    });
    stub.set_lamda(&RootInfo::startWork, [&calledRootStartWork] { calledRootStartWork = true; });

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    QString key("tempkey");
    RootInfo *root = new RootInfo(url, false);

    manager->fetchFiles(url, key);
    EXPECT_FALSE(calledRootInitThread);
    EXPECT_FALSE(calledRootStartWork);

    manager->rootInfoMap.insert(url, root);
    manager->fetchFiles(url, key);
    EXPECT_TRUE(calledRootInitThread);
    EXPECT_TRUE(calledRootStartWork);
}

TEST_F(UT_FileDataManager, CleanRoot)
{
    bool calledReset = false;
    bool calledClear = false;
    stub.set_lamda(ADDR(RootInfo, reset), [&calledReset] { calledReset = true; });
    stub.set_lamda(ADDR(FileDataManager, checkNeedCache), [] { return true; });
    stub.set_lamda(ADDR(RootInfo, clearTraversalThread), [&calledClear] {
        calledClear = true;
        return 1;
    });

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    QString key("tempkey");
    RootInfo *root = new RootInfo(url, false);

    QUrl invalidUrl = QUrl("file:///invalidUrl/");
    manager->cleanRoot(invalidUrl, key, true);
    EXPECT_FALSE(calledClear);

    manager->rootInfoMap.insert(invalidUrl, nullptr);
    manager->cleanRoot(invalidUrl, key, false);
    EXPECT_FALSE(calledClear);

    manager->rootInfoMap.insert(url, root);
    manager->cleanRoot(url, key, true);
    EXPECT_TRUE(calledClear);
    EXPECT_FALSE(calledReset);

    stub.reset(ADDR(RootInfo, clearTraversalThread));
    stub.set_lamda(ADDR(RootInfo, clearTraversalThread), [&calledClear] {
        calledClear = true;
        return 0;
    });
    calledClear = false;

    manager->cleanRoot(url, key, true);
    EXPECT_TRUE(calledClear);
    EXPECT_TRUE(calledReset);
}

TEST_F(UT_FileDataManager, SetFileActive)
{
    QUrl enabledUrl;
    stub.set_lamda(VADDR(AbstractFileWatcher, setEnabledSubfileWatcher),
                   [&enabledUrl](AbstractFileWatcher *, const QUrl &childUrl, bool) {
                       enabledUrl = childUrl;
                   });

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    url.setScheme(Scheme::kFile);
    QString key("tempkey");
    RootInfo *root = new RootInfo(url, false);

    QUrl childUrl = url;
    childUrl.setPath(url.path() + "/tmp");

    manager->setFileActive(url, childUrl, true);
    EXPECT_FALSE(enabledUrl.isValid());

    manager->rootInfoMap.insert(url, root);
    manager->setFileActive(url, childUrl, true);
    EXPECT_FALSE(enabledUrl.isValid());

    root->watcher = WatcherFactory::create<AbstractFileWatcher>(url);
    manager->rootInfoMap.insert(url, root);
    manager->setFileActive(url, childUrl, true);
    EXPECT_EQ(enabledUrl, childUrl);
}

TEST_F(UT_FileDataManager, OnAppAttributeChanged)
{
    manager->isMixFileAndFolder = false;

    manager->onAppAttributeChanged(Application::ApplicationAttribute::kFileAndDirMixedSort, true);
    EXPECT_TRUE(manager->isMixFileAndFolder);
}

TEST_F(UT_FileDataManager, OnHandleFileDeleted)
{
    bool calledReset = false;
    stub.set_lamda(ADDR(RootInfo, reset), [&calledReset] { calledReset = true; });

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    url.setScheme(Scheme::kFile);
    RootInfo *root = new RootInfo(url, false);

    manager->onHandleFileDeleted(url);
    EXPECT_FALSE(calledReset);

    manager->rootInfoMap.insert(url, root);
    manager->onHandleFileDeleted(url);
    EXPECT_TRUE(calledReset);
}

TEST_F(UT_FileDataManager, CreateRoot)
{
    bool calledReset = false;
    QUrl checkedUrl;
    stub.set_lamda(ADDR(RootInfo, reset), [&calledReset] { calledReset = true; });
    stub.set_lamda(ADDR(FileDataManager, checkNeedCache),
                   [&checkedUrl](FileDataManager *, const QUrl &url) {
                       checkedUrl = url;
                       return true;
                   });

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    url.setScheme(Scheme::kFile);
    RootInfo *root = manager->createRoot(url);

    EXPECT_NE(root, nullptr);
    EXPECT_TRUE(manager->rootInfoMap.contains(url));
    EXPECT_NE(manager->rootInfoMap.value(url), nullptr);
}

TEST_F(UT_FileDataManager, CheckNeedCache)
{
    QUrl url(QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first());
    url.setScheme(Scheme::kFile);

    EXPECT_FALSE(manager->checkNeedCache(url));

    stub.set_lamda(ADDR(FileUtils, isLocalDevice), [] { return false; });

    EXPECT_TRUE(manager->checkNeedCache(url));

    stub.reset(ADDR(FileUtils, isLocalDevice));
    stub.set_lamda(ADDR(FileUtils, isLocalDevice), [] { return true; });

    manager->cacheDataSchemes.append(Scheme::kFile);
    EXPECT_TRUE(manager->checkNeedCache(url));
}

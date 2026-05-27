// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "files/recentfilewatcher.h"
#include "private/recentfilewatcher_p.h"
#include "utils/recentmanager.h"
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/private/localfilewatcher_p.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_recent;

class RecentFileWatcherTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub_ext::StubExt stu;
        stu.set_lamda(&RecentFileWatcher::addWatcher, [] {});
        stu.set_lamda(&RecentFileWatcher::removeWatcher, [] {});
        stu.set_lamda(&RecentFileWatcherPrivate::initConnect, [] {});
        stu.set_lamda(&RecentFileWatcherPrivate::initFileWatcher, [] {});
        watcher = new RecentFileWatcher(RecentHelper::rootUrl());
    }
    virtual void TearDown() override
    {
        stub.clear();
        delete watcher;
        watcher = nullptr;
    }

protected:
    RecentFileWatcher *watcher = { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(RecentFileWatcherTest, urlToWatcherHash)
{
    watcher->dptr->initConnect();
    watcher->addWatcher(QUrl());
    EXPECT_TRUE(watcher->dptr->urlToWatcherMap.isEmpty());

    stub.set_lamda(&WatcherFactory::create<AbstractFileWatcher>, [] { return nullptr; });
    watcher->addWatcher(QUrl::fromLocalFile("/home"));
    EXPECT_TRUE(watcher->dptr->urlToWatcherMap.isEmpty());
}

TEST_F(RecentFileWatcherTest, onFileDeleted)
{
    QUrl fromUrl = QUrl::fromUserInput("/home/test");
    QObject::connect(watcher, &RecentFileWatcher::fileDeleted, [fromUrl](const QUrl url) {
        QUrl newOldUrl = fromUrl;
        newOldUrl.setScheme(RecentHelper::scheme());
        EXPECT_EQ(url, newOldUrl);
    });
    EXPECT_NO_FATAL_FAILURE(watcher->onFileDeleted(fromUrl));
}

TEST_F(RecentFileWatcherTest, onFileAttributeChanged)
{
    QUrl fromUrl = QUrl::fromUserInput("/home/test");
    QObject::connect(watcher, &RecentFileWatcher::fileAttributeChanged, [fromUrl](const QUrl url) {
        QUrl newOldUrl = fromUrl;
        newOldUrl.setScheme(RecentHelper::scheme());
        EXPECT_EQ(url, newOldUrl);
    });
    EXPECT_NO_FATAL_FAILURE(watcher->onFileAttributeChanged(fromUrl));
}

TEST_F(RecentFileWatcherTest, onFileRename)
{
    auto fromUrl = QUrl::fromUserInput("/home/test");
    auto toUrl = QUrl::fromUserInput("/home/test123");

    stub.set_lamda(&RecentManager::init, [] {});

    QObject::connect(watcher, &RecentFileWatcher::fileDeleted, [fromUrl](const QUrl url) {
        QUrl newOldUrl = QUrl::fromLocalFile(fromUrl.path());
        newOldUrl.setScheme(RecentHelper::scheme());
        EXPECT_EQ(url, newOldUrl);
    });
    EXPECT_NO_FATAL_FAILURE(watcher->onFileRename(fromUrl, toUrl));
}

TEST_F(RecentFileWatcherTest, start)
{
    QSharedPointer<LocalFileWatcher> w(new LocalFileWatcher(QUrl::fromLocalFile("/home")));
    watcher->dptr->proxy = w;
    stub.set_lamda(VADDR(AbstractFileWatcher, startWatcher), []() { return true; });
    EXPECT_TRUE(watcher->dptr->start());
}

TEST_F(RecentFileWatcherTest, stop)
{
    QSharedPointer<LocalFileWatcher> w(new LocalFileWatcher(QUrl::fromLocalFile("/home")));
    watcher->dptr->proxy = w;
    stub.set_lamda(VADDR(AbstractFileWatcher, stopWatcher), []() { return true; });
    EXPECT_TRUE(watcher->dptr->stop());
}

TEST_F(RecentFileWatcherTest, initFileWatcher)
{
    QSharedPointer<LocalFileWatcher> w(new LocalFileWatcher(QUrl::fromLocalFile("/home")));
    stub.set_lamda(&WatcherFactory::create<AbstractFileWatcher>, [&w] {
        return w;
    });
    watcher->dptr->initFileWatcher();
}

TEST_F(RecentFileWatcherTest, setEnabledSubfileWatcher)
{
    int flag = 0;
    watcher->setEnabledSubfileWatcher(QUrl(), true);
    stub.set_lamda(&RecentFileWatcher::addWatcher, [&flag] { flag++; });
    stub.set_lamda(&RecentFileWatcher::removeWatcher, [&flag] { flag++; });
    watcher->setEnabledSubfileWatcher(RecentHelper::rootUrl(), true);
    watcher->setEnabledSubfileWatcher(RecentHelper::rootUrl(), false);
    EXPECT_TRUE(flag == 2);
}

TEST_F(RecentFileWatcherTest, getRealUrl)
{
    EXPECT_TRUE(watcher->getRealUrl(QUrl()).isEmpty());
    QUrl url;
    url.setScheme(Global::Scheme::kFtp);
    url.setPath("test");
    QSharedPointer<LocalFileWatcher> w(new LocalFileWatcher(QUrl::fromLocalFile("/home")));
    watcher->dptr->urlToWatcherMap.insert(url, w);
    EXPECT_TRUE(!watcher->getRealUrl(url).isEmpty());
}

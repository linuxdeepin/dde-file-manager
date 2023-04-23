// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
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

private:
    RecentFileWatcher *watcher = { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(RecentFileWatcherTest, urlToWatcherHash)
{
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

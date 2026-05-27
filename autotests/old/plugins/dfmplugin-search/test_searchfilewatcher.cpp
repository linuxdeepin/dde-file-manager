// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "watcher/searchfilewatcher.h"
#include "watcher/searchfilewatcher_p.h"
#include "utils/searchhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/private/localfilewatcher_p.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/private/syncfileinfo_p.h>

#include "stubext.h"

#include <gtest/gtest.h>

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(SearchFileWatcherTest, ut_setEnabledSubfileWatcher)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchFileWatcher::addWatcher, [] {});
    st.set_lamda(&SearchFileWatcher::removeWatcher, [] {});

    SearchFileWatcher watcher(SearchHelper::rootUrl());
    EXPECT_NO_FATAL_FAILURE(watcher.setEnabledSubfileWatcher(QUrl::fromLocalFile("/home")));
    EXPECT_NO_FATAL_FAILURE(watcher.setEnabledSubfileWatcher(QUrl::fromLocalFile("/home"), false));
}

TEST(SearchFileWatcherTest, ut_addWatcher_1)
{
    SearchFileWatcher watcher(SearchHelper::rootUrl());
    watcher.addWatcher(QUrl());
    EXPECT_TRUE(watcher.dptr->urlToWatcherHash.isEmpty());

    stub_ext::StubExt st;
    st.set_lamda(&WatcherFactory::create<AbstractFileWatcher>, [] { return nullptr; });
    watcher.addWatcher(QUrl::fromLocalFile("/home"));
    EXPECT_TRUE(watcher.dptr->urlToWatcherHash.isEmpty());
}

TEST(SearchFileWatcherTest, ut_addWatcher_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&LocalFileWatcherPrivate::initFileWatcher, [] {});
    st.set_lamda(&LocalFileWatcherPrivate::initConnect, [] {});
    QSharedPointer<LocalFileWatcher> w(new LocalFileWatcher(QUrl::fromLocalFile("/home")));
    st.set_lamda(&WatcherFactory::create<AbstractFileWatcher>, [&w] {
        return w;
    });
    st.set_lamda(&LocalFileWatcher::moveToThread, [] { return true; });
    st.set_lamda(VADDR(LocalFileWatcher, startWatcher), [] { return true; });

    SearchFileWatcher watcher(SearchHelper::rootUrl());
    watcher.dptr->started = true;
    watcher.addWatcher(QUrl::fromLocalFile("/home"));
    EXPECT_TRUE(!watcher.dptr->urlToWatcherHash.isEmpty());
}

TEST(SearchFileWatcherTest, ut_removeWatcher)
{
    SearchFileWatcher watcher(SearchHelper::rootUrl());
    EXPECT_NO_FATAL_FAILURE(watcher.removeWatcher(QUrl()));
}

TEST(SearchFileWatcherTest, ut_onFileDeleted)
{
    SearchFileWatcher watcher(SearchHelper::rootUrl());
    EXPECT_NO_FATAL_FAILURE(watcher.onFileDeleted(QUrl()));
}

TEST(SearchFileWatcherTest, ut_onFileAttributeChanged)
{
    SearchFileWatcher watcher(SearchHelper::rootUrl());
    EXPECT_NO_FATAL_FAILURE(watcher.onFileAttributeChanged(QUrl()));
}

TEST(SearchFileWatcherTest, ut_onFileRenamed)
{
    auto fromUrl = QUrl::fromUserInput("/home/test");
    auto toUrl = QUrl::fromUserInput("/home/test123");

    stub_ext::StubExt st;
    st.set_lamda(&SyncFileInfoPrivate::init, [] {});
    st.set_lamda(&InfoFactory::create<FileInfo>, [&toUrl] {
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(toUrl));
    });
    st.set_lamda(VADDR(SyncFileInfo, displayOf), [] {
        return "test123";
    });

    auto rootUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/"), "test", "123");
    SearchFileWatcher watcher(rootUrl);
    EXPECT_NO_FATAL_FAILURE(watcher.onFileRenamed(fromUrl, toUrl));
}

TEST(SearchFileWatcherPrivateTest, ut_start)
{
    stub_ext::StubExt st;
    st.set_lamda(&LocalFileWatcherPrivate::initFileWatcher, [] {});
    st.set_lamda(&LocalFileWatcherPrivate::initConnect, [] {});
    st.set_lamda(VADDR(LocalFileWatcher, startWatcher), [] { return true; });

    SearchFileWatcher watcher(SearchHelper::rootUrl());
    auto url = QUrl::fromLocalFile("/home");
    auto w = QSharedPointer<LocalFileWatcher>(new LocalFileWatcher(url));
    watcher.dptr->urlToWatcherHash.insert(url, w);

    EXPECT_TRUE(watcher.dptr->start());
}

TEST(SearchFileWatcherPrivateTest, ut_stop)
{
    stub_ext::StubExt st;
    st.set_lamda(&LocalFileWatcherPrivate::initFileWatcher, [] {});
    st.set_lamda(&LocalFileWatcherPrivate::initConnect, [] {});
    st.set_lamda(VADDR(LocalFileWatcher, stopWatcher), [] { return true; });

    SearchFileWatcher watcher(SearchHelper::rootUrl());
    auto url = QUrl::fromLocalFile("/home");
    auto w = QSharedPointer<LocalFileWatcher>(new LocalFileWatcher(url));
    watcher.dptr->urlToWatcherHash.insert(url, w);

    EXPECT_TRUE(watcher.dptr->stop());
}

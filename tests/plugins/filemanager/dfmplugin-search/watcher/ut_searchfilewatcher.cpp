/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liuzhangjian<liuzhangjian@uniontech.com>
 *
 * Maintainer: liuzhangjian<liuzhangjian@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "watcher/searchfilewatcher.h"
#include "watcher/searchfilewatcher_p.h"
#include "utils/searchhelper.h"

#include "dfm-base/base/schemefactory.h"
#include "dfm-base/file/local/localfilewatcher.h"
#include "dfm-base/file/local/private/localfilewatcher_p.h"
#include "dfm-base/file/local/localfileinfo.h"

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
    st.set_lamda(&LocalFileWatcher::moveToThread, [] {});
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
    st.set_lamda(&LocalFileInfo::init, [] {});
    st.set_lamda(&InfoFactory::create<AbstractFileInfo>, [&toUrl] {
        return QSharedPointer<LocalFileInfo>(new LocalFileInfo(toUrl));
    });
    st.set_lamda(VADDR(LocalFileInfo, displayOf), [] {
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

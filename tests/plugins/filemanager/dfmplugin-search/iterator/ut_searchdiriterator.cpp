// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "iterator/searchdiriterator.h"
#include "iterator/searchdiriterator_p.h"
#include "utils/searchhelper.h"
#include "utils/custommanager.h"
#include "events/searcheventcaller.h"
#include "searchmanager/searchmanager.h"

#include <dfm-base/file/local/localfilewatcher.h>
#include <dfm-base/file/local/private/localfilewatcher_p.h>
#include <dfm-base/file/local/private/syncfileinfo_p.h>
#include <dfm-base/base/schemefactory.h>

#include "stubext.h"

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPSEARCH_USE_NAMESPACE

TEST(SearchDirIteratorTest, ut_next)
{
    SearchDirIterator iterator({});

    auto retUrl = iterator.next();
    EXPECT_FALSE(retUrl.isValid());

    iterator.d->childrens.append(QUrl::fromLocalFile("/home"));
    retUrl = iterator.next();
    EXPECT_TRUE(retUrl.isValid());
}

TEST(SearchDirIteratorTest, ut_hasNext)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchDirIteratorPrivate::doSearch, [] { return; });
    st.set_lamda(&SearchEventCaller::sendStopSpinner, [] { return; });

    SearchDirIterator iterator({});
    iterator.d->searchFinished = true;
    iterator.d->childrens.append(QUrl::fromLocalFile("/"));
    EXPECT_TRUE(iterator.hasNext());

    iterator.d->searchStoped = true;
    EXPECT_FALSE(iterator.hasNext());
}

TEST(SearchDirIteratorTest, ut_fileName)
{
    stub_ext::StubExt st;
    st.set_lamda(&SyncFileInfoPrivate::init, [] {});
    st.set_lamda(&InfoFactory::create<FileInfo>, [] {
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl::fromLocalFile("/home")));
    });
    st.set_lamda(VADDR(SyncFileInfoPrivate, fileName), [] {
        return "/home";
    });

    SearchDirIterator it({});
    it.d->currentFileUrl = QUrl::fromLocalFile("/home");
    auto filename = it.fileName();
    EXPECT_FALSE(filename.isEmpty());
}

TEST(SearchDirIteratorTest, ut_fileUrl)
{
    SearchDirIterator it({});
    it.d->currentFileUrl = QUrl::fromLocalFile("/");
    auto url = it.fileUrl();
    EXPECT_TRUE(url.isValid());
}

TEST(SearchDirIteratorTest, ut_url)
{
    SearchDirIterator it({});
    auto url = it.url();
    EXPECT_EQ(url, SearchHelper::rootUrl());
}

TEST(SearchDirIteratorTest, ut_close)
{
    SearchDirIterator it({});
    EXPECT_NO_FATAL_FAILURE(it.close());

    it.d->taskId = 123;
    stub_ext::StubExt st;
    typedef void (SearchManager::*Stop)(const QString &);
    auto stop = static_cast<Stop>(&SearchManager::stop);
    st.set_lamda(stop, [] {});

    EXPECT_NO_FATAL_FAILURE(it.close());
}

TEST(SearchDirIteratorPrivateTest, ut_doSearch)
{
    const auto &searchUrl = SearchHelper::fromSearchFile(QUrl::fromLocalFile("/home"), "test", "123");
    SearchDirIterator it(searchUrl);

    stub_ext::StubExt st;
    st.set_lamda(&LocalFileWatcherPrivate::initFileWatcher, [] {});
    st.set_lamda(&LocalFileWatcherPrivate::initConnect, [] {});
    st.set_lamda(&CustomManager::isDisableSearch, [] { return false; });
    st.set_lamda(&CustomManager::redirectedPath, [] { return "/home"; });
    st.set_lamda(&SearchEventCaller::sendStartSpinner, [] {});
    st.set_lamda(&SearchManager::search, [] { return true; });

    EXPECT_NO_FATAL_FAILURE(it.d->doSearch());
}

TEST(SearchDirIteratorPrivateTest, ut_onMatched)
{
    SearchDirIterator it({});
    it.d->taskId = "123";

    stub_ext::StubExt st;
    st.set_lamda(&SearchManager::matchedResults, [] {
        return QList<QUrl>() << QUrl();
    });
    st.set_lamda(&SearchEventCaller::sendShowAdvanceSearchButton, [] {});

    EXPECT_NO_FATAL_FAILURE(it.d->onMatched("123"));
}

TEST(SearchDirIteratorPrivateTest, ut_onSearchCompleted)
{
    SearchDirIterator it({});
    it.d->taskId = "123";

    it.d->onSearchCompleted("123");
    EXPECT_TRUE(it.d->searchFinished);
}

TEST(SearchDirIteratorPrivateTest, ut_onSearchStoped)
{
    SearchDirIterator it({});
    it.d->taskId = "123";

    stub_ext::StubExt st;
    st.set_lamda(&SearchEventCaller::sendStopSpinner, [] {});

    it.d->onSearchStoped("123");
    EXPECT_TRUE(it.d->searchStoped);
}

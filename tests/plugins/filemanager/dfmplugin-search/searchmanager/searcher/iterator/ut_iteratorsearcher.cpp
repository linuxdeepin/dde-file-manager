// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmanager/searcher/iterator/iteratorsearcher.h"
#include "stubext.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <gtest/gtest.h>

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(IteratorSearcherTest, ut_search_1)
{
    IteratorSearcher search(QUrl::fromLocalFile("/home"), "key");
    search.status.storeRelease(AbstractSearcher::kRuning);

    EXPECT_FALSE(search.search());
}

TEST(IteratorSearcherTest, ut_search_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&IteratorSearcher::doSearch, [] { __DBG_STUB_INVOKE__ });

    IteratorSearcher search(QUrl::fromLocalFile("/home"), "key");
    search.allResults << QUrl::fromLocalFile("/home");

    EXPECT_TRUE(search.search());
}

TEST(IteratorSearcherTest, stop)
{
    IteratorSearcher search(QUrl::fromLocalFile("/home"), "key");
    search.stop();

    EXPECT_EQ(search.status.loadAcquire(), AbstractSearcher::kTerminated);
}

TEST(IteratorSearcherTest, hasItem)
{
    IteratorSearcher search(QUrl::fromLocalFile("/home"), "key");

    EXPECT_FALSE(search.hasItem());
}

TEST(IteratorSearcherTest, takeAll)
{
    IteratorSearcher search(QUrl::fromLocalFile("/home"), "key");
    auto result = search.takeAll();

    EXPECT_TRUE(result.isEmpty());
}

TEST(IteratorSearcherTest, tryNotify)
{
    stub_ext::StubExt st;
    st.set_lamda(&QTime::elapsed, [] { __DBG_STUB_INVOKE__ return 100; });

    IteratorSearcher search(QUrl::fromLocalFile("/home"), "key");
    search.allResults << QUrl::fromLocalFile("/home");
    search.tryNotify();

    EXPECT_EQ(search.lastEmit, 100);
}

TEST(IteratorSearcherTest, doSearch_1)
{
    IteratorSearcher search(QUrl::fromLocalFile("/home"), "key");
    search.doSearch();

    EXPECT_NE(search.status.loadAcquire(), AbstractSearcher::kRuning);
}

TEST(IteratorSearcherTest, doSearch_2)
{
    DirIteratorFactory::regClass<LocalDirIterator>("file");
    IteratorSearcher search(QUrl::fromLocalFile("/home"), "home");
    search.status.storeRelease(AbstractSearcher::kRuning);

    stub_ext::StubExt st;
    bool hasNext = true;
    st.set_lamda(VADDR(LocalDirIterator, hasNext), [&] {
        __DBG_STUB_INVOKE__
        return hasNext;
    });
    st.set_lamda(VADDR(LocalDirIterator, next), [&] {
        __DBG_STUB_INVOKE__
        hasNext = false;
        return QUrl::fromLocalFile("/home");
    });
    st.set_lamda(VADDR(LocalDirIterator, fileInfo), [] { __DBG_STUB_INVOKE__ return FileInfoPointer(new SyncFileInfo(QUrl::fromLocalFile("/home"))); });

    UrlRoute::regScheme("file", "/");
    search.doSearch();

    EXPECT_FALSE(hasNext);
    EXPECT_FALSE(search.allResults.isEmpty());
    EXPECT_TRUE(search.searchPathList.isEmpty());
}

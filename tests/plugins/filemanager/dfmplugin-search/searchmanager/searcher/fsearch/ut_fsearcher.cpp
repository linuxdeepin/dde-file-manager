// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "searchmanager/searcher/fsearch/fsearcher.h"
#include "searchmanager/searcher/fsearch/fsearchhandler.h"
#include "utils/searchhelper.h"

#include "stubext.h"

#include <dfm-base/base/urlroute.h>

#include <gtest/gtest.h>

#include <QElapsedTimer>

DPSEARCH_USE_NAMESPACE
DFMBASE_USE_NAMESPACE

TEST(FSearcherTest, ut_isSupport_1)
{
    UrlRoute::regScheme("recent", "/", {}, true);
    EXPECT_FALSE(FSearcher::isSupport(QUrl("recent:///")));
}

TEST(FSearcherTest, ut_isSupport_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&FSearchHandler::checkPathSearchable, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_TRUE(FSearcher::isSupport(QUrl::fromLocalFile("/")));
}

TEST(FSearcherTest, ut_search_1)
{
    FSearcher searcher(QUrl::fromLocalFile("/"), "test");
    EXPECT_FALSE(searcher.search());
}

TEST(FSearcherTest, ut_search_2)
{
    UrlRoute::regScheme("file", "/");
    FSearcher searcher(QUrl::fromLocalFile("/"), "test");

    stub_ext::StubExt st;
    st.set_lamda(&FSearchHandler::loadDatabase, [] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(&FSearchHandler::search, [&] { __DBG_STUB_INVOKE__ return true; });
    st.set_lamda(VADDR(FSearcher, hasItem), [] { __DBG_STUB_INVOKE__ return true; });

    typedef bool (QWaitCondition::*Wait)(QMutex *, unsigned long);
    auto wait = static_cast<Wait>(&QWaitCondition::wait);
    st.set_lamda(wait, [] { __DBG_STUB_INVOKE__ return true; });

    EXPECT_TRUE(searcher.search());
    EXPECT_EQ(searcher.status.loadAcquire(), AbstractSearcher::kCompleted);
}

TEST(FSearcherTest, ut_stop)
{
    stub_ext::StubExt st;
    st.set_lamda(&FSearchHandler::stop, [] { __DBG_STUB_INVOKE__ });

    FSearcher searcher(QUrl::fromLocalFile("/"), "test");
    searcher.stop();

    EXPECT_EQ(searcher.status.loadAcquire(), AbstractSearcher::kTerminated);
}

TEST(FSearcherTest, ut_hasItem)
{
    FSearcher searcher(QUrl::fromLocalFile("/"), "test");
    EXPECT_FALSE(searcher.hasItem());
}

TEST(FSearcherTest, ut_takeAll)
{
    FSearcher searcher(QUrl::fromLocalFile("/"), "test");
    searcher.allResults << QUrl::fromLocalFile("/home");

    auto all = searcher.takeAll();
    EXPECT_FALSE(all.isEmpty());
    EXPECT_TRUE(searcher.allResults.isEmpty());
}

TEST(FSearcherTest, ut_tryNotify)
{
    stub_ext::StubExt st;
    typedef qint64 (QElapsedTimer::*Elapsed)() const;
    auto elapsed = static_cast<Elapsed>(&QElapsedTimer::elapsed);
    st.set_lamda(elapsed, [] { __DBG_STUB_INVOKE__ return 100; });

    FSearcher searcher(QUrl::fromLocalFile("/"), "test");
    searcher.allResults << QUrl::fromLocalFile("/home");
    searcher.tryNotify();

    EXPECT_EQ(searcher.lastEmit, 100);
}

TEST(FSearcherTest, ut_receiveResultCallback_1)
{
    FSearcher searcher(QUrl::fromLocalFile("/"), "test");
    searcher.receiveResultCallback("/home", false, &searcher);

    EXPECT_NE(searcher.status.loadAcquire(), AbstractSearcher::kRuning);
}

TEST(FSearcherTest, ut_receiveResultCallback_2)
{
    stub_ext::StubExt st;
    st.set_lamda(&SearchHelper::isHiddenFile, [] { __DBG_STUB_INVOKE__ return false; });

    FSearcher searcher(QUrl::fromLocalFile("/"), "test");
    searcher.status.storeRelease(AbstractSearcher::kRuning);

    searcher.receiveResultCallback("/home", true, &searcher);

    EXPECT_TRUE(searcher.allResults.isEmpty());
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QSignalSpy>
#include <QUrl>
#include <QString>

#include "stubext.h"

#include <dfm-search/dsearch_global.h>
#include <dfm-base/base/urlroute.h>

#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"
#include "searchmanager/searcher/abstractsearcher.h"

using namespace dfmplugin_search;
DFMBASE_USE_NAMESPACE

class DFMSearcherTest : public testing::Test
{
protected:
    stub_ext::StubExt stub;

    void TearDown() override
    {
        stub.clear();
    }
};

// --- supportUrl (static, pure logic) ---

TEST_F(DFMSearcherTest, SupportUrl_FileScheme_ReturnsTrue)
{
    EXPECT_TRUE(DFMSearcher::supportUrl(QUrl("file:///home")));
}

TEST_F(DFMSearcherTest, SupportUrl_NonFileScheme_ReturnsFalse)
{
    EXPECT_FALSE(DFMSearcher::supportUrl(QUrl("trash:///")));
}

TEST_F(DFMSearcherTest, SupportUrl_RemoteScheme_ReturnsFalse)
{
    EXPECT_FALSE(DFMSearcher::supportUrl(QUrl("smb:///share")));
}

TEST_F(DFMSearcherTest, SupportUrl_EmptyUrl_ReturnsFalse)
{
    EXPECT_FALSE(DFMSearcher::supportUrl(QUrl()));
}

TEST_F(DFMSearcherTest, SupportUrl_FtpScheme_ReturnsFalse)
{
    EXPECT_FALSE(DFMSearcher::supportUrl(QUrl("ftp:///host")));
}

// --- matchPath (static) ---

TEST_F(DFMSearcherTest, MatchPath_NonEmptyPath_NoCrash)
{
    QString result = DFMSearcher::matchPath("/home/user");
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(DFMSearcherTest, MatchPath_EmptyPath_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(DFMSearcher::matchPath(""));
}

// --- realSearchPath (static) ---

TEST_F(DFMSearcherTest, RealSearchPath_FileUrl_NoCrash)
{
    EXPECT_NO_FATAL_FAILURE(DFMSearcher::realSearchPath(QUrl("file:///home/user")));
}

// PMS:320351 mips 无 anything 索引时内容搜索无结果且不结束：全文检索启动时未校验搜索目录是否在索引目录内
TEST_F(DFMSearcherTest, BUG320351_Search_Content_NonIndexedPath_EmitsFinishedImmediately)
{
    stub.set_lamda(&DFMSEARCH::Global::isFileNameIndexReadyForSearch, []() { return true; });
    stub.set_lamda(&DFMSEARCH::Global::isPathInFileNameIndexDirectory, [](const QString &) { return false; });
    stub.set_lamda(qOverload<const QUrl &>(&UrlRoute::urlToPath), [](const QUrl &url) { return url.toLocalFile(); });

    DFMSearcher searcher(QUrl::fromLocalFile("/tmp/ut-non-indexed-dir"), "kw", nullptr, DFMSEARCH::SearchType::Content);
    QSignalSpy spy(&searcher, &AbstractSearcher::finished);

    const bool ret = searcher.search();

    EXPECT_TRUE(ret);
    EXPECT_EQ(spy.count(), 1);
}

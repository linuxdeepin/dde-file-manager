// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QString>

#include "stubext.h"

#include "searchmanager/searcher/dfmsearch/dfmsearcher.h"
#include "searchmanager/searcher/abstractsearcher.h"

using namespace dfmplugin_search;

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

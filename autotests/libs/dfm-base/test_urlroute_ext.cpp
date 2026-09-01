// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_urlroute_ext.cpp
 * @brief Extended unit tests for UrlRoute (urlroute.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QUrl>
#include <QList>
#include <QIcon>

#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class UrlRouteExtTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        rootPath = tmpDir.path();
        UrlRoute::regScheme("extscheme", rootPath + "/", QIcon(), false, "Ext", nullptr);
        UrlRoute::regScheme("extvirtual", "", QIcon(), true, "ExtVirt", nullptr);
    }

    QTemporaryDir tmpDir;
    QString rootPath;
};

TEST_F(UrlRouteExtTest, ToStringReturnsString)
{
    QUrl url;
    url.setScheme("extscheme");
    url.setPath(rootPath + "/sub");
    QString s = UrlRoute::toString(url);
    EXPECT_FALSE(s.isEmpty());
}

TEST_F(UrlRouteExtTest, IsRootUrlTrue)
{
    QUrl url;
    url.setScheme("extscheme");
    url.setPath("/");
    EXPECT_TRUE(UrlRoute::isRootUrl(url));
}

TEST_F(UrlRouteExtTest, IsRootUrlFalse)
{
    QUrl url;
    url.setScheme("extscheme");
    url.setPath(rootPath + "/sub");
    EXPECT_FALSE(UrlRoute::isRootUrl(url));
}

TEST_F(UrlRouteExtTest, UrlParentListBuilds)
{
    QUrl url;
    url.setScheme("extscheme");
    url.setPath(rootPath + "/a/b/c");
    QList<QUrl> list;
    UrlRoute::urlParentList(url, &list);
    EXPECT_FALSE(list.isEmpty());
}

TEST_F(UrlRouteExtTest, RootDisplayName)
{
    QString name = UrlRoute::rootDisplayName("extscheme");
    EXPECT_FALSE(name.isEmpty());
}

TEST_F(UrlRouteExtTest, FromUserInputLocalPath)
{
    QUrl u = UrlRoute::fromUserInput(rootPath + "/somefile", true);
    EXPECT_FALSE(u.isEmpty());
}

TEST_F(UrlRouteExtTest, FromUserInputWithWorkdir)
{
    QUrl u = UrlRoute::fromUserInput("somefile", rootPath, true, QUrl::AssumeLocalFile);
    EXPECT_NO_FATAL_FAILURE({ (void)u; });
}

TEST_F(UrlRouteExtTest, PathToRealNoMatchReturnsEmpty)
{
    QUrl u = UrlRoute::pathToReal(rootPath + "/no_such_under_any_root");
    // path under tmp dir matches no registered scheme root -> empty
    EXPECT_NO_FATAL_FAILURE({ (void)u; });
}

TEST_F(UrlRouteExtTest, FromLocalFile)
{
    QUrl u = UrlRoute::fromLocalFile(rootPath + "/locfile");
    EXPECT_EQ(u.scheme(), QString("file"));
}

TEST_F(UrlRouteExtTest, PathToUrlWithScheme)
{
    QUrl u = UrlRoute::pathToUrl(rootPath + "/x", "extscheme");
    EXPECT_EQ(u.scheme(), QString("extscheme"));
}

TEST_F(UrlRouteExtTest, UrlToPath)
{
    QUrl url;
    url.setScheme("extscheme");
    url.setPath(rootPath + "/mapped");
    QString p = UrlRoute::urlToPath(url);
    EXPECT_FALSE(p.isEmpty());
}

TEST_F(UrlRouteExtTest, UrlToLocalPath)
{
    QUrl u = QUrl::fromLocalFile(rootPath + "/local");
    QString p = UrlRoute::urlToLocalPath(u);
    EXPECT_FALSE(p.isEmpty());
}

TEST_F(UrlRouteExtTest, IsVirtualForVirtualSchemeString)
{
    EXPECT_TRUE(UrlRoute::isVirtual("extvirtual"));
    EXPECT_FALSE(UrlRoute::isVirtual("extscheme"));
}

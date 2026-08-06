// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_urlroute.cpp
 * @brief Unit tests for UrlRoute (urlroute.cpp)
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QUrl>
#include <QIcon>

#include <dfm-base/base/urlroute.h>
#include <dfm-base/dfm_global_defines.h>

using namespace dfmbase;

class UrlRouteTest : public testing::Test
{
protected:
    void SetUp() override
    {
        tmpDir.isValid();
        rootPath = tmpDir.path();
        // Ensure root path ends without trailing slash for regScheme formatting
    }

    QTemporaryDir tmpDir;
    QString rootPath;
};

TEST_F(UrlRouteTest, HasSchemeUnregisteredReturnsFalse)
{
    EXPECT_FALSE(UrlRoute::hasScheme("myscheme_unregistered_xyz"));
}

TEST_F(UrlRouteTest, RegisterVirtualSchemeSucceeds)
{
    QString err;
    bool ok = UrlRoute::regScheme("testvirtualscheme", "", QIcon(), true, "TestVirtual", &err);
    EXPECT_TRUE(ok);
    EXPECT_TRUE(UrlRoute::hasScheme("testvirtualscheme"));
}

TEST_F(UrlRouteTest, RegisterDuplicateSchemeFails)
{
    QString err;
    UrlRoute::regScheme("testdup", "", QIcon(), true, "TestDup", &err);
    bool ok = UrlRoute::regScheme("testdup", "", QIcon(), true, "TestDup2", &err);
    EXPECT_FALSE(ok);
}

TEST_F(UrlRouteTest, RegisterNonVirtualWithExistingRoot)
{
    QString err;
    bool ok = UrlRoute::regScheme("testlocal", rootPath + "/", QIcon(), false, "TestLocal", &err);
    EXPECT_TRUE(ok);
}

TEST_F(UrlRouteTest, RegisterNonVirtualWithNonExistentRootFails)
{
    QString err;
    bool ok = UrlRoute::regScheme("testbadroot", "/no/such/path/xyz/", QIcon(), false, "TestBad", &err);
    EXPECT_FALSE(ok);
}

TEST_F(UrlRouteTest, IsVirtualForVirtualScheme)
{
    UrlRoute::regScheme("testv2", "", QIcon(), true, "V2", nullptr);
    QUrl url;
    url.setScheme("testv2");
    url.setPath("/");
    EXPECT_TRUE(UrlRoute::isVirtual(url));
}

TEST_F(UrlRouteTest, IsVirtualForUnregisteredReturnsFalse)
{
    QUrl url;
    url.setScheme("nosuchschemev");
    url.setPath("/");
    EXPECT_FALSE(UrlRoute::isVirtual(url));
}

TEST_F(UrlRouteTest, IsVirtualBySchemeString)
{
    UrlRoute::regScheme("testv3", "", QIcon(), true, "V3", nullptr);
    EXPECT_TRUE(UrlRoute::isVirtual("testv3"));
    EXPECT_FALSE(UrlRoute::isVirtual("testv3_missing"));
}

TEST_F(UrlRouteTest, RootPathForRegisteredScheme)
{
    UrlRoute::regScheme("testroot", rootPath + "/", QIcon(), false, "TestRoot", nullptr);
    QString rp = UrlRoute::rootPath("testroot");
    EXPECT_FALSE(rp.isEmpty());
}

TEST_F(UrlRouteTest, RootUrlForRegisteredScheme)
{
    UrlRoute::regScheme("testrooturl", rootPath + "/", QIcon(), false, "TestRootUrl", nullptr);
    QUrl u = UrlRoute::rootUrl("testrooturl");
    EXPECT_EQ(u.scheme(), QString("testrooturl"));
}

TEST_F(UrlRouteTest, UrlParentOfChild)
{
    UrlRoute::regScheme("testparent", rootPath + "/", QIcon(), false, "TestParent", nullptr);
    QUrl child;
    child.setScheme("testparent");
    child.setPath(UrlRoute::rootPath("testparent") + "/sub");
    QUrl parent = UrlRoute::urlParent(child);
    EXPECT_EQ(parent.scheme(), QString("testparent"));
}

// ---- Coverage additions for previously-uncovered UrlRoute / SchemeNode API ----

TEST_F(UrlRouteTest, IconForUnregisteredSchemeReturnsNullIcon)
{
    // Unregistered scheme hits the early-return branch.
    EXPECT_TRUE(UrlRoute::icon("scheme_not_registered_zzz").isNull());
}

TEST_F(UrlRouteTest, IconForRegisteredSchemeReturnsStoredIcon)
{
    QString err;
    ASSERT_TRUE(UrlRoute::regScheme("uticonscheme", rootPath + "/", QIcon(), false, "UtIcon", &err));
    // Registered scheme hits the lookup branch (stored icon is null here).
    QIcon ic = UrlRoute::icon("uticonscheme");
    EXPECT_TRUE(ic.isNull());
}

TEST_F(UrlRouteTest, FromStringListConvertsStringsToUrls)
{
    const QStringList strs { "file:///tmp", "file:///home" };
    const QList<QUrl> urls = UrlRoute::fromStringList(strs);
    EXPECT_EQ(urls.size(), 2);
    EXPECT_EQ(urls.at(0).scheme().toStdString(), "file");
    EXPECT_EQ(urls.at(1).scheme().toStdString(), "file");
}

TEST_F(UrlRouteTest, UrlsToByteArrayRoundTripsThroughByteArrayToUrls)
{
    QList<QUrl> original;
    original << QUrl("file:///a/1") << QUrl("file:///b/2") << QUrl("file:///c/3");
    const QByteArray blob = UrlRoute::urlsToByteArray(original);
    EXPECT_FALSE(blob.isEmpty());
    const QList<QUrl> restored = UrlRoute::byteArrayToUrls(blob);
    EXPECT_EQ(restored.size(), original.size());
    for (int i = 0; i < original.size(); ++i)
        EXPECT_EQ(restored.at(i).toString(), original.at(i).toString());
}

TEST_F(UrlRouteTest, IsAncestorsUrlReturnsTrueWhenAncestorInChain)
{
    QString err;
    ASSERT_TRUE(UrlRoute::regScheme("utanc", rootPath + "/", QIcon(), false, "UtAnc", &err));
    QUrl child;
    child.setScheme("utanc");
    child.setPath(UrlRoute::rootPath("utanc") + "a/b");
    QUrl ancestor;
    ancestor.setScheme("utanc");
    ancestor.setPath(UrlRoute::rootPath("utanc") + "a");
    QList<QUrl> list;
    EXPECT_TRUE(UrlRoute::isAncestorsUrl(child, ancestor, &list));
    EXPECT_FALSE(list.isEmpty());
}

TEST_F(UrlRouteTest, IsAncestorsUrlReturnsFalseWhenAncestorNotInChain)
{
    QString err;
    ASSERT_TRUE(UrlRoute::regScheme("utanc2", rootPath + "/", QIcon(), false, "UtAnc2", &err));
    QUrl child;
    child.setScheme("utanc2");
    child.setPath(UrlRoute::rootPath("utanc2") + "a/b");
    QUrl stranger;
    stranger.setScheme("utanc2");
    stranger.setPath(UrlRoute::rootPath("utanc2") + "x");
    QList<QUrl> list;
    EXPECT_FALSE(UrlRoute::isAncestorsUrl(child, stranger, &list));
}

TEST_F(UrlRouteTest, SchemeNodeDefaultIsEmpty)
{
    SchemeNode node;
    EXPECT_TRUE(node.isEmpty());
}

TEST_F(UrlRouteTest, SchemeNodeWithPathIsNotEmpty)
{
    SchemeNode node(rootPath);
    EXPECT_FALSE(node.isEmpty());
    EXPECT_EQ(node.rootPath().toStdString(), rootPath.toStdString());
}

TEST_F(UrlRouteTest, SchemeNodeOperatorAssignCopiesPathAndVirtualFlag)
{
    SchemeNode src(rootPath, QIcon(), true, "srcname");
    SchemeNode dst;
    dst = src;
    EXPECT_EQ(dst.rootPath().toStdString(), rootPath.toStdString());
    EXPECT_TRUE(dst.isVirtual());
}

TEST_F(UrlRouteTest, SchemeNodePathIconReturnsStoredIcon)
{
    SchemeNode node(rootPath, QIcon());
    EXPECT_TRUE(node.pathIcon().isNull());
}

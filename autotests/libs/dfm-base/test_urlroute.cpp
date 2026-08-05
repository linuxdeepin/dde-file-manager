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

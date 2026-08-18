// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "utils/shareutils.h"
#include "dfmplugin_myshares_global.h"

#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QIcon>
#include <QString>
#include <QObject>

using namespace dfmplugin_myshares;

class UT_ShareUtils : public testing::Test
{
    // Test interface
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(UT_ShareUtils, Scheme_ReturnsUserShare)
{
    EXPECT_EQ(QString("usershare"), ShareUtils::scheme());
}

TEST_F(UT_ShareUtils, Instance_ReturnsSameSingleton)
{
    ShareUtils *first = ShareUtils::instance();
    ASSERT_TRUE(first != nullptr);
    EXPECT_EQ(first, ShareUtils::instance());
}

TEST_F(UT_ShareUtils, Icon_ReturnsThemedIcon)
{
    const QString name = ShareUtils::icon().themeName();
    EXPECT_TRUE(name == "folder-publicshare" || name.isEmpty());
}

TEST_F(UT_ShareUtils, DisplayName_ReturnsMyShares)
{
    EXPECT_EQ(QObject::tr("My Shares"), ShareUtils::displayName());
}

TEST_F(UT_ShareUtils, RootUrl_ReturnsUserShareRoot)
{
    EXPECT_EQ(QString("usershare"), ShareUtils::rootUrl().scheme());
    EXPECT_EQ(QString("/"), ShareUtils::rootUrl().path());
}

TEST_F(UT_ShareUtils, MakeShareUrl_AbsolutePath_SetsSchemeAndPath)
{
    QString path = "/hello/world";
    auto url = ShareUtils::makeShareUrl(path);
    EXPECT_EQ(QString("usershare"), url.scheme());
    EXPECT_EQ(path, url.path());
}

TEST_F(UT_ShareUtils, MakeShareUrl_RelativePath_KeepsPath)
{
    QString path = "this/is/share/url";
    auto url = ShareUtils::makeShareUrl(path);
    EXPECT_EQ(QString("usershare"), url.scheme());
    EXPECT_EQ(path, url.path());
}

TEST_F(UT_ShareUtils, MakeShareUrl_RootPath_EqualsRootUrl)
{
    EXPECT_EQ(ShareUtils::rootUrl(), ShareUtils::makeShareUrl("/"));
}

TEST_F(UT_ShareUtils, ConvertToLocalUrl_NonShareScheme_ReturnsInvalidUrl)
{
    const QUrl fileUrl("file:///hello/world");
    EXPECT_FALSE(ShareUtils::convertToLocalUrl(fileUrl).isValid());
}

TEST_F(UT_ShareUtils, ConvertToLocalUrl_ShareScheme_ConvertsToFileUrl)
{
    const QUrl shareUrl = ShareUtils::makeShareUrl("/hello/world");
    const QUrl localUrl = ShareUtils::convertToLocalUrl(shareUrl);
    EXPECT_EQ(QString("file"), localUrl.scheme());
    EXPECT_EQ(shareUrl.path(), localUrl.path());
}

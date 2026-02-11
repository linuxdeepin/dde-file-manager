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

TEST_F(UT_ShareUtils, Scheme)
{
    EXPECT_EQ("usershare", ShareUtils::scheme());
}

TEST_F(UT_ShareUtils, Icon)
{
    //    EXPECT_EQ("folder-publicshare", ShareUtils::icon().themeName());
    EXPECT_NO_FATAL_FAILURE(ShareUtils::icon().themeName());
}

TEST_F(UT_ShareUtils, DisplayName)
{
    EXPECT_EQ(QObject::tr("My Shares"), ShareUtils::displayName());
}

TEST_F(UT_ShareUtils, RootUrl)
{
    EXPECT_EQ("usershare", ShareUtils::rootUrl().scheme());
    EXPECT_EQ("/", ShareUtils::rootUrl().path());
}

TEST_F(UT_ShareUtils, MakeShareUrl)
{
    QString path = "/hello/world";
    auto url = ShareUtils::makeShareUrl(path);
    EXPECT_EQ("usershare", url.scheme());
    EXPECT_EQ(path, url.path());

    path = "this/is/share/url";
    url = ShareUtils::makeShareUrl(path);
    EXPECT_EQ(path, url.path());
}

TEST_F(UT_ShareUtils, ConvertToLocalUrl)
{
    auto localUrl = QUrl::fromLocalFile("");
    EXPECT_FALSE(ShareUtils::convertToLocalUrl(localUrl).isValid());

    auto testUrl = ShareUtils::makeShareUrl("/hello/world");
    localUrl = ShareUtils::convertToLocalUrl(testUrl);
    EXPECT_EQ("file", localUrl.scheme());
    EXPECT_EQ(testUrl.path(), localUrl.path());
}

TEST_F(UT_ShareUtils, UrlsToLocal)
{
    QList<QUrl> outputs, inputs;

    inputs = { ShareUtils::makeShareUrl("/hello"), ShareUtils::makeShareUrl("/world") };
    outputs.clear();
}

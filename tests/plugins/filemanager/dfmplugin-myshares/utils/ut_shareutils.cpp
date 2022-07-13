/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "plugins/filemanager/dfmplugin-myshares/utils/shareutils.h"

#include <gtest/gtest.h>

#include <QDebug>

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
    EXPECT_FALSE(ShareUtils::instance()->urlsToLocal({}, nullptr));

    QList<QUrl> outputs, inputs;
    inputs = { QUrl::fromLocalFile("/") };
    EXPECT_FALSE(ShareUtils::instance()->urlsToLocal(inputs, &outputs));

    inputs = { ShareUtils::makeShareUrl("hello"), QUrl("burn:///world") };
    EXPECT_FALSE(ShareUtils::instance()->urlsToLocal(inputs, &outputs));

    inputs = { ShareUtils::makeShareUrl("/hello"), ShareUtils::makeShareUrl("/world") };
    outputs.clear();
    EXPECT_TRUE(ShareUtils::instance()->urlsToLocal(inputs, &outputs));
    EXPECT_EQ(outputs.count(), 2);
}

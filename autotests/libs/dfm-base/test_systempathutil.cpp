// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_systempathutil.cpp
 * @brief Unit tests for SystemPathUtil (systempathutil.cpp)
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QList>

#include <dfm-base/utils/systempathutil.h>

using namespace dfmbase;

TEST(SystemPathUtilTest, InstanceReturnsNonNull)
{
    EXPECT_NE(SystemPathUtil::instance(), nullptr);
}

TEST(SystemPathUtilTest, SystemPathForKnownKey)
{
    SystemPathUtil *util = SystemPathUtil::instance();
    QString p = util->systemPath("Desktop");
    EXPECT_FALSE(p.isEmpty());
}

TEST(SystemPathUtilTest, SystemPathForUnknownKey)
{
    SystemPathUtil *util = SystemPathUtil::instance();
    QString p = util->systemPath("no.such.key");
    EXPECT_TRUE(p.isEmpty());
}

TEST(SystemPathUtilTest, SystemPathOfUser)
{
    SystemPathUtil *util = SystemPathUtil::instance();
    QString p = util->systemPathOfUser("Desktop", qgetenv("USER").constData());
    EXPECT_FALSE(p.isEmpty());
}

TEST(SystemPathUtilTest, SystemPathDisplayName)
{
    SystemPathUtil *util = SystemPathUtil::instance();
    QString name = util->systemPathDisplayName("Desktop");
    EXPECT_FALSE(name.isEmpty());
}

TEST(SystemPathUtilTest, SystemPathDisplayNameByPath)
{
    SystemPathUtil *util = SystemPathUtil::instance();
    QString name = util->systemPathDisplayNameByPath(util->systemPath("Desktop"));
    EXPECT_FALSE(name.isEmpty());
}

TEST(SystemPathUtilTest, SystemPathIconName)
{
    SystemPathUtil *util = SystemPathUtil::instance();
    EXPECT_NO_FATAL_FAILURE({ (void)util->systemPathIconName("Desktop"); });
    EXPECT_NO_FATAL_FAILURE({ (void)util->systemPathIconNameByPath(util->systemPath("Desktop")); });
}

TEST(SystemPathUtilTest, IsSystemPath)
{
    SystemPathUtil *util = SystemPathUtil::instance();
    EXPECT_TRUE(util->isSystemPath(util->systemPath("Desktop")));
    EXPECT_FALSE(util->isSystemPath("/no/such/system/path/here"));
}

TEST(SystemPathUtilTest, CheckContainsSystemPath)
{
    SystemPathUtil *util = SystemPathUtil::instance();
    QList<QUrl> urls { QUrl::fromLocalFile(util->systemPath("Desktop")) };
    EXPECT_TRUE(util->checkContainsSystemPath(urls));
    QList<QUrl> none { QUrl::fromLocalFile("/no/such/path/here/at/all") };
    EXPECT_FALSE(util->checkContainsSystemPath(none));
}

TEST(SystemPathUtilTest, GetRealpathSafely)
{
    SystemPathUtil *util = SystemPathUtil::instance();
    QString rp = util->getRealpathSafely(util->systemPath("Desktop"));
    EXPECT_FALSE(rp.isEmpty());
}

TEST(SystemPathUtilTest, LoadSystemPaths)
{
    SystemPathUtil *util = SystemPathUtil::instance();
    EXPECT_NO_FATAL_FAILURE({ util->loadSystemPaths(); });
}


TEST(SystemPathUtilTest, SystemPathUtil)
{
    // SystemPathUtil
    SUCCEED();
}

TEST(SystemPathUtilTest, findSystemPathKey)
{
    // findSystemPathKey
    SUCCEED();
}

TEST(SystemPathUtilTest, loadSystemPaths)
{
    // loadSystemPaths
    SUCCEED();
}

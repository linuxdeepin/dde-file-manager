// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_universalutils.cpp
 * @brief Unit tests for pure-logic functions of UniversalUtils
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QFontMetrics>
#include <QFont>
#include <QVariantMap>
#include <QVariantHash>

#include <dfm-base/utils/universalutils.h>

using namespace dfmbase;

TEST(UniversalUtilsTest, InMainThreadReturnsTrue)
{
    EXPECT_TRUE(UniversalUtils::inMainThread());
}

TEST(UniversalUtilsTest, SizeFormatBytes)
{
    QString unit;
    double val = UniversalUtils::sizeFormat(512, unit);
    EXPECT_EQ(unit, QString("B"));
    EXPECT_NEAR(val, 512.0, 0.01);
}

TEST(UniversalUtilsTest, SizeFormatKilobytes)
{
    QString unit;
    double val = UniversalUtils::sizeFormat(2048, unit);
    EXPECT_EQ(unit, QString("KB"));
    EXPECT_NEAR(val, 2.0, 0.01);
}

TEST(UniversalUtilsTest, SizeFormatMegabytes)
{
    QString unit;
    double val = UniversalUtils::sizeFormat(5 * 1024 * 1024, unit);
    EXPECT_EQ(unit, QString("MB"));
    EXPECT_NEAR(val, 5.0, 0.01);
}

TEST(UniversalUtilsTest, SizeFormatString)
{
    QString s = UniversalUtils::sizeFormat(1536, 2);
    EXPECT_EQ(s, QString("1.50 KB"));
}

TEST(UniversalUtilsTest, ConvertFromQMap)
{
    QVariantMap map;
    map.insert("a", 1);
    map.insert("b", QString("hello"));
    QVariantHash h = UniversalUtils::convertFromQMap(map);
    EXPECT_EQ(h.value("a").toInt(), 1);
    EXPECT_EQ(h.value("b").toString(), QString("hello"));
}

TEST(UniversalUtilsTest, UrlEqualsIdenticalUrls)
{
    EXPECT_TRUE(UniversalUtils::urlEquals(QUrl("file:///home/user"), QUrl("file:///home/user")));
}

TEST(UniversalUtilsTest, UrlEqualsTrailingSlashDifference)
{
    EXPECT_TRUE(UniversalUtils::urlEquals(QUrl("file:///home/user"), QUrl("file:///home/user/")));
}

TEST(UniversalUtilsTest, UrlEqualsDifferentSchemes)
{
    EXPECT_FALSE(UniversalUtils::urlEquals(QUrl("file:///home/user"), QUrl("trash:///home/user")));
}

TEST(UniversalUtilsTest, UrlEqualsInvalidUrl)
{
    EXPECT_FALSE(UniversalUtils::urlEquals(QUrl(""), QUrl("file:///home/user")));
}

TEST(UniversalUtilsTest, UrlEqualsWithQuerySameQuery)
{
    EXPECT_TRUE(UniversalUtils::urlEqualsWithQuery(
            QUrl("file:///home/user?k=1"), QUrl("file:///home/user?k=1")));
}

TEST(UniversalUtilsTest, UrlEqualsWithQueryDifferentQuery)
{
    EXPECT_FALSE(UniversalUtils::urlEqualsWithQuery(
            QUrl("file:///home/user?k=1"), QUrl("file:///home/user?k=2")));
}

TEST(UniversalUtilsTest, IsNetworkRootTrue)
{
    EXPECT_TRUE(UniversalUtils::isNetworkRoot(QUrl("network:///")));
}

TEST(UniversalUtilsTest, IsNetworkRootFalse)
{
    EXPECT_FALSE(UniversalUtils::isNetworkRoot(QUrl("file:///home")));
}

TEST(UniversalUtilsTest, IsParentUrlTrue)
{
    EXPECT_TRUE(UniversalUtils::isParentUrl(QUrl("file:///home/user/docs"),
                                            QUrl("file:///home/user")));
}

TEST(UniversalUtilsTest, IsParentUrlFalse)
{
    EXPECT_FALSE(UniversalUtils::isParentUrl(QUrl("file:///home/user"),
                                             QUrl("file:///home/other")));
}

TEST(UniversalUtilsTest, CovertUrlToLocalPathAbsolute)
{
    EXPECT_EQ(UniversalUtils::covertUrlToLocalPath("/home/user"), QString("/home/user"));
}

TEST(UniversalUtilsTest, CovertUrlToLocalPathFromUrl)
{
    EXPECT_EQ(UniversalUtils::covertUrlToLocalPath("file:///home/user"), QString("/home/user"));
}

TEST(UniversalUtilsTest, GetTextLineHeightEmptyText)
{
    QFont font;
    QFontMetrics fm(font);
    int h = UniversalUtils::getTextLineHeight(QString(""), fm);
    EXPECT_GT(h, 0);
}

TEST(UniversalUtilsTest, GetTextLineHeightNonEmpty)
{
    QFont font;
    QFontMetrics fm(font);
    int h = UniversalUtils::getTextLineHeight(QString("hello world"), fm);
    EXPECT_GT(h, 0);
}

TEST(UniversalUtilsTest, CheckDbusServiceUnregisteredReturnsFalse)
{
    EXPECT_FALSE(UniversalUtils::checkDbusService("org.nonexistent.service.12345", false));
    EXPECT_FALSE(UniversalUtils::checkDbusService("org.nonexistent.service.12345", true));
}

// ---- Coverage addition: getTextLineHeight(QModelIndex, QFontMetrics) ----

#include <QModelIndex>

TEST(UniversalUtilsTest, GetTextLineHeightWithInvalidIndexReturnsFontHeight)
{
    QFont f;
    QFontMetrics fm(f);
    QModelIndex idx;   // invalid index -> empty text -> returns font height
    int h = UniversalUtils::getTextLineHeight(idx, fm);
    EXPECT_GT(h, 0);
}

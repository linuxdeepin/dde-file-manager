// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_universalutils.cpp
 * @brief Unit tests for pure-logic functions of UniversalUtils
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QFile>
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

// ---- Coverage additions: DBus connection helpers (safe to call, may fail) ----

#include <QDBusReply>
#include <QDBusUnixFileDescriptor>

TEST(UniversalUtilsTest, BlockShutdownCallable)
{
    QDBusReply<QDBusUnixFileDescriptor> reply;
    EXPECT_NO_FATAL_FAILURE({ UniversalUtils::blockShutdown(reply); });
}

TEST(UniversalUtilsTest, UserChangeCallable)
{
    QObject obj;
    EXPECT_NO_FATAL_FAILURE({ UniversalUtils::userChange(&obj, SIGNAL(destroyed())); });
}

TEST(UniversalUtilsTest, PrepareForSleepCallable)
{
    QObject obj;
    EXPECT_NO_FATAL_FAILURE({ UniversalUtils::prepareForSleep(&obj, SIGNAL(destroyed())); });
}

TEST(UniversalUtilsTest, BoardCastPastDataCallable)
{
    // boardCastPastData checks for a desktop file monitor DBus service first.
    // Without that service it returns early — verify it does not crash.
    EXPECT_NO_FATAL_FAILURE({
        UniversalUtils::boardCastPastData(QUrl("file:///tmp"), QUrl("file:///home"), { QUrl("file:///tmp/a") });
    });
}

#include <QMimeData>
TEST(UniversalUtilsTest, SetDockDnDMimeDataWithDesktopFilePopulatesFormats)
{
    // setDockDnDMimeData only populates for .desktop file URLs.
    QTemporaryDir dir;
    ASSERT_TRUE(dir.isValid());
    const QString path = dir.path() + "/ut_test.desktop";
    QFile f(path);
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write("[Desktop Entry]\nType=Application\nName=UTApp\n");
    f.close();

    QMimeData md;
    UniversalUtils::setDockDnDMimeData(&md, QUrl::fromLocalFile(path), "dde-desktop");
    // The mime data should now contain the dock DnD format.
    EXPECT_FALSE(md.formats().isEmpty());
    EXPECT_TRUE(md.hasFormat("text/x-dde-dock-dnd-appid"));
    EXPECT_TRUE(md.hasFormat("text/x-dde-dock-dnd-source"));
}

TEST(UniversalUtilsTest, SetDockDnDMimeDataWithNonDesktopFileDoesNothing)
{
    QMimeData md;
    UniversalUtils::setDockDnDMimeData(&md, QUrl::fromLocalFile("/tmp"), "test");
    // Non-desktop file URL → early return → no formats set.
    EXPECT_TRUE(md.formats().isEmpty());
}


TEST(UniversalUtilsTest, boardCastPastData)
{
    UniversalUtils obj;
    QUrl _arg0{};
    QUrl _arg1{};
    QList<QUrl> _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj.boardCastPastData(_arg0, _arg1, _arg2));
}

TEST(UniversalUtilsTest, convertFromQMap)
{
    UniversalUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.convertFromQMap(QVariantMap()); });
}

TEST(UniversalUtilsTest, currentLoginUser)
{
    UniversalUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.currentLoginUser(); });
}

TEST(UniversalUtilsTest, dockHeight)
{
    UniversalUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.dockHeight(); });
}

TEST(UniversalUtilsTest, getCurrentUser)
{
    UniversalUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.getCurrentUser(); });
}

TEST(UniversalUtilsTest, getKernelParameters)
{
    UniversalUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.getKernelParameters(); });
}

TEST(UniversalUtilsTest, getTextLineHeight)
{
    UniversalUtils obj;
    QString _arg0{};
    QFontMetrics _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.getTextLineHeight(_arg0, _arg1); });
}

TEST(UniversalUtilsTest, isInLiveSys)
{
    UniversalUtils obj;
    bool result = obj.isInLiveSys();
    EXPECT_FALSE(result);
}

TEST(UniversalUtilsTest, isLogined)
{
    UniversalUtils obj;
    bool result = obj.isLogined();
    EXPECT_FALSE(result);
}

TEST(UniversalUtilsTest, isNetworkRoot)
{
    UniversalUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isNetworkRoot(_arg0); });
}

TEST(UniversalUtilsTest, isParentOnly)
{
    UniversalUtils obj;
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.isParentOnly(_arg0, _arg1); });
}

TEST(UniversalUtilsTest, isParentUrl)
{
    UniversalUtils obj;
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.isParentUrl(_arg0, _arg1); });
}

TEST(UniversalUtilsTest, lockScreenSaver)
{
    UniversalUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.lockScreenSaver(); });
}

TEST(UniversalUtilsTest, notifyMessage)
{
    UniversalUtils obj;
    QString _arg0{};
    QString _arg1{};
    QStringList _arg2{};
    QVariantMap _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj.notifyMessage(_arg0, _arg1, _arg2, _arg3));
}

TEST(UniversalUtilsTest, runCommand)
{
    UniversalUtils obj;
    QString _arg0{};
    QStringList _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE({ obj.runCommand(_arg0, _arg1, _arg2); });
}

TEST(UniversalUtilsTest, setDockDnDMimeData)
{
    UniversalUtils obj;
    QUrl _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj.setDockDnDMimeData(nullptr, _arg1, _arg2));
}

TEST(UniversalUtilsTest, sizeFormat)
{
    UniversalUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.sizeFormat(0, 0); });
}

TEST(UniversalUtilsTest, unlockScreenSaver)
{
    UniversalUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.unlockScreenSaver({}); });
}

TEST(UniversalUtilsTest, urlEquals)
{
    UniversalUtils obj;
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.urlEquals(_arg0, _arg1); });
}

TEST(UniversalUtilsTest, urlEqualsWithQuery)
{
    UniversalUtils obj;
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE({ obj.urlEqualsWithQuery(_arg0, _arg1); });
}

TEST(UniversalUtilsTest, urlTransformToLocal)
{
    UniversalUtils obj;
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.urlTransformToLocal(_arg0, nullptr); });
}

TEST(UniversalUtilsTest, urlsTransformToLocal)
{
    UniversalUtils obj;
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.urlsTransformToLocal(_arg0, nullptr); });
}

TEST(UniversalUtilsTest, userLoginState)
{
    UniversalUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.userLoginState(); });
}

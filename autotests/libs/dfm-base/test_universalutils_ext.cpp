// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_universalutils_ext.cpp
 * @brief Extended unit tests for UniversalUtils functions.
 */

#include <gtest/gtest.h>
#include <QUrl>
#include <QList>
#include <QFont>
#include <QFontMetrics>
#include <QModelIndex>
#include <QVariantMap>
#include <QVariantHash>
#include <QProcess>

#include <dfm-base/utils/universalutils.h>

using namespace dfmbase;

TEST(UniversalUtilsExtTest, NotifyMessageSingle)
{
    EXPECT_NO_FATAL_FAILURE({ UniversalUtils::notifyMessage(QString("hello")); });
}

TEST(UniversalUtilsExtTest, NotifyMessageTitleAndBody)
{
    EXPECT_NO_FATAL_FAILURE({ UniversalUtils::notifyMessage(QString("title"), QString("body")); });
}

TEST(UniversalUtilsExtTest, NotifyMessageFull)
{
    EXPECT_NO_FATAL_FAILURE({
        UniversalUtils::notifyMessage(QString("title"), QString("body"),
                                      QStringList { "default" }, QVariantMap {});
    });
}

TEST(UniversalUtilsExtTest, UserLoginState)
{
    EXPECT_NO_FATAL_FAILURE({ (void)UniversalUtils::userLoginState(); });
}

TEST(UniversalUtilsExtTest, CurrentLoginUser)
{
    EXPECT_NO_FATAL_FAILURE({ (void)UniversalUtils::currentLoginUser(); });
}

TEST(UniversalUtilsExtTest, IsLogined)
{
    EXPECT_NO_FATAL_FAILURE({ (void)UniversalUtils::isLogined(); });
}

TEST(UniversalUtilsExtTest, ComputerMemory)
{
    EXPECT_NO_FATAL_FAILURE({ (void)UniversalUtils::computerMemory(); });
}

TEST(UniversalUtilsExtTest, ComputerInformation)
{
    QString cpu, sys, edition, version;
    EXPECT_NO_FATAL_FAILURE({ UniversalUtils::computerInformation(cpu, sys, edition, version); });
}

TEST(UniversalUtilsExtTest, RunCommandTrue)
{
    EXPECT_TRUE(UniversalUtils::runCommand("true", QStringList {}));
}

TEST(UniversalUtilsExtTest, RunCommandFalse)
{
    // Non-existent executable: startDetached fails to launch
    EXPECT_FALSE(UniversalUtils::runCommand("/no/such/command/xyz", QStringList {}));
}

TEST(UniversalUtilsExtTest, DockHeight)
{
    EXPECT_NO_FATAL_FAILURE({ (void)UniversalUtils::dockHeight(); });
}

TEST(UniversalUtilsExtTest, GetKernelParameters)
{
    EXPECT_NO_FATAL_FAILURE({ (void)UniversalUtils::getKernelParameters(); });
}

TEST(UniversalUtilsExtTest, IsInLiveSys)
{
    EXPECT_NO_FATAL_FAILURE({ (void)UniversalUtils::isInLiveSys(); });
}

TEST(UniversalUtilsExtTest, UrlTransformToLocal)
{
    QUrl src("file:///home/user/docs");
    QUrl target;
    // file scheme returns false (already local)
    bool ok = UniversalUtils::urlTransformToLocal(src, &target);
    EXPECT_FALSE(ok);
    EXPECT_EQ(target, src);
}

TEST(UniversalUtilsExtTest, UrlsTransformToLocal)
{
    QList<QUrl> src { QUrl("file:///home/user/a"), QUrl("file:///home/user/b") };
    QList<QUrl> target;
    bool ok = UniversalUtils::urlsTransformToLocal(src, &target);
    EXPECT_FALSE(ok);
}

TEST(UniversalUtilsExtTest, GetCurrentUser)
{
    EXPECT_NO_FATAL_FAILURE({ (void)UniversalUtils::getCurrentUser(); });
}

TEST(UniversalUtilsExtTest, IsParentOnly)
{
    // isParentOnly returns true when child's parent dir equals the given parent
    EXPECT_TRUE(UniversalUtils::isParentOnly(QUrl("file:///home/user/docs"),
                                              QUrl("file:///home/user")));
    // "file:///home/user" parent dir is "/home", not "/home/user"
    EXPECT_FALSE(UniversalUtils::isParentOnly(QUrl("file:///home/user"),
                                             QUrl("file:///home/user")));
}

TEST(UniversalUtilsExtTest, SizeFormatDoubleZero)
{
    QString unit;
    double v = UniversalUtils::sizeFormat(0, unit);
    EXPECT_EQ(unit, QString("B"));
    EXPECT_NEAR(v, 0.0, 0.01);
}

TEST(UniversalUtilsExtTest, LockUnlockScreenSaver)
{
    uint32_t cookie = 0;
    EXPECT_NO_FATAL_FAILURE({ cookie = UniversalUtils::lockScreenSaver(); });
    EXPECT_NO_FATAL_FAILURE({ (void)UniversalUtils::unlockScreenSaver(cookie); });
}

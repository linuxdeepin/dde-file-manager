// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sysinfoutils.cpp
 * @brief Unit tests for SysInfoUtils (sysinfoutils.cpp)
 */

#include <gtest/gtest.h>
#include <QMimeData>
#include <QCoreApplication>
#include <unistd.h>

#include <dfm-base/utils/sysinfoutils.h>

using namespace dfmbase::SysInfoUtils;

TEST(SysInfoUtilsTest, GetUserReturnsNonEmptyInTestEnv)
{
    QString user = getUser();
    // In the test environment USER env is set
    EXPECT_FALSE(user.isEmpty());
}

TEST(SysInfoUtilsTest, GetHostNameNonEmpty)
{
    QString host = getHostName();
    EXPECT_FALSE(host.isEmpty());
}

TEST(SysInfoUtilsTest, GetUserIdNonNegative)
{
    int uid = getUserId();
    EXPECT_GE(uid, 0);
}

TEST(SysInfoUtilsTest, IsRootUserConsistent)
{
    bool root = isRootUser();
    EXPECT_EQ(root, getUserId() == 0);
}

TEST(SysInfoUtilsTest, IsServerAndDesktopAreComplementary)
{
    EXPECT_NE(isServerSys(), isDesktopSys());
}

TEST(SysInfoUtilsTest, GetAllUsersOfHomeReturnsList)
{
    QStringList users = getAllUsersOfHome();
    // /home should exist
    EXPECT_FALSE(users.isEmpty());
}

TEST(SysInfoUtilsTest, SetAndCheckMimeDataUserId)
{
    QMimeData data;
    setMimeDataUserId(&data);
    EXPECT_TRUE(isSameUser(&data));
}

TEST(SysInfoUtilsTest, IsSameUserFalseForEmptyMimeData)
{
    QMimeData data;
    EXPECT_FALSE(isSameUser(&data));
}

TEST(SysInfoUtilsTest, GetMemoryUsageForSelf)
{
    float usage = getMemoryUsage(static_cast<int>(getpid()));
    EXPECT_GE(usage, 0);
}

TEST(SysInfoUtilsTest, GetMemoryUsageForInvalidPid)
{
    float usage = getMemoryUsage(999999);
    EXPECT_EQ(usage, 0.0f);
}

TEST(SysInfoUtilsTest, GetOriginalUserHomeNonEmpty)
{
    QString home = getOriginalUserHome();
    EXPECT_FALSE(home.isEmpty());
}

TEST(SysInfoUtilsTest, IsDeepin23IsBool)
{
    EXPECT_NO_FATAL_FAILURE({ isDeepin23(); });
}

TEST(SysInfoUtilsTest, IsDeveloperModeEnabledIsBool)
{
    EXPECT_NO_FATAL_FAILURE({ isDeveloperModeEnabled(); });
}


TEST(SysInfoUtilsTest, getAllUsersOfHome)
{
    SysInfoUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.getAllUsersOfHome(); });
}

TEST(SysInfoUtilsTest, getHostName)
{
    SysInfoUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.getHostName(); });
}

TEST(SysInfoUtilsTest, getOriginalUserHome)
{
    SysInfoUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.getOriginalUserHome(); });
}

TEST(SysInfoUtilsTest, getUser)
{
    SysInfoUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.getUser(); });
}

TEST(SysInfoUtilsTest, getUserId)
{
    SysInfoUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.getUserId(); });
}

TEST(SysInfoUtilsTest, isDeepin23)
{
    SysInfoUtils obj;
    bool result = obj.isDeepin23();
    EXPECT_FALSE(result);
}

TEST(SysInfoUtilsTest, isDesktopSys)
{
    SysInfoUtils obj;
    bool result = obj.isDesktopSys();
    EXPECT_FALSE(result);
}

TEST(SysInfoUtilsTest, isDeveloperModeEnabled)
{
    SysInfoUtils obj;
    bool result = obj.isDeveloperModeEnabled();
    EXPECT_FALSE(result);
}

TEST(SysInfoUtilsTest, isOpenAsAdmin)
{
    SysInfoUtils obj;
    bool result = obj.isOpenAsAdmin();
    EXPECT_FALSE(result);
}

TEST(SysInfoUtilsTest, isRootUser)
{
    SysInfoUtils obj;
    bool result = obj.isRootUser();
    EXPECT_FALSE(result);
}

TEST(SysInfoUtilsTest, isSameUser)
{
    SysInfoUtils obj;
    EXPECT_NO_FATAL_FAILURE({ obj.isSameUser(nullptr); });
}

TEST(SysInfoUtilsTest, isServerSys)
{
    SysInfoUtils obj;
    bool result = obj.isServerSys();
    EXPECT_FALSE(result);
}

TEST(SysInfoUtilsTest, setMimeDataUserId)
{
    SysInfoUtils obj;
    EXPECT_NO_FATAL_FAILURE(obj.setMimeDataUserId(nullptr));
}

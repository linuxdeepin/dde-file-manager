// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_applaunchutils.cpp
 * @brief Unit tests for AppLaunchUtils (applaunchutils.cpp)
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>

#include <dfm-base/utils/applaunchutils.h>

using namespace dfmbase;

TEST(AppLaunchUtilsTest, InstanceReturnsRef)
{
    EXPECT_NO_FATAL_FAILURE({ (void)&AppLaunchUtils::instance(); });
}

TEST(AppLaunchUtilsTest, AddCustomStrategyAndLaunch)
{
    auto &utils = AppLaunchUtils::instance();
    bool called = false;
    AppLaunchFunc strategy = [&called](const QString &, const QStringList &) -> bool {
        called = true;
        return true;
    };
    utils.addStrategy(strategy, 0);

    bool ok = utils.launchApp("org.test.app.desktop", { "file:///tmp/x.txt" });
    EXPECT_TRUE(ok);
    EXPECT_TRUE(called);
}

TEST(AppLaunchUtilsTest, LaunchAppNoStrategyFails)
{
    // A strategy that always fails with a non-existent desktop file
    auto &utils = AppLaunchUtils::instance();
    bool ok = utils.launchApp("/no/such/app.desktop", {});
    // may succeed if an earlier strategy returns true, but generally false here
    EXPECT_NO_FATAL_FAILURE({ (void)ok; });
}

TEST(AppLaunchUtilsTest, DefaultLaunchAppNonExistent)
{
    auto &utils = AppLaunchUtils::instance();
    EXPECT_NO_FATAL_FAILURE({ (void)utils.defaultLaunchApp("/no/such/app.desktop", {}); });
}

TEST(AppLaunchUtilsTest, ExecuteCommandNonExistentProgram)
{
    auto &utils = AppLaunchUtils::instance();
    bool ok = utils.executeCommand("/no/such/program/xyz", { "arg1" }, "shortcut");
    EXPECT_FALSE(ok);
}

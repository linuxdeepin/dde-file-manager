// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "units/vaultupgradeunit.h"
#include "utils/upgradeutils.h"

#include <gtest/gtest.h>
#include <QObject>
#include <QString>

using namespace dfm_upgrade;

// Test for VaultUpgradeUnit
TEST(VaultUpgradeUnit, name)
{
    VaultUpgradeUnit unit;
    EXPECT_EQ(unit.name(), QString("VaultUpgradeUnit"));
}

// Test for UpgradeUtils
TEST(UpgradeUtils, genericAttribute)
{
    // 测试genericAttribute函数
    QVariant result = UpgradeUtils::genericAttribute("nonexistent_key");
    EXPECT_TRUE(result.isNull());
}

TEST(UpgradeUtils, applicationAttribute)
{
    // 测试applicationAttribute函数
    QVariant result = UpgradeUtils::applicationAttribute("nonexistent_key");
    EXPECT_TRUE(result.isNull());
}

TEST(UpgradeUtils, backupFile)
{
    // 测试backupFile函数
    // 使用不存在的文件路径，应该返回false
    bool result = UpgradeUtils::backupFile("/nonexistent/source/file", "/tmp");
    EXPECT_FALSE(result);
}
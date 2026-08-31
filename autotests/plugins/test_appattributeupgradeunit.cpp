// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_appattributeupgradeunit.cpp
 * @brief Unit tests for AppAttributeUpgradeUnit methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/units/appattributeupgradeunit.h"

#include <QTest>

using namespace src;

class AppAttributeUpgradeUnitTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AppAttributeUpgradeUnit();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AppAttributeUpgradeUnit *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AppAttributeUpgradeUnitTest, backupAppAttribute)
{
    // Test bool getter: backupAppAttribute()
    bool result = obj->backupAppAttribute();
    EXPECT_FALSE(result);

}

TEST_F(AppAttributeUpgradeUnitTest, initialize)
{
    // Test method: bool initialize((const QMap<QString, QString> &args))
    QMap<QString, QString> _arg0{};
    auto result = obj->initialize(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(AppAttributeUpgradeUnitTest, transIconSizeLevel)
{
    // Test method: int transIconSizeLevel((int oldIconSizeLevel))
    auto result = obj->transIconSizeLevel(0);
    EXPECT_GE(result, 0);

}

TEST_F(AppAttributeUpgradeUnitTest, writeConfigFile)
{
    // Test bool getter: writeConfigFile()
    bool result = obj->writeConfigFile();
    EXPECT_FALSE(result);

}

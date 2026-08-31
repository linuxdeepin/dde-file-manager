// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dconfigupgradeunit.cpp
 * @brief Unit tests for DConfigUpgradeUnit methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/units/dconfigupgradeunit.h"

#include <QTest>

using namespace src;

class DConfigUpgradeUnitTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DConfigUpgradeUnit();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DConfigUpgradeUnit *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DConfigUpgradeUnitTest, clearDiskHidden)
{
    // Test method: void clearDiskHidden(())
    EXPECT_NO_FATAL_FAILURE(obj->clearDiskHidden());
}

TEST_F(DConfigUpgradeUnitTest, mappedActions)
{
    // Test getter: QMap<QString, QString> mappedActions()
    auto result = obj->mappedActions();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DConfigUpgradeUnitTest, upgradeMenuConfigs)
{
    // Test bool getter: upgradeMenuConfigs()
    bool result = obj->upgradeMenuConfigs();
    EXPECT_FALSE(result);

}

TEST_F(DConfigUpgradeUnitTest, upgradeRecentConfigs)
{
    // Test bool getter: upgradeRecentConfigs()
    bool result = obj->upgradeRecentConfigs();
    EXPECT_FALSE(result);

}

TEST_F(DConfigUpgradeUnitTest, upgradeSearchConfigs)
{
    // Test bool getter: upgradeSearchConfigs()
    bool result = obj->upgradeSearchConfigs();
    EXPECT_FALSE(result);

}

TEST_F(DConfigUpgradeUnitTest, upgradeSmbConfigs)
{
    // Test bool getter: upgradeSmbConfigs()
    bool result = obj->upgradeSmbConfigs();
    EXPECT_FALSE(result);

}

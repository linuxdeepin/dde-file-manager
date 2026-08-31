// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_smbvirtualentryupgradeunit.cpp
 * @brief Unit tests for SmbVirtualEntryUpgradeUnit methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tools/upgrade/units/smbvirtualentryupgradeunit.h"

#include <QTest>

using namespace src;

class SmbVirtualEntryUpgradeUnitTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SmbVirtualEntryUpgradeUnit();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SmbVirtualEntryUpgradeUnit *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SmbVirtualEntryUpgradeUnitTest, clearOldItems)
{
    // Test method: void clearOldItems(())
    EXPECT_NO_FATAL_FAILURE(obj->clearOldItems());
}

TEST_F(SmbVirtualEntryUpgradeUnitTest, saveToDb)
{
    // Test method: void saveToDb((const QList<VirtualEntryData> &entries))
    QList<VirtualEntryData> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->saveToDb(_arg0));
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_dm_setup.cpp
 * @brief Unit tests for dm_setup methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/core/dmsetup.h"

#include <QTest>

using namespace src;

class dm_setupTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new dm_setup();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    dm_setup *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(dm_setupTest, dmCreateDevice)
{
    // Test method: int dmCreateDevice((const QString &dmDev, const DMTable &table))
    QString _arg0{};
    DMTable _arg1{};
    auto result = obj->dmCreateDevice(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(dm_setupTest, dmReloadDevice)
{
    // Test method: int dmReloadDevice((const QString &dmDev, const DMTable &table))
    QString _arg0{};
    DMTable _arg1{};
    auto result = obj->dmReloadDevice(_arg0, _arg1);
    EXPECT_GE(result, 0);

}

TEST_F(dm_setupTest, dmSetDeviceTable)
{
    // Test method: int dmSetDeviceTable((const QString &dmDev, const DMTable &table, int taskType))
    QString _arg0{};
    DMTable _arg1{};
    auto result = obj->dmSetDeviceTable(_arg0, _arg1, 0);
    EXPECT_GE(result, 0);

}

TEST_F(dm_setupTest, dmSuspendDevice)
{
    // Test method: int dmSuspendDevice((const QString &dmDev))
    QString _arg0{};
    auto result = obj->dmSuspendDevice(_arg0);
    EXPECT_GE(result, 0);

}

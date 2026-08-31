// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_discdevicescanner.cpp
 * @brief Unit tests for DiscDeviceScanner methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/device/private/discdevicescanner.h"

#include <QTest>

using namespace src;

class DiscDeviceScannerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DiscDeviceScanner();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DiscDeviceScanner *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DiscDeviceScannerTest, startScan)
{
    // Test bool getter: startScan()
    bool result = obj->startScan();
    EXPECT_FALSE(result);

}

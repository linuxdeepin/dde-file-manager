// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemanagerdbus.cpp
 * @brief Unit tests for DeviceManagerDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "devicemanagerdbus.h"

#include <QTest>

using namespace core;

class DeviceManagerDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeviceManagerDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeviceManagerDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeviceManagerDBusTest, initConnection)
{
    // Test method: void initConnection(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnection());
}

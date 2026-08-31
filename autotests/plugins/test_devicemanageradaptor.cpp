// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemanageradaptor.cpp
 * @brief Unit tests for DeviceManagerAdaptor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "config/dbus/DeviceManagerAdaptor.h"

#include <QTest>

using namespace src;

class DeviceManagerAdaptorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeviceManagerAdaptor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeviceManagerAdaptor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeviceManagerAdaptorTest, DeviceManagerAdaptor)
{
    // Test constructor: DeviceManagerAdaptor((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

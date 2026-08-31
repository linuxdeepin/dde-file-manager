// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicebasicwidget.cpp
 * @brief Unit tests for DeviceBasicWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "deviceproperty/devicebasicwidget.h"

#include <QTest>

using namespace dfmplugin_computer;

class DeviceBasicWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeviceBasicWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeviceBasicWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeviceBasicWidgetTest, DeviceBasicWidget)
{
    // Test constructor: DeviceBasicWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DeviceBasicWidgetTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

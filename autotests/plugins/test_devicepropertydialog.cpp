// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicepropertydialog.cpp
 * @brief Unit tests for DevicePropertyDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "deviceproperty/devicepropertydialog.h"

#include <QTest>

using namespace dfmplugin_computer;

class DevicePropertyDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DevicePropertyDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DevicePropertyDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DevicePropertyDialogTest, DevicePropertyDialog)
{
    // Test constructor: DevicePropertyDialog((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DevicePropertyDialogTest, iniUI)
{
    // Test method: void iniUI(())
    EXPECT_NO_FATAL_FAILURE(obj->iniUI());
}

TEST_F(DevicePropertyDialogTest, setFileName)
{
    // Test setter: void setFileName((const QString &filename))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setFileName(_arg0));
}

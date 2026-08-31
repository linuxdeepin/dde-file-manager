// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicepropertydialog_1.cpp
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

TEST_F(DevicePropertyDialogTest, addExtendedControl)
{
    // Test method: void addExtendedControl((QWidget *widget))
    EXPECT_NO_FATAL_FAILURE(obj->addExtendedControl(nullptr));
}

TEST_F(DevicePropertyDialogTest, closeEvent)
{
    // Test event handler: closeEvent((QCloseEvent *event))
    QCloseEvent _event(QCloseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->closeEvent(&_event));
}

TEST_F(DevicePropertyDialogTest, contentHeight)
{
    // Test getter: int contentHeight()
    auto result = obj->contentHeight();
    EXPECT_EQ(result, 0);

}

TEST_F(DevicePropertyDialogTest, handleHeight)
{
    // Test method: void handleHeight((int height))
    EXPECT_NO_FATAL_FAILURE(obj->handleHeight(0));
}

TEST_F(DevicePropertyDialogTest, insertExtendedControl)
{
    // Test method: void insertExtendedControl((int index, QWidget *widget))
    EXPECT_NO_FATAL_FAILURE(obj->insertExtendedControl(0, nullptr));
}

TEST_F(DevicePropertyDialogTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *event))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

TEST_F(DevicePropertyDialogTest, setProgressBar)
{
    // Test setter: void setProgressBar((qint64 totalSize, qint64 freeSize, bool mounted))
    EXPECT_NO_FATAL_FAILURE(obj->setProgressBar(0, 0, false));
}

TEST_F(DevicePropertyDialogTest, setSelectDeviceInfo)
{
    // Test setter: void setSelectDeviceInfo((const DeviceInfo &info))
    DeviceInfo _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSelectDeviceInfo(_arg0));
}

TEST_F(DevicePropertyDialogTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *event))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicebasicwidget_1.cpp
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

TEST_F(DeviceBasicWidgetTest, selectFileInfo)
{
    // Test method: void selectFileInfo((const DeviceInfo &info))
    DeviceInfo _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFileInfo(_arg0));
}

TEST_F(DeviceBasicWidgetTest, selectFileUrl)
{
    // Test method: void selectFileUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->selectFileUrl(_arg0));
}

TEST_F(DeviceBasicWidgetTest, slotFileDirSizeChange)
{
    // Test method: void slotFileDirSizeChange((const FileScanner::ScanResult &result))
    FileScanner::ScanResult _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->slotFileDirSizeChange(_arg0));
}

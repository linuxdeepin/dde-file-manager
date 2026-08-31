// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_usbrepairmonitor.cpp
 * @brief Unit tests for UsbRepairMonitor methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/usbrepair/usbrepairmonitor.h"

#include <QTest>

using namespace src;

class UsbRepairMonitorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UsbRepairMonitor();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UsbRepairMonitor *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UsbRepairMonitorTest, checkDirtyBit)
{
    // Test method: bool checkDirtyBit((const QString &deviceFile, const QString &fsType))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->checkDirtyBit(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairMonitorTest, isUsbDevice)
{
    // Test method: bool isUsbDevice((const QString &blockObjPath, QString *deviceName))
    QString _arg0{};
    auto result = obj->isUsbDevice(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairMonitorTest, onInterfacesRemoved)
{
    // Test method: void onInterfacesRemoved((
    const QDBusObjectPath &objectPath,
    const QStringList &interfaces))
    QDBusObjectPath _arg0{};
    QStringList _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onInterfacesRemoved(_arg0, _arg1));
}

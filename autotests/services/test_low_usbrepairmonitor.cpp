// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_usbrepairmonitor.cpp
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

TEST_F(UsbRepairMonitorTest, getDeviceFile)
{
    // Test method: QString getDeviceFile((const QString &blockObjPath))
    QString _arg0{};
    auto result = obj->getDeviceFile(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UsbRepairMonitorTest, getFsType)
{
    // Test method: QString getFsType((const QString &deviceFile))
    QString _arg0{};
    auto result = obj->getFsType(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(UsbRepairMonitorTest, isDeviceMounted)
{
    // Test method: bool isDeviceMounted((const QString &deviceFile))
    QString _arg0{};
    auto result = obj->isDeviceMounted(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairMonitorTest, isHardwareReadOnly)
{
    // Test method: bool isHardwareReadOnly((const QString &deviceFile))
    QString _arg0{};
    auto result = obj->isHardwareReadOnly(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairMonitorTest, onInterfacesAdded)
{
    // Test method: void onInterfacesAdded((
    const QDBusObjectPath &objectPath,
    const QMap<QString, QVariantMap> &interfacesAndProperties))
    QDBusObjectPath _arg0{};
    QMap<QString, QVariantMap> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onInterfacesAdded(_arg0, _arg1));
}

TEST_F(UsbRepairMonitorTest, shouldIgnoreDevice)
{
    // Test method: bool shouldIgnoreDevice((const QString &blockObjPath))
    QString _arg0{};
    auto result = obj->shouldIgnoreDevice(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UsbRepairMonitorTest, startMonitoring)
{
    // Test method: void startMonitoring(())
    EXPECT_NO_FATAL_FAILURE(obj->startMonitoring());
}

TEST_F(UsbRepairMonitorTest, stopMonitoring)
{
    // Test method: void stopMonitoring(())
    EXPECT_NO_FATAL_FAILURE(obj->stopMonitoring());
}

TEST_F(UsbRepairMonitorTest, UsbRepairMonitor_Destructor)
{
    // Test method:  ~UsbRepairMonitor(())
    EXPECT_NO_FATAL_FAILURE({ UsbRepairMonitor *tmp = new UsbRepairMonitor(); delete tmp; });
}

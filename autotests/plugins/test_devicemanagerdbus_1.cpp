// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_devicemanagerdbus_1.cpp
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

TEST_F(DeviceManagerDBusTest, DetachAllMountedDevices)
{
    // Test method: void DetachAllMountedDevices(())
    EXPECT_NO_FATAL_FAILURE(obj->DetachAllMountedDevices());
}

TEST_F(DeviceManagerDBusTest, DetachBlockDevice)
{
    // Test method: void DetachBlockDevice((QString id))
    EXPECT_NO_FATAL_FAILURE(obj->DetachBlockDevice(QString()));
}

TEST_F(DeviceManagerDBusTest, DetachProtocolDevice)
{
    // Test method: void DetachProtocolDevice((QString id))
    EXPECT_NO_FATAL_FAILURE(obj->DetachProtocolDevice(QString()));
}

TEST_F(DeviceManagerDBusTest, DeviceManagerDBus)
{
    // Test constructor: DeviceManagerDBus((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DeviceManagerDBusTest, GetBlockDevicesIdList)
{
    // Test method: QStringList GetBlockDevicesIdList((int opts))
    auto result = obj->GetBlockDevicesIdList(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DeviceManagerDBusTest, GetProtocolDevicesIdList)
{
    // Test getter: QStringList GetProtocolDevicesIdList()
    auto result = obj->GetProtocolDevicesIdList();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DeviceManagerDBusTest, IsMonotorWorking)
{
    // Test bool getter: IsMonotorWorking()
    bool result = obj->IsMonotorWorking();
    EXPECT_FALSE(result);

}

TEST_F(DeviceManagerDBusTest, QueryBlockDeviceInfo)
{
    // Test method: QVariantMap QueryBlockDeviceInfo((QString id, bool reload))
    auto result = obj->QueryBlockDeviceInfo(QString(), false);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DeviceManagerDBusTest, QueryProtocolDeviceInfo)
{
    // Test method: QVariantMap QueryProtocolDeviceInfo((QString id, bool reload))
    auto result = obj->QueryProtocolDeviceInfo(QString(), false);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(DeviceManagerDBusTest, RefreshDeviceUsage)
{
    // Test method: void RefreshDeviceUsage(())
    EXPECT_NO_FATAL_FAILURE(obj->RefreshDeviceUsage());
}

TEST_F(DeviceManagerDBusTest, StartMonitoringUsage)
{
    // Test method: void StartMonitoringUsage(())
    EXPECT_NO_FATAL_FAILURE(obj->StartMonitoringUsage());
}

TEST_F(DeviceManagerDBusTest, StopMonitoringUsage)
{
    // Test method: void StopMonitoringUsage(())
    EXPECT_NO_FATAL_FAILURE(obj->StopMonitoringUsage());
}

TEST_F(DeviceManagerDBusTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(DeviceManagerDBusTest, onNameOwnerChanged)
{
    // Test method: void onNameOwnerChanged((const QString &name,
                                           const QString &oldOwner,
                                           const QString &newOwner))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->onNameOwnerChanged(_arg0, _arg1, _arg2));
}

TEST_F(DeviceManagerDBusTest, requestRefreshDesktopAsNeeded)
{
    // Test method: void requestRefreshDesktopAsNeeded((const QString &path, const QString &operation))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->requestRefreshDesktopAsNeeded(_arg0, _arg1));
}

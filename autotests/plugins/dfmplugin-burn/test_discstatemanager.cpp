// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/common/dfmplugin-burn/utils/discstatemanager.h"

#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/dbusservice/global_server_defines.h>

#include <QSignalSpy>
#include <QVariant>

#include <gtest/gtest.h>

DPBURN_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace GlobalServerDefines;

class UT_DiscStateManager : public testing::Test
{
protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_DiscStateManager, instance)
{
    DiscStateManager *manager1 = DiscStateManager::instance();
    DiscStateManager *manager2 = DiscStateManager::instance();

    EXPECT_TRUE(manager1 != nullptr);
    EXPECT_EQ(manager1, manager2);   // Should be singleton
}

TEST_F(UT_DiscStateManager, initilaize)
{
    DiscStateManager::instance()->initilaize();
}

TEST_F(UT_DiscStateManager, ghostMountForBlankDisc)
{
    bool getAllBlockIdsCalled = false;

    stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds), [&getAllBlockIdsCalled] {
        __DBG_STUB_INVOKE__
        getAllBlockIdsCalled = true;
        QStringList ids;
        ids << "/org/freedesktop/UDisks2/block_devices/sr0";
        return ids;
    });

    bool queryBlockInfoCalled = false;
    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [&queryBlockInfoCalled] {
        __DBG_STUB_INVOKE__
        queryBlockInfoCalled = true;
        QVariantMap info;
        info[DeviceProperty::kOptical] = true;
        info[DeviceProperty::kOpticalBlank] = true;
        info[DeviceProperty::kMountPoint] = QString();   // Not mounted
        return info;
    });

    bool mountCalled = false;
    stub.set_lamda(ADDR(DeviceManager, mountBlockDevAsync), [&mountCalled] {
        __DBG_STUB_INVOKE__
        mountCalled = true;
    });

    DiscStateManager::instance()->ghostMountForBlankDisc();

    EXPECT_TRUE(getAllBlockIdsCalled);
    EXPECT_TRUE(queryBlockInfoCalled);
    EXPECT_TRUE(mountCalled);
}

TEST_F(UT_DiscStateManager, ghostMountForBlankDisc_NoOpticalDevices)
{
    stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds), [] {
        __DBG_STUB_INVOKE__
        QStringList ids;
        ids << "/org/freedesktop/UDisks2/block_devices/sda1";   // Not optical
        return ids;
    });

    bool queryBlockInfoCalled = false;
    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [&queryBlockInfoCalled] {
        __DBG_STUB_INVOKE__
        queryBlockInfoCalled = true;
        QVariantMap info;
        info[DeviceProperty::kOptical] = false;   // Not optical
        return info;
    });

    bool mountCalled = false;
    stub.set_lamda(ADDR(DeviceManager, mountBlockDevAsync), [&mountCalled] {
        __DBG_STUB_INVOKE__
        mountCalled = true;
    });

    DiscStateManager::instance()->ghostMountForBlankDisc();

    EXPECT_TRUE(queryBlockInfoCalled);
    EXPECT_FALSE(mountCalled);   // Should not mount non-optical devices
}

TEST_F(UT_DiscStateManager, ghostMountForBlankDisc_NotBlank)
{
    stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds), [] {
        __DBG_STUB_INVOKE__
        QStringList ids;
        ids << "/org/freedesktop/UDisks2/block_devices/sr0";
        return ids;
    });

    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kOptical] = true;
        info[DeviceProperty::kOpticalBlank] = false;   // Not blank
        return info;
    });

    bool mountCalled = false;
    stub.set_lamda(ADDR(DeviceManager, mountBlockDevAsync), [&mountCalled] {
        __DBG_STUB_INVOKE__
        mountCalled = true;
    });

    DiscStateManager::instance()->ghostMountForBlankDisc();

    EXPECT_FALSE(mountCalled);   // Should not mount non-blank discs
}

TEST_F(UT_DiscStateManager, ghostMountForBlankDisc_AlreadyMounted)
{
    stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds), [] {
        __DBG_STUB_INVOKE__
        QStringList ids;
        ids << "/org/freedesktop/UDisks2/block_devices/sr0";
        return ids;
    });

    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [] {
        __DBG_STUB_INVOKE__
        QVariantMap info;
        info[DeviceProperty::kOptical] = true;
        info[DeviceProperty::kOpticalBlank] = true;
        info[DeviceProperty::kMountPoint] = "/media/sr0";   // Already mounted
        return info;
    });

    bool mountCalled = false;
    stub.set_lamda(ADDR(DeviceManager, mountBlockDevAsync), [&mountCalled] {
        __DBG_STUB_INVOKE__
        mountCalled = true;
    });

    DiscStateManager::instance()->ghostMountForBlankDisc();

    EXPECT_FALSE(mountCalled);   // Should not mount already mounted discs
}

TEST_F(UT_DiscStateManager, onDevicePropertyChanged_OpticalBlank)
{
    bool ghostMountCalled = false;

    stub.set_lamda(ADDR(DiscStateManager, ghostMountForBlankDisc), [&ghostMountCalled] {
        __DBG_STUB_INVOKE__
        ghostMountCalled = true;
    });

    DiscStateManager::instance()->onDevicePropertyChanged(
            "/org/freedesktop/UDisks2/block_devices/sr0",
            "OpticalBlank",
            QVariant(true));

    EXPECT_TRUE(ghostMountCalled);
}

TEST_F(UT_DiscStateManager, onDevicePropertyChanged_OpticalBlank_False)
{
    bool ghostMountCalled = false;

    stub.set_lamda(ADDR(DiscStateManager, ghostMountForBlankDisc), [&ghostMountCalled] {
        __DBG_STUB_INVOKE__
        ghostMountCalled = true;
    });

    DiscStateManager::instance()->onDevicePropertyChanged(
            "/org/freedesktop/UDisks2/block_devices/sr0",
            "OpticalBlank",
            QVariant(false));

    EXPECT_FALSE(ghostMountCalled);   // Should not call for non-blank
}

TEST_F(UT_DiscStateManager, onDevicePropertyChanged_OtherProperty)
{
    bool ghostMountCalled = false;

    stub.set_lamda(ADDR(DiscStateManager, ghostMountForBlankDisc), [&ghostMountCalled] {
        __DBG_STUB_INVOKE__
        ghostMountCalled = true;
    });

    DiscStateManager::instance()->onDevicePropertyChanged(
            "/org/freedesktop/UDisks2/block_devices/sr0",
            "SomeOtherProperty",
            QVariant("value"));

    EXPECT_FALSE(ghostMountCalled);   // Should not call for other properties
}

TEST_F(UT_DiscStateManager, onDevicePropertyChanged_NonOpticalDevice)
{
    bool ghostMountCalled = false;

    stub.set_lamda(ADDR(DiscStateManager, ghostMountForBlankDisc), [&ghostMountCalled] {
        __DBG_STUB_INVOKE__
        ghostMountCalled = true;
    });

    DiscStateManager::instance()->onDevicePropertyChanged(
            "/org/freedesktop/UDisks2/block_devices/sda1",   // Not optical
            "OpticalBlank",
            QVariant(true));

    EXPECT_FALSE(ghostMountCalled);   // Should not call for non-optical devices
}

TEST_F(UT_DiscStateManager, ghostMountForBlankDisc_EmptyDeviceList)
{
    stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds), [] {
        __DBG_STUB_INVOKE__
        return QStringList();   // Empty list
    });

    bool mountCalled = false;
    stub.set_lamda(ADDR(DeviceManager, mountBlockDevAsync), [&mountCalled] {
        __DBG_STUB_INVOKE__
        mountCalled = true;
    });

    DiscStateManager::instance()->ghostMountForBlankDisc();

    EXPECT_FALSE(mountCalled);   // Should not mount anything
}

TEST_F(UT_DiscStateManager, ghostMountForBlankDisc_MultipleDevices)
{
    stub.set_lamda(ADDR(DeviceProxyManager, getAllBlockIds), [] {
        __DBG_STUB_INVOKE__
        QStringList ids;
        ids << "/org/freedesktop/UDisks2/block_devices/sr0"
            << "/org/freedesktop/UDisks2/block_devices/sr1"
            << "/org/freedesktop/UDisks2/block_devices/sda1";   // Mix of optical and non-optical
        return ids;
    });

    int queryCount = 0;
    stub.set_lamda(ADDR(DeviceProxyManager, queryBlockInfo), [&queryCount](DeviceProxyManager *, const QString &id, bool reload) {
        __DBG_STUB_INVOKE__
        queryCount++;
        QVariantMap info;
        if (id.contains("sr")) {
            info[DeviceProperty::kOptical] = true;
            info[DeviceProperty::kOpticalBlank] = true;
            info[DeviceProperty::kMountPoint] = QString();
        } else {
            info[DeviceProperty::kOptical] = false;
        }
        return info;
    });

    int mountCount = 0;
    stub.set_lamda(ADDR(DeviceManager, mountBlockDevAsync), [&mountCount] {
        __DBG_STUB_INVOKE__
        mountCount++;
    });

    DiscStateManager::instance()->ghostMountForBlankDisc();

    EXPECT_EQ(queryCount, 3);   // Should query all devices
    EXPECT_EQ(mountCount, 2);   // Should only mount the two optical devices
}

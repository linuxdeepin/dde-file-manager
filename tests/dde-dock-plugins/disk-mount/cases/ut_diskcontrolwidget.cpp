/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     max-lv<lvwujun@uniontech.com>
 *
 * Maintainer: dengkeyun<dengkeyun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ut_mock_stub_disk_gio.h"
#include "diskcontrolwidget.h"

#include <QWidget>
#include <gtest/gtest.h>

#include <ddiskmanager.h>
#include <dblockdevice.h>
#include <ddiskdevice.h>
#include <dfmsettings.h>
#include <dgiosettings.h>
#include <DDesktopServices>
#include <dgiovolumemanager.h>
#include <DDBusSender>

#include "dattachedvfsdevice.h"
#include <dfmsettings.h>

#include "stub.h"
#include "stubext.h"
#include "ut_mock_stub_diskdevice.h"
#include <QProcess>

DFM_USE_NAMESPACE
namespace  {
    class TestDiskControlWidget : public testing::Test {
    public:

        void SetUp() override
        {
            mCtrlWidget.reset( new DiskControlWidget());
            mDiskManager = mCtrlWidget->startMonitor();
        }
        void TearDown() override
        {
        }

    public:
        DDiskManager* mDiskManager;
        std::shared_ptr<DiskControlWidget> mCtrlWidget;
    };
}

QVariant value_true_stub(const QString &group, const QString &key, const QVariant &defaultValue)
{
    return true;
}

QVariant value_false_stub(const QString &group, const QString &key, const QVariant &defaultValue)
{
    return false;
}

TEST_F(TestDiskControlWidget, can_send_notifymsg)
{
    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    mCtrlWidget->NotifyMsg("msg details");
    mCtrlWidget->NotifyMsg("title", "msg informations");
}

TEST_F(TestDiskControlWidget, can_do_StartupAutoMount)
{
    bool (DBlockDevice::*fileSystem)() const = &DBlockDevice::hasFileSystem;
    QStringList (*blockDevices)(QVariantMap) = &DDiskManager::blockDevices;

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    Stub stub;
    stub.set(blockDevices, blockDevices_DDiskManager_stub);

    stub.set(ADDR(DDiskDevice, ejectable), ejectable_stub);
    stub.set(fileSystem, hasFileSystem_stub);
    stub.set(ADDR(DBlockDevice,mountPoints), gio_mountPoints_more_stub);
    stub.set(ADDR(DBlockDevice,hintIgnore), hintIgnore_stub);
    stub.set(ADDR(DBlockDevice,hintSystem), hintSystem_stub);
    stub.set(ADDR(DDiskDevice,removable), removable_stub);
    stub.set(ADDR(DDiskDevice,canPowerOff), canPowerOff_stub);

    mCtrlWidget->doStartupAutoMount();
}

TEST_F(TestDiskControlWidget, can_monitor_DriveConnected)
{
    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    Stub stub;
    stub.set(ADDR(DDiskDevice,removable), removable_stub);

    emit mDiskManager->diskDeviceAdded(dgio_devpath_stub());
}

TEST_F(TestDiskControlWidget, can_monitor_DriveDisconnected)
{
    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    Stub stub;
    stub.set(ADDR(DDiskDevice,removable), removable_stub);

    emit mDiskManager->diskDeviceRemoved(dgio_devpath_stub());
}

TEST_F(TestDiskControlWidget, can_monitor_onMountRemoved)
{
    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    Stub stub;
    stub.set(ADDR(DDiskDevice,removable), removable_stub);

    emit mDiskManager->mountRemoved(dgio_devpath_stub(),dgio_devpath_stub().toUtf8());
}

TEST_F(TestDiskControlWidget, can_monitor_onVolumeAdded)
{
    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    Stub stub;
    stub.set(ADDR(DDiskDevice,removable), removable_stub);

    emit mDiskManager->fileSystemAdded(dgio_devpath_stub());
}

TEST_F(TestDiskControlWidget, can_monitor_onVolumeRemoved)
{
    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    Stub stub;
    stub.set(ADDR(DDiskDevice,removable), removable_stub);

    emit mDiskManager->fileSystemRemoved(dgio_devpath_stub());
}

TEST_F(TestDiskControlWidget, can_monitor_blockDeviceAdded_with_normalset)
{
    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    Stub stub;
    stub.set(ADDR(DDiskDevice,removable), removable_stub);

    emit mDiskManager->blockDeviceAdded(dgio_devpath_stub());
}

TEST_F(TestDiskControlWidget, can_monitor_blockDeviceAdded_without_automount_and_open)
{
    QVariant (DFMSettings::*func_value)(const QString &group, const QString &key, const QVariant &defaultValue) const = & DFMSettings::value;

    Stub stub;
    stub.set(ADDR(DDiskDevice,removable), removable_stub);
    stub.set(func_value, value_false_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    emit mDiskManager->blockDeviceAdded(dgio_devpath_stub());
}

TEST_F(TestDiskControlWidget, can_monitor_blockDeviceAdded_with_automount_and_open)
{
    QVariant (DFMSettings::*func_value)(const QString &group, const QString &key, const QVariant &defaultValue) const = & DFMSettings::value;
    bool (DBlockDevice::*fileSystem)() const = &DBlockDevice::hasFileSystem;

    Stub stub;

    stub.set(fileSystem, hasFileSystem_stub);
    stub.set(ADDR(DBlockDevice,mount), getDummyMountPoint);
    stub.set(ADDR(DDiskDevice,removable), removable_stub);
    stub.set(func_value, value_true_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    emit mDiskManager->blockDeviceAdded(dgio_devpath_stub());
}

TEST_F(TestDiskControlWidget, can_monitor_blockDeviceAdded_automount_and_open_but_mountpoint_is_empty)
{
    QVariant (DFMSettings::*func_value)(const QString &group, const QString &key, const QVariant &defaultValue) const = & DFMSettings::value;
    bool (DBlockDevice::*fileSystem)() const = &DBlockDevice::hasFileSystem;

    Stub stub;

    stub.set(fileSystem, hasFileSystem_stub);
    stub.set(ADDR(DBlockDevice,mount), get_empty_string_stub);
    stub.set(ADDR(DDiskDevice,removable), removable_stub);

    stub.set(func_value, value_true_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    emit mDiskManager->blockDeviceAdded(dgio_devpath_stub());
}

TEST_F(TestDiskControlWidget, can_monitor_diskDeviceRemoved)
{
    bool (DBlockDevice::*fileSystem)() const = &DBlockDevice::hasFileSystem;
    QStringList (*blockDevices)(QVariantMap) = &DDiskManager::blockDevices;

    Stub stub;
    stub.set(blockDevices, blockDevices_DDiskManager_stub);

    stub.set(ADDR(DDiskDevice, ejectable), ejectable_stub);
    stub.set(fileSystem, hasFileSystem_stub);
    stub.set(ADDR(DBlockDevice,mountPoints), gio_mountPoints_more_stub);
    stub.set(ADDR(DBlockDevice,hintIgnore), hintIgnore_stub);
    stub.set(ADDR(DBlockDevice,hintSystem), hintSystem_stub);
    stub.set(ADDR(DDiskDevice,removable), removable_stub);
    stub.set(ADDR(DDiskDevice,canPowerOff), canPowerOff_stub);

    stub.set(ADDR(DGioVolumeManager,getMounts), get_gvfs_Mounts_stub);
    stub.set(ADDR(DGioMount, getRootFile), get_gvfs_RootFile_stub);
    stub.set(ADDR(QUrl, scheme), scheme_burn_stub);
    stub.set(ADDR(DGioMount, isShadowed), isShadowed_stub);
    stub.set(ADDR(DGioMount, name), name_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    emit mDiskManager->diskDeviceRemoved(dgio_devpath_stub());
}


TEST_F(TestDiskControlWidget, can_unmountall_u_blk_device)
{
    bool (DBlockDevice::*fileSystem)() const = &DBlockDevice::hasFileSystem;
    QStringList (*blockDevices)(QVariantMap) = &DDiskManager::blockDevices;

    Stub stub;
    stub.set(blockDevices, blockDevices_DDiskManager_stub);

    stub.set(ADDR(DDiskDevice, ejectable), ejectable_stub);
    stub.set(fileSystem, hasFileSystem_stub);
    stub.set(ADDR(DBlockDevice,mountPoints), gio_mountPoints_more_stub);
    stub.set(ADDR(DBlockDevice,hintIgnore), hintIgnore_stub);
    stub.set(ADDR(DBlockDevice,hintSystem), hintSystem_stub);
    stub.set(ADDR(DDiskDevice,removable), removable_stub);
    stub.set(ADDR(DDiskDevice,canPowerOff), canPowerOff_stub);

    stub.set(ADDR(DGioVolumeManager,getMounts), get_gvfs_Mounts_stub);
    stub.set(ADDR(DGioMount, getRootFile), get_gvfs_RootFile_stub);
    stub.set(ADDR(QUrl, scheme), scheme_burn_stub);
    stub.set(ADDR(DGioMount, isShadowed), isShadowed_stub);
    stub.set(ADDR(DGioMount, name), name_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    mCtrlWidget->unmountAll();
    sleep(nTimeout_udisk);
}

TEST_F(TestDiskControlWidget, cant_unmountall_u_blk_device)
{
    bool (DBlockDevice::*fileSystem)() const = &DBlockDevice::hasFileSystem;
    QStringList (*blockDevices)(QVariantMap) = &DDiskManager::blockDevices;

    Stub stub;
    stub.set(blockDevices, blockDevices_DDiskManager_stub);

    stub.set(ADDR(DDiskDevice, ejectable), ejectable_stub);
    stub.set(fileSystem, hasFileSystem_stub);
    stub.set(ADDR(DBlockDevice,mountPoints), gio_mountPoints_more_stub);
    stub.set(ADDR(DBlockDevice,hintIgnore), hintIgnore_stub);
    stub.set(ADDR(DBlockDevice,hintSystem), hintSystem_stub);
    stub.set(ADDR(DDiskDevice,removable), removable_stub);
    stub.set(ADDR(DDiskDevice,lastError), lastError_stub);

    stub.set(ADDR(DGioVolumeManager,getMounts), get_gvfs_Mounts_stub);
    stub.set(ADDR(DGioMount, getRootFile), get_gvfs_RootFile_stub);
    stub.set(ADDR(QUrl, scheme), scheme_burn_stub);
    stub.set(ADDR(DGioMount, isShadowed), isShadowed_stub);
    stub.set(ADDR(DGioMount, name), name_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    mCtrlWidget->unmountAll();
    sleep(nTimeout_udisk);
}

TEST_F(TestDiskControlWidget, can_unmountall_optical_blk_device)
{
    bool (DBlockDevice::*fileSystem)() const = &DBlockDevice::hasFileSystem;
    QStringList (*blockDevices)(QVariantMap) = &DDiskManager::blockDevices;

    Stub stub;
    stub.set(blockDevices, blockDevices_DDiskManager_stub);

    stub.set(ADDR(DDiskDevice, ejectable), ejectable_stub);
    stub.set(fileSystem, hasFileSystem_stub);
    stub.set(ADDR(DBlockDevice,mountPoints), gio_mountPoints_more_stub);
    stub.set(ADDR(DBlockDevice,hintIgnore), hintIgnore_stub);
    stub.set(ADDR(DBlockDevice,hintSystem), hintSystem_stub);
    stub.set(ADDR(DDiskDevice,optical), optical_stub);

    stub.set(ADDR(DGioVolumeManager,getMounts), get_gvfs_Mounts_stub);
    stub.set(ADDR(DGioMount, getRootFile), get_gvfs_RootFile_stub);
    stub.set(ADDR(QUrl, scheme), scheme_burn_stub);
    stub.set(ADDR(DGioMount, isShadowed), isShadowed_stub);
    stub.set(ADDR(DGioMount, name), name_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    mCtrlWidget->unmountAll();
    sleep(nTimeout_udisk);
}

TEST_F(TestDiskControlWidget, cant_unmountall_optical_blk_device)
{
    bool (DBlockDevice::*fileSystem)() const = &DBlockDevice::hasFileSystem;
    QStringList (*blockDevices)(QVariantMap) = &DDiskManager::blockDevices;

    Stub stub;
    stub.set(blockDevices, blockDevices_DDiskManager_stub);

    stub.set(ADDR(DDiskDevice, ejectable), ejectable_stub);
    stub.set(fileSystem, hasFileSystem_stub);
    stub.set(ADDR(DBlockDevice,mountPoints), gio_mountPoints_more_stub);
    stub.set(ADDR(DBlockDevice,hintIgnore), hintIgnore_stub);
    stub.set(ADDR(DBlockDevice,hintSystem), hintSystem_stub);
    stub.set(ADDR(DDiskDevice,lastError), lastError_stub);
    stub.set(ADDR(DDiskDevice,optical), optical_stub);

    stub.set(ADDR(DGioVolumeManager,getMounts), get_gvfs_Mounts_stub);
    stub.set(ADDR(DGioMount, getRootFile), get_gvfs_RootFile_stub);
    stub.set(ADDR(QUrl, scheme), scheme_burn_stub);
    stub.set(ADDR(DGioMount, isShadowed), isShadowed_stub);
    stub.set(ADDR(DGioMount, name), name_stub);

    stub_ext::StubExt stue;
    stue.set_lamda(&DDBusCaller::call, [](){QDBusMessage msg; return QDBusPendingCall::fromCompletedCall(msg);});

    mCtrlWidget->unmountAll();
    sleep(nTimeout_udisk);
}

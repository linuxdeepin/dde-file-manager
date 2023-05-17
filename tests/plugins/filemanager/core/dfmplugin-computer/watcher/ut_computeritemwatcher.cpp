// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "plugins/filemanager/core/dfmplugin-computer/watcher/computeritemwatcher.h"
#include "plugins/filemanager/core/dfmplugin-computer/utils/computerutils.h"
#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/base/configs/configsynchronizer.h>

#include <dfm-framework/event/event.h>

#include <gtest/gtest.h>

DPCOMPUTER_USE_NAMESPACE

class UT_ComputerItemWatcher : public testing::Test
{
protected:
    virtual void SetUp() override { }
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
    ComputerItemWatcher *ins { ComputerItemWatcher::instance() };
};

TEST_F(UT_ComputerItemWatcher, Items)
{
    stub.set_lamda(&ComputerItemWatcher::getUserDirItems, [] { __DBG_STUB_INVOKE__ return ComputerDataList {}; });
    stub.set_lamda(&ComputerItemWatcher::getBlockDeviceItems, [] { __DBG_STUB_INVOKE__ return ComputerDataList {}; });
    stub.set_lamda(&ComputerItemWatcher::getProtocolDeviceItems, [] { __DBG_STUB_INVOKE__ return ComputerDataList {}; });
    stub.set_lamda(&ComputerItemWatcher::getAppEntryItems, [] { __DBG_STUB_INVOKE__ return ComputerDataList {}; });
    EXPECT_NO_FATAL_FAILURE(ins->items());
    EXPECT_TRUE(ins->items().count() == 0);
}

TEST_F(UT_ComputerItemWatcher, GetInitedItems)
{
    EXPECT_NO_FATAL_FAILURE(ins->getInitedItems());
}

TEST_F(UT_ComputerItemWatcher, TypeCompare)
{
    typedef bool (*Sort)(DFMEntryFileInfoPointer, DFMEntryFileInfoPointer);
    stub.set_lamda(static_cast<Sort>(ComputerUtils::sortItem), [] { __DBG_STUB_INVOKE__ return true; });
    EXPECT_TRUE(ins->typeCompare({}, {}));
    EXPECT_NO_FATAL_FAILURE(ins->typeCompare({}, {}));
}

TEST_F(UT_ComputerItemWatcher, AddDevice)
{
    stub.set_lamda(&ComputerItemWatcher::addDevice, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->addDevice("device", QUrl("entry:///sda1.blockdev"), ComputerItemData::kLargeItem));
}

TEST_F(UT_ComputerItemWatcher, RemoveDevice)
{
    stub.set_lamda(&ComputerItemWatcher::removeSidebarItem, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->removeDevice(QUrl()));
}

TEST_F(UT_ComputerItemWatcher, UpdateSidebarItem)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QUrl, QVariantMap &&);
    stub.set_lamda(static_cast<Push>(&dpf::EventChannelManager::push), [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins->updateSidebarItem(QUrl(""), "newName", false));
}

TEST_F(UT_ComputerItemWatcher, AddSidebarItem)
{
    stub.set_lamda(ComputerUtils::shouldSystemPartitionHide, [] { __DBG_STUB_INVOKE__ return false; });
    stub.set_lamda(VADDR(EntryFileInfo, extraProperties), [] {
        __DBG_STUB_INVOKE__
        return QVariantHash { { GlobalServerDefines::DeviceProperty::kRemovable, true } };
    });
    stub.set_lamda(VADDR(EntryFileInfo, renamable), [] { __DBG_STUB_INVOKE__ return true; });
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QUrl, QVariantMap &&);
    stub.set_lamda(static_cast<Push>(&dpf::EventChannelManager::push), [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins->addSidebarItem(nullptr));
    DFMEntryFileInfoPointer info(new EntryFileInfo(QUrl("entry:///sdb1.blockdev")));
    EXPECT_NO_FATAL_FAILURE(ins->addSidebarItem(info));
}

TEST_F(UT_ComputerItemWatcher, RemoveSidebarItem)
{
    typedef QVariant (dpf::EventChannelManager::*Push)(const QString &, const QString &, QUrl);
    stub.set_lamda(static_cast<Push>(&dpf::EventChannelManager::push), [] { __DBG_STUB_INVOKE__ return QVariant(); });
    EXPECT_NO_FATAL_FAILURE(ins->removeSidebarItem(QUrl()));
}

TEST_F(UT_ComputerItemWatcher, InsertUrlMapper)
{
    EXPECT_NO_FATAL_FAILURE(ins->insertUrlMapper("smb://1.2.3.4/hello", QUrl::fromLocalFile("/")));
    EXPECT_NO_FATAL_FAILURE(ins->insertUrlMapper("/org/freedesktop/UDisks2/block_devices/sdb1", QUrl::fromLocalFile("/")));
    EXPECT_NO_FATAL_FAILURE(ins->insertUrlMapper("/org/freedesktop/UDisks2/block_devices/sr0", QUrl::fromLocalFile("/home")));
    ins->routeMapper.clear();
}

TEST_F(UT_ComputerItemWatcher, StartQueryItems)
{
    stub.set_lamda(&ComputerItemWatcher::items, [] { __DBG_STUB_INVOKE__ return ComputerDataList {}; });
    EXPECT_NO_FATAL_FAILURE(ins->startQueryItems());
}

TEST_F(UT_ComputerItemWatcher, OnDeviceAdded)
{
    stub.set_lamda(VADDR(EntryFileInfo, exists), [] { __DBG_STUB_INVOKE__ return true; });
    stub.set_lamda(&ComputerItemWatcher::cacheItem, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&ComputerItemWatcher::removeDevice, [] { __DBG_STUB_INVOKE__ });
    auto &&url = ComputerUtils::makeProtocolDevUrl("smb://1.2.3.4/hello");
    EXPECT_NO_FATAL_FAILURE(ins->onDeviceAdded(url, 0));
}

TEST_F(UT_ComputerItemWatcher, OnDevicePropertyChagnedQVar)
{
    stub.set_lamda(&ComputerItemWatcher::onDevicePropertyChangedQDBusVar, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->onDevicePropertyChangedQVar("", "", ""));
}

TEST_F(UT_ComputerItemWatcher, OnDevicePropertyChangedQDBusVar)
{
    EXPECT_NO_FATAL_FAILURE(ins->onDevicePropertyChangedQDBusVar("", "", {}));
    stub.set_lamda(&ComputerItemWatcher::removeDevice, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&ComputerItemWatcher::onDeviceAdded, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&ComputerItemWatcher::onUpdateBlockItem, [] { __DBG_STUB_INVOKE__ });
    bool isLoop = false;
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [&isLoop]() -> QVariantMap { __DBG_STUB_INVOKE__ return { { GlobalServerDefines::DeviceProperty::kIsLoopDevice, isLoop } }; });

    EXPECT_NO_FATAL_FAILURE(ins->onDevicePropertyChangedQDBusVar("/org/freedesktop/UDisks2/block_devices/sbd1", GlobalServerDefines::DeviceProperty::kHintIgnore, QDBusVariant(false)));
    EXPECT_NO_FATAL_FAILURE(ins->onDevicePropertyChangedQDBusVar("/org/freedesktop/UDisks2/block_devices/sbd1", GlobalServerDefines::DeviceProperty::kHintIgnore, QDBusVariant(true)));
    EXPECT_NO_FATAL_FAILURE(ins->onDevicePropertyChangedQDBusVar("/org/freedesktop/UDisks2/block_devices/sbd1", GlobalServerDefines::DeviceProperty::kOptical, QDBusVariant(true)));
    EXPECT_NO_FATAL_FAILURE(ins->onDevicePropertyChangedQDBusVar("/org/freedesktop/UDisks2/block_devices/sbd1", GlobalServerDefines::DeviceProperty::kHasFileSystem, QDBusVariant(true)));

    isLoop = true;
    EXPECT_NO_FATAL_FAILURE(ins->onDevicePropertyChangedQDBusVar("/org/freedesktop/UDisks2/block_devices/sbd1", GlobalServerDefines::DeviceProperty::kHasFileSystem, QDBusVariant(true)));
}

TEST_F(UT_ComputerItemWatcher, OnGenAttributeChanged)
{
    EXPECT_NO_FATAL_FAILURE(ins->onGenAttributeChanged(Application::GenericAttribute::kShowFileSystemTagOnDiskIcon, true));
    EXPECT_NO_FATAL_FAILURE(ins->onGenAttributeChanged(Application::GenericAttribute::kHiddenSystemPartition, true));

    stub.set_lamda(&ComputerItemWatcher::removeDevice, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&ComputerItemWatcher::removeSidebarItem, [] { __DBG_STUB_INVOKE__ });

    EXPECT_NO_FATAL_FAILURE(ins->onGenAttributeChanged(Application::GenericAttribute::kAlwaysShowOfflineRemoteConnections, false));
    EXPECT_NO_FATAL_FAILURE(ins->onGenAttributeChanged(Application::GenericAttribute::kAlwaysShowOfflineRemoteConnections, true));
    EXPECT_NO_FATAL_FAILURE(ins->onGenAttributeChanged(Application::GenericAttribute::kHideLoopPartitions, true));
}

TEST_F(UT_ComputerItemWatcher, OnDConfigChanged)
{
    stub.set_lamda(&DConfigManager::value, [] { __DBG_STUB_INVOKE__ return QStringList {}; });
    stub.set_lamda(ComputerUtils::allValidBlockUUIDs, [] { __DBG_STUB_INVOKE__ return QStringList {}; });
    stub.set_lamda(ComputerUtils::blkDevUrlByUUIDs, [] { __DBG_STUB_INVOKE__ return QList<QUrl> {}; });
    EXPECT_NO_FATAL_FAILURE(ins->onDConfigChanged("org.deepin.dde.file-manager", "dfm.disk.hidden"));
}

TEST_F(UT_ComputerItemWatcher, OnBlockDeviceAdded)
{
    stub.set_lamda(&ComputerItemWatcher::onDeviceAdded, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->onBlockDeviceAdded("/org/freedesktop/UDisks2/block_devices/sdb1"));
}

TEST_F(UT_ComputerItemWatcher, OnBlockDeviceRemoved)
{
    stub.set_lamda(&ComputerItemWatcher::removeDevice, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->onBlockDeviceRemoved("/org/freedesktop/UDisks2/block_devices/sdc2"));
}

TEST_F(UT_ComputerItemWatcher, OnBlockDeviceMounted)
{
    stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [] { return QVariantMap { {} }; });
    stub.set_lamda(&ComputerItemWatcher::onUpdateBlockItem, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->onBlockDeviceMounted("", ""));
}

TEST_F(UT_ComputerItemWatcher, OnBlockDeviceUnmounted)
{
    stub.set_lamda(&ComputerItemWatcher::onUpdateBlockItem, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->onBlockDeviceUnmounted(""));
}

TEST_F(UT_ComputerItemWatcher, OnBlockDeviceLocked)
{
    stub.set_lamda(&ComputerItemWatcher::onUpdateBlockItem, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->onBlockDeviceLocked(""));
}

TEST_F(UT_ComputerItemWatcher, OnUpdateBlockItem)
{
    EXPECT_NO_FATAL_FAILURE(ins->onUpdateBlockItem("/org/freedesktop/UDisks2/block_devices/sdb1"));
}

TEST_F(UT_ComputerItemWatcher, OnProtocolDeviceMounted)
{
    stub.set_lamda(&ComputerItemWatcher::onDeviceAdded, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->onProtocolDeviceMounted("smb://1.2.3.4/", "mnt"));
}

TEST_F(UT_ComputerItemWatcher, OnProtocolDeeviceUnmounted)
{
    stub.set_lamda(&ComputerItemWatcher::onDeviceAdded, [] { __DBG_STUB_INVOKE__ });
    stub.set_lamda(&ComputerItemWatcher::removeDevice, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->onProtocolDeviceUnmounted("smb://1.2.3.4"));
}

TEST_F(UT_ComputerItemWatcher, OnDeviceSizeChanged)
{
    EXPECT_NO_FATAL_FAILURE(ins->onDeviceSizeChanged("/org/freedesktop/UDisks2/block_devices/sdb1", 0, 0));
}

TEST_F(UT_ComputerItemWatcher, OnProtocolDeviceRemoved)
{
    stub.set_lamda(&ComputerItemWatcher::removeDevice, [] { __DBG_STUB_INVOKE__ });
    EXPECT_NO_FATAL_FAILURE(ins->onProtocolDeviceRemoved("smb://1.2.3.4"));
}

TEST_F(UT_ComputerItemWatcher, InitConn)
{
    EXPECT_NO_FATAL_FAILURE(ins->initConn());
}

TEST_F(UT_ComputerItemWatcher, InitDeviceConn)
{
    EXPECT_NO_FATAL_FAILURE(ins->initDeviceConn());
}

TEST_F(UT_ComputerItemWatcher, InitAppWatcher)
{
    EXPECT_NO_FATAL_FAILURE(ins->initAppWatcher());
}

TEST_F(UT_ComputerItemWatcher, GetUserDirItems) { }
TEST_F(UT_ComputerItemWatcher, GetBlockDeviceItems) { }
TEST_F(UT_ComputerItemWatcher, GetProtocolDeviceItems) { }
TEST_F(UT_ComputerItemWatcher, GetStashedProtocolItems) { }
TEST_F(UT_ComputerItemWatcher, GetAppEntryItems) { }
TEST_F(UT_ComputerItemWatcher, AddGroup) { }
TEST_F(UT_ComputerItemWatcher, GetGroup) { }
TEST_F(UT_ComputerItemWatcher, UserDirGroup) { }
TEST_F(UT_ComputerItemWatcher, DiskGroup) { }
TEST_F(UT_ComputerItemWatcher, GetGroupId) { }
TEST_F(UT_ComputerItemWatcher, CacheItem) { }

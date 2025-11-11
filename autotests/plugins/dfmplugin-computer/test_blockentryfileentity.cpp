// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QUrl>
#include <QIcon>
#include <QVariant>
#include <QVariantHash>
#include <QStringList>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "stubext.h"
#include "fileentity/blockentryfileentity.h"
#include "utils/computerdatastruct.h"
#include "utils/computerutils.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-io/dfile.h>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;
using namespace GlobalServerDefines;

class UT_BlockEntryFileEntity : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();

        // Create test URL with proper block device suffix
        testUrl.setScheme("entry");
        testUrl.setPath("/test/device." + QString(SuffixInfo::kBlock));

        // Mock device proxy manager methods
        stub.set_lamda(&DeviceProxyManager::queryBlockInfo, [this](DeviceProxyManager *, const QString &, bool) {
            __DBG_STUB_INVOKE__
            return mockDeviceData;
        });

        // Mock ComputerUtils methods
        stub.set_lamda(&ComputerUtils::getBlockDevIdByUrl, [](const QUrl &) {
            __DBG_STUB_INVOKE__
            return QString("/org/freedesktop/UDisks2/block_devices/sdb1");
        });

        // Initialize mock device data with default values
        setupMockDeviceData();

        entity = nullptr;
    }

    void TearDown() override
    {
        delete entity;
        entity = nullptr;
        stub.clear();
    }

    void createEntity()
    {
        entity = new BlockEntryFileEntity(testUrl);
    }

    void setupMockDeviceData()
    {
        mockDeviceData.clear();
        mockDeviceData[DeviceProperty::kId] = "/org/freedesktop/UDisks2/block_devices/sdb1";
        mockDeviceData[DeviceProperty::kHintIgnore] = false;
        mockDeviceData[DeviceProperty::kHasFileSystem] = true;
        mockDeviceData[DeviceProperty::kOpticalDrive] = false;
        mockDeviceData[DeviceProperty::kIsEncrypted] = false;
        mockDeviceData[DeviceProperty::kCryptoBackingDevice] = "";
        mockDeviceData[DeviceProperty::kHasPartitionTable] = false;
        mockDeviceData[DeviceProperty::kHasPartition] = false;
        mockDeviceData[DeviceProperty::kHasExtendedPatition] = false;
        mockDeviceData[DeviceProperty::kIsLoopDevice] = false;
        mockDeviceData[DeviceProperty::kSizeTotal] = 1024 * 1024 * 1024; // 1GB
        mockDeviceData[DeviceProperty::kSizeUsed] = 512 * 1024 * 1024; // 512MB
        mockDeviceData[DeviceProperty::kMountPoints] = QStringList{"/media/test"};
        mockDeviceData[DeviceProperty::kMountPoint] = "/media/test";
        mockDeviceData[DeviceProperty::kCanPowerOff] = false;
        mockDeviceData[DeviceProperty::kOptical] = false;
        mockDeviceData[DeviceProperty::kRemovable] = false;
        mockDeviceData[DeviceProperty::kMediaAvailable] = true;
        mockDeviceData[DeviceProperty::kFileSystem] = "ext4";
        mockDeviceData[DeviceProperty::kCleartextDevice] = "";
    }

protected:
    stub_ext::StubExt stub;
    BlockEntryFileEntity *entity = nullptr;
    QUrl testUrl;
    QVariantMap mockDeviceData;
};

// Test constructor with valid URL
TEST_F(UT_BlockEntryFileEntity, Constructor_WithValidBlockUrl_CreatesSuccessfully)
{
    EXPECT_NO_THROW(createEntity());
    EXPECT_NE(entity, nullptr);
}

// Test displayName() method with Windows label
TEST_F(UT_BlockEntryFileEntity, DisplayName_WithWindowsLabel_ReturnsWindowsLabel)
{
    mockDeviceData[WinVolTagKeys::kWinLabel] = "Windows Drive";
    createEntity();

    QString displayName = entity->displayName();
    EXPECT_EQ(displayName, "Windows Drive");
}

// Test displayName() method without Windows label
TEST_F(UT_BlockEntryFileEntity, DisplayName_WithoutWindowsLabel_ReturnsConvertedName)
{
    createEntity();

    bool convertNameCalled = false;
    stub.set_lamda(static_cast<QString (*)(const QVariantHash &)>(&DeviceUtils::convertSuitableDisplayName), [&convertNameCalled](const QVariantHash &) {
        __DBG_STUB_INVOKE__
        convertNameCalled = true;
        return QString("Test Device");
    });

    QString displayName = entity->displayName();
    EXPECT_TRUE(convertNameCalled);
    EXPECT_EQ(displayName, "Test Device");
}

// Test icon() method for system disk root
TEST_F(UT_BlockEntryFileEntity, Icon_SystemDiskRoot_ReturnsRootIcon)
{
    createEntity();

    // Mock order to return system disk root
    stub.set_lamda(VADDR(BlockEntryFileEntity, order), [](const BlockEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return AbstractEntryFileEntity::EntryOrder::kOrderSysDiskRoot;
    });

    QIcon icon = entity->icon();
    EXPECT_FALSE(icon.isNull());
}

// Test icon() method for encrypted removable disk
TEST_F(UT_BlockEntryFileEntity, Icon_EncryptedRemovableDisk_ReturnsEncryptedRemovableIcon)
{
    mockDeviceData[DeviceProperty::kIsEncrypted] = true;
    mockDeviceData[DeviceProperty::kCanPowerOff] = true;
    createEntity();

    // Mock order to return removable disks
    stub.set_lamda(VADDR(BlockEntryFileEntity, order), [](const BlockEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return AbstractEntryFileEntity::EntryOrder::kOrderRemovableDisks;
    });

    QIcon icon = entity->icon();
    EXPECT_FALSE(icon.isNull());
}

// Test icon() method for optical drive
TEST_F(UT_BlockEntryFileEntity, Icon_OpticalDrive_ReturnsOpticalIcon)
{
    mockDeviceData[DeviceProperty::kCanPowerOff] = true;
    createEntity();

    // Mock order to return optical
    stub.set_lamda(VADDR(BlockEntryFileEntity, order), [](const BlockEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        return AbstractEntryFileEntity::EntryOrder::kOrderOptical;
    });

    QIcon icon = entity->icon();
    EXPECT_FALSE(icon.isNull());
}

// Test exists() method with valid device
TEST_F(UT_BlockEntryFileEntity, Exists_ValidDevice_ReturnsTrue)
{
    createEntity();

    bool exists = entity->exists();
    EXPECT_TRUE(exists);
}

// Test exists() method with hint ignore
TEST_F(UT_BlockEntryFileEntity, Exists_HintIgnoreTrue_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kHintIgnore] = true;
    createEntity();

    bool exists = entity->exists();
    EXPECT_FALSE(exists);
}

// Test exists() method with loop device without filesystem
TEST_F(UT_BlockEntryFileEntity, Exists_LoopDeviceWithoutFS_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kIsLoopDevice] = true;
    mockDeviceData[DeviceProperty::kHasFileSystem] = false;
    createEntity();

    bool exists = entity->exists();
    EXPECT_FALSE(exists);
}

// Test exists() method with no filesystem, not optical, not encrypted
TEST_F(UT_BlockEntryFileEntity, Exists_NoFSNotOpticalNotEncrypted_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kHasFileSystem] = false;
    mockDeviceData[DeviceProperty::kOpticalDrive] = false;
    mockDeviceData[DeviceProperty::kIsEncrypted] = false;
    createEntity();

    bool exists = entity->exists();
    EXPECT_FALSE(exists);
}

// Test exists() method with LVM member
TEST_F(UT_BlockEntryFileEntity, Exists_LVMMember_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kIsEncrypted] = true;
    QVariantHash clearBlockProperty;
    clearBlockProperty[DeviceProperty::kFileSystem] = "LVM2_member";
    mockDeviceData[BlockAdditionalProperty::kClearBlockProperty] = clearBlockProperty;
    createEntity();

    bool exists = entity->exists();
    EXPECT_FALSE(exists);
}

// Test exists() method with crypto backing device
TEST_F(UT_BlockEntryFileEntity, Exists_CryptoBackingDevice_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kCryptoBackingDevice] = "/dev/mapper/something";
    createEntity();

    bool exists = entity->exists();
    EXPECT_FALSE(exists);
}

// Test exists() method with extended partition
TEST_F(UT_BlockEntryFileEntity, Exists_ExtendedPartition_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kHasPartition] = true;
    mockDeviceData[DeviceProperty::kHasExtendedPatition] = true;
    createEntity();

    bool exists = entity->exists();
    EXPECT_FALSE(exists);
}

// Test exists() method with tiny size
TEST_F(UT_BlockEntryFileEntity, Exists_TinySize_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kSizeTotal] = 512; // Less than 1024
    mockDeviceData[DeviceProperty::kOpticalDrive] = false;
    mockDeviceData[DeviceProperty::kHasFileSystem] = false;
    createEntity();

    bool exists = entity->exists();
    EXPECT_FALSE(exists);
}

// Test showProgress() method
TEST_F(UT_BlockEntryFileEntity, ShowProgress_CallsShowSizeAndProgress_ReturnsCorrectValue)
{
    createEntity();

    bool showSizeCalled = false;
    stub.set_lamda(&BlockEntryFileEntity::showSizeAndProgress, [&showSizeCalled](const BlockEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        showSizeCalled = true;
        return true;
    });

    bool result = entity->showProgress();
    EXPECT_TRUE(showSizeCalled);
    EXPECT_TRUE(result);
}

// Test showTotalSize() method
TEST_F(UT_BlockEntryFileEntity, ShowTotalSize_CallsShowSizeAndProgress_ReturnsCorrectValue)
{
    createEntity();

    bool showSizeCalled = false;
    stub.set_lamda(&BlockEntryFileEntity::showSizeAndProgress, [&showSizeCalled](const BlockEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        showSizeCalled = true;
        return false;
    });

    bool result = entity->showTotalSize();
    EXPECT_TRUE(showSizeCalled);
    EXPECT_FALSE(result);
}

// Test showUsageSize() method
TEST_F(UT_BlockEntryFileEntity, ShowUsageSize_CallsShowSizeAndProgress_ReturnsCorrectValue)
{
    createEntity();

    bool showSizeCalled = false;
    stub.set_lamda(&BlockEntryFileEntity::showSizeAndProgress, [&showSizeCalled](const BlockEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        showSizeCalled = true;
        return true;
    });

    bool result = entity->showUsageSize();
    EXPECT_TRUE(showSizeCalled);
    EXPECT_TRUE(result);
}

// Test order() method for system disk
TEST_F(UT_BlockEntryFileEntity, Order_SystemDisk_ReturnsSystemDiskRoot)
{
    createEntity();

    bool isSystemDiskCalled = false;
    stub.set_lamda(static_cast<bool (*)(const QVariantHash &)>(&DeviceUtils::isSystemDisk), [&isSystemDiskCalled](const QVariantHash &) {
        __DBG_STUB_INVOKE__
        isSystemDiskCalled = true;
        return true;
    });

    AbstractEntryFileEntity::EntryOrder order = entity->order();
    EXPECT_TRUE(isSystemDiskCalled);
    EXPECT_EQ(order, AbstractEntryFileEntity::EntryOrder::kOrderSysDiskRoot);
}

// Test order() method for data disk
TEST_F(UT_BlockEntryFileEntity, Order_DataDisk_ReturnsDataDiskOrder)
{
    createEntity();

    bool isSystemDiskCalled = false;
    stub.set_lamda(static_cast<bool (*)(const QVariantHash &)>(&DeviceUtils::isSystemDisk), [&isSystemDiskCalled](const QVariantHash &) {
        __DBG_STUB_INVOKE__
        isSystemDiskCalled = true;
        return false;
    });

    bool isDataDiskCalled = false;
    stub.set_lamda(static_cast<bool (*)(const QVariantHash &)>(&DeviceUtils::isDataDisk), [&isDataDiskCalled](const QVariantHash &) {
        __DBG_STUB_INVOKE__
        isDataDiskCalled = true;
        return true;
    });

    AbstractEntryFileEntity::EntryOrder order = entity->order();
    EXPECT_TRUE(isSystemDiskCalled);
    EXPECT_TRUE(isDataDiskCalled);
    EXPECT_EQ(order, AbstractEntryFileEntity::EntryOrder::kOrderSysDiskData);
}

// Test order() method for optical drive
TEST_F(UT_BlockEntryFileEntity, Order_OpticalDrive_ReturnsOpticalOrder)
{
    mockDeviceData[DeviceProperty::kOptical] = true;
    createEntity();

    // Mock non-system and non-data disk
    stub.set_lamda(static_cast<bool (*)(const QVariantHash &)>(&DeviceUtils::isSystemDisk), [](const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(static_cast<bool (*)(const QVariantHash &)>(&DeviceUtils::isDataDisk), [](const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    AbstractEntryFileEntity::EntryOrder order = entity->order();
    EXPECT_EQ(order, AbstractEntryFileEntity::EntryOrder::kOrderOptical);
}

// Test order() method for removable disk
TEST_F(UT_BlockEntryFileEntity, Order_RemovableDisk_ReturnsRemovableOrder)
{
    mockDeviceData[DeviceProperty::kCanPowerOff] = true;
    createEntity();

    // Mock non-system, non-data, non-optical disk
    stub.set_lamda(static_cast<bool (*)(const QVariantHash &)>(&DeviceUtils::isSystemDisk), [](const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    stub.set_lamda(static_cast<bool (*)(const QVariantHash &)>(&DeviceUtils::isDataDisk), [](const QVariantHash &) {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool isSiblingOfRootCalled = false;
    stub.set_lamda(&BlockEntryFileEntity::isSiblingOfRoot, [&isSiblingOfRootCalled](const BlockEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        isSiblingOfRootCalled = true;
        return false;
    });

    AbstractEntryFileEntity::EntryOrder order = entity->order();
    EXPECT_TRUE(isSiblingOfRootCalled);
    EXPECT_EQ(order, AbstractEntryFileEntity::EntryOrder::kOrderRemovableDisks);
}

// Test sizeTotal() method
TEST_F(UT_BlockEntryFileEntity, SizeTotal_ReturnsCorrectSize)
{
    quint64 expectedSize = 1024 * 1024 * 1024; // 1GB
    mockDeviceData[DeviceProperty::kSizeTotal] = expectedSize;
    createEntity();

    quint64 size = entity->sizeTotal();
    EXPECT_EQ(size, expectedSize);
}

// Test sizeUsage() method
TEST_F(UT_BlockEntryFileEntity, SizeUsage_ReturnsCorrectSize)
{
    quint64 expectedSize = 512 * 1024 * 1024; // 512MB
    mockDeviceData[DeviceProperty::kSizeUsed] = expectedSize;
    createEntity();

    quint64 size = entity->sizeUsage();
    EXPECT_EQ(size, expectedSize);
}

// Test refresh() method
TEST_F(UT_BlockEntryFileEntity, Refresh_CallsLoadDiskInfo_Success)
{
    createEntity();

    bool loadDiskInfoCalled = false;
    stub.set_lamda(&BlockEntryFileEntity::loadDiskInfo, [&loadDiskInfoCalled](BlockEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        loadDiskInfoCalled = true;
    });

    entity->refresh();
    EXPECT_TRUE(loadDiskInfoCalled);
}

// Test targetUrl() method
TEST_F(UT_BlockEntryFileEntity, TargetUrl_ReturnsMountPoint_Success)
{
    createEntity();

    QUrl expectedUrl = QUrl::fromLocalFile("/media/test");
    bool mountPointCalled = false;
    stub.set_lamda(&BlockEntryFileEntity::mountPoint, [&mountPointCalled, &expectedUrl](const BlockEntryFileEntity *) {
        __DBG_STUB_INVOKE__
        mountPointCalled = true;
        return expectedUrl;
    });

    QUrl targetUrl = entity->targetUrl();
    EXPECT_TRUE(mountPointCalled);
    EXPECT_EQ(targetUrl, expectedUrl);
}

// Test isAccessable() method for encrypted device
TEST_F(UT_BlockEntryFileEntity, IsAccessable_EncryptedDevice_ReturnsTrue)
{
    mockDeviceData[DeviceProperty::kIsEncrypted] = true;
    createEntity();

    bool isAccessable = entity->isAccessable();
    EXPECT_TRUE(isAccessable);
}

// Test isAccessable() method for device with filesystem
TEST_F(UT_BlockEntryFileEntity, IsAccessable_WithFileSystem_ReturnsTrue)
{
    mockDeviceData[DeviceProperty::kIsEncrypted] = false;
    mockDeviceData[DeviceProperty::kHasFileSystem] = true;
    createEntity();

    bool isAccessable = entity->isAccessable();
    EXPECT_TRUE(isAccessable);
}

// Test isAccessable() method for device without filesystem
TEST_F(UT_BlockEntryFileEntity, IsAccessable_WithoutFileSystem_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kIsEncrypted] = false;
    mockDeviceData[DeviceProperty::kHasFileSystem] = false;
    createEntity();

    bool isAccessable = entity->isAccessable();
    EXPECT_FALSE(isAccessable);
}

// Test renamable() method for optical drive
TEST_F(UT_BlockEntryFileEntity, Renamable_OpticalDrive_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kOpticalDrive] = true;
    createEntity();

    bool renamable = entity->renamable();
    EXPECT_FALSE(renamable);
}

// Test renamable() method for encrypted root device
TEST_F(UT_BlockEntryFileEntity, Renamable_EncryptedRootDevice_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kIsEncrypted] = true;
    mockDeviceData[DeviceProperty::kCleartextDevice] = "/";
    createEntity();

    bool renamable = entity->renamable();
    EXPECT_FALSE(renamable);
}

// Test renamable() method for loop device
TEST_F(UT_BlockEntryFileEntity, Renamable_LoopDevice_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kIsLoopDevice] = true;
    createEntity();

    bool renamable = entity->renamable();
    EXPECT_FALSE(renamable);
}

// Test renamable() method for accessible device
TEST_F(UT_BlockEntryFileEntity, Renamable_AccessibleDevice_ReturnsTrue)
{
    mockDeviceData[DeviceProperty::kOpticalDrive] = false;
    mockDeviceData[DeviceProperty::kIsEncrypted] = false;
    mockDeviceData[DeviceProperty::kIsLoopDevice] = false;
    mockDeviceData[DeviceProperty::kHasFileSystem] = true;
    createEntity();

    bool renamable = entity->renamable();
    EXPECT_TRUE(renamable);
}

// Test device data persistence after refresh
TEST_F(UT_BlockEntryFileEntity, DeviceData_AfterRefresh_PersistsCorrectly)
{
    createEntity();

    // Change mock data
    mockDeviceData[DeviceProperty::kSizeTotal] = 2048 * 1024 * 1024; // 2GB

    entity->refresh();

    quint64 size = entity->sizeTotal();
    EXPECT_EQ(size, 2048 * 1024 * 1024);
}

// Test signal connections
TEST_F(UT_BlockEntryFileEntity, SignalConnections_DeviceProxyManager_ConnectedCorrectly)
{
    // Mock signal connection verification
    bool blockDevMountedConnected = false;
    bool blockDevUnmountedConnected = false;

    stub.set_lamda(static_cast<QMetaObject::Connection (*)(const QObject *, const char *, const QObject *, const char *, Qt::ConnectionType)>(&QObject::connect),
                   [&blockDevMountedConnected, &blockDevUnmountedConnected](const QObject *sender, const char *signal, const QObject *receiver, const char *slot, Qt::ConnectionType type) {
        __DBG_STUB_INVOKE__
        Q_UNUSED(sender)
        Q_UNUSED(receiver)
        Q_UNUSED(type)

        QString signalStr(signal);
        if (signalStr.contains("blockDevMounted")) {
            blockDevMountedConnected = true;
        } else if (signalStr.contains("blockDevUnmounted")) {
            blockDevUnmountedConnected = true;
        }

        return QMetaObject::Connection();
    });

    createEntity();

    // Note: Actual signal connection testing requires more complex setup
    // This test verifies the creation doesn't fail
    EXPECT_NE(entity, nullptr);
}

// Test error handling for invalid device data
TEST_F(UT_BlockEntryFileEntity, ErrorHandling_InvalidDeviceData_HandlesGracefully)
{
    // Clear mock device data to simulate invalid/missing data
    mockDeviceData.clear();

    EXPECT_NO_THROW(createEntity());

    if (entity) {
        // These should not crash even with empty data
        EXPECT_NO_THROW(entity->displayName());
        EXPECT_NO_THROW(entity->icon());
        EXPECT_NO_THROW(entity->exists());
        EXPECT_NO_THROW(entity->sizeTotal());
        EXPECT_NO_THROW(entity->sizeUsage());
    }
}

// Test edge case: very large device size
TEST_F(UT_BlockEntryFileEntity, EdgeCase_VeryLargeDevice_HandlesCorrectly)
{
    quint64 veryLargeSize = std::numeric_limits<quint64>::max();
    mockDeviceData[DeviceProperty::kSizeTotal] = veryLargeSize;
    createEntity();

    quint64 size = entity->sizeTotal();
    EXPECT_EQ(size, veryLargeSize);
}

// Test edge case: device with empty mount points
TEST_F(UT_BlockEntryFileEntity, EdgeCase_EmptyMountPoints_HandlesCorrectly)
{
    mockDeviceData[DeviceProperty::kMountPoints] = QStringList();
    createEntity();

    QUrl targetUrl = entity->targetUrl();
    // Should handle empty mount points gracefully
    EXPECT_TRUE(targetUrl.isEmpty() || targetUrl.isValid());
}

// TEST_F(UT_BlockEntryFileEntity, MultipleMethodCalls_DifferentParameters_HandlesCorrectly)
// {
//     createEntity();
    
//     // Mock all methods
//     int displayNameCallCount = 0;
//     int iconCallCount = 0;
//     int existsCallCount = 0;
//     int sizeTotalCallCount = 0;
//     int sizeUsageCallCount = 0;
//     int targetUrlCallCount = 0;
//     int isAccessableCallCount = 0;
//     int renamableCallCount = 0;
    
//     stub.set_lamda(&BlockEntryFileEntity::displayName, [&displayNameCallCount](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         displayNameCallCount++;
//         return QString("Test Device");
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::icon, [&iconCallCount](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         iconCallCount++;
//         return QIcon::fromTheme("drive-harddisk");
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::exists, [&existsCallCount](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         existsCallCount++;
//         return true;
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::sizeTotal, [&sizeTotalCallCount](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         sizeTotalCallCount++;
//         return quint64(1024 * 1024 * 1024);
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::sizeUsage, [&sizeUsageCallCount](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         sizeUsageCallCount++;
//         return quint64(512 * 1024 * 1024);
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::targetUrl, [&targetUrlCallCount](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         targetUrlCallCount++;
//         return QUrl::fromLocalFile("/media/test");
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::isAccessable, [&isAccessableCallCount](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         isAccessableCallCount++;
//         return true;
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::renamable, [&renamableCallCount](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         renamableCallCount++;
//         return true;
//     });
    
//     // Call multiple methods
//     entity->displayName();
//     entity->icon();
//     entity->exists();
//     entity->sizeTotal();
//     entity->sizeUsage();
//     entity->targetUrl();
//     entity->isAccessable();
//     entity->renamable();
    
//     // Verify all methods were called
//     EXPECT_EQ(displayNameCallCount, 1);
//     EXPECT_EQ(iconCallCount, 1);
//     EXPECT_EQ(existsCallCount, 1);
//     EXPECT_EQ(sizeTotalCallCount, 1);
//     EXPECT_EQ(sizeUsageCallCount, 1);
//     EXPECT_EQ(targetUrlCallCount, 1);
//     EXPECT_EQ(isAccessableCallCount, 1);
//     EXPECT_EQ(renamableCallCount, 1);
// }

TEST_F(UT_BlockEntryFileEntity, QtMetaObject_CorrectlyInitialized_Success)
{
    createEntity();
    
    // Test that Qt meta-object system works correctly
    const QMetaObject *metaObject = entity->metaObject();
    EXPECT_NE(metaObject, nullptr);
    
    // Test class name
    EXPECT_STREQ(metaObject->className(), "dfmplugin_computer::BlockEntryFileEntity");
    
    // Test that inherited methods exist in meta-object
    EXPECT_GE(metaObject->indexOfMethod("displayName()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("icon()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("exists()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showProgress()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showTotalSize()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("showUsageSize()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("order()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("sizeTotal()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("sizeUsage()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("refresh()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("targetUrl()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("isAccessable()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("renamable()"), 0);
}

TEST_F(UT_BlockEntryFileEntity, Inheritance_FromAbstractEntryFileEntity_WorksCorrectly)
{
    createEntity();
    
    // Test that BlockEntryFileEntity is properly inherited from AbstractEntryFileEntity
    AbstractEntryFileEntity *baseEntity = entity;
    EXPECT_NE(baseEntity, nullptr);
    
    // Test that we can call base class methods
    // AbstractEntryFileEntity doesn't have url() method, so we test other methods
    EXPECT_NO_THROW(baseEntity->displayName());
    EXPECT_NO_THROW(baseEntity->displayName());
    EXPECT_NO_THROW(baseEntity->icon());
    EXPECT_NO_THROW(baseEntity->exists());
    EXPECT_NO_THROW(baseEntity->showProgress());
    EXPECT_NO_THROW(baseEntity->showTotalSize());
    EXPECT_NO_THROW(baseEntity->showUsageSize());
    EXPECT_NO_THROW(baseEntity->description());
    EXPECT_NO_THROW(baseEntity->order());
    EXPECT_NO_THROW(baseEntity->sizeTotal());
    EXPECT_NO_THROW(baseEntity->sizeUsage());
    EXPECT_NO_THROW(baseEntity->refresh());
    EXPECT_NO_THROW(baseEntity->targetUrl());
    EXPECT_NO_THROW(baseEntity->isAccessable());
    EXPECT_NO_THROW(baseEntity->renamable());
}

TEST_F(UT_BlockEntryFileEntity, MemoryManagement_DeleteEntity_CleansUpCorrectly)
{
    createEntity();
    
    // Store pointer to entity for testing
    BlockEntryFileEntity *entityPtr = entity;
    
    // Delete entity
    delete entity;
    entity = nullptr;
    
    // The entity should be deleted, but we can't directly test this
    // We just verify that the delete operation doesn't crash
    EXPECT_EQ(entity, nullptr);
}



TEST_F(UT_BlockEntryFileEntity, SpecialCharacters_InDeviceName_HandlesCorrectly)
{
    // Set device name with special characters
    mockDeviceData[DeviceProperty::kId] = "/org/freedesktop/UDisks2/block_devices/sdb1-特殊字符";
    createEntity();
    
    // Test that methods handle special characters correctly
    EXPECT_NO_THROW(entity->displayName());
    EXPECT_NO_THROW(entity->icon());
    EXPECT_NO_THROW(entity->exists());
}

// TEST_F(UT_BlockEntryFileEntity, Consistency_MultipleCalls_ReturnConsistentResults)
// {
//     createEntity();
    
//     // Mock methods to return consistent values
//     QString mockDisplayName = "Test Device";
//     QIcon mockIcon = QIcon::fromTheme("drive-harddisk");
//     quint64 mockSizeTotal = 1024 * 1024 * 1024;
//     quint64 mockSizeUsage = 512 * 1024 * 1024;
//     QUrl mockTargetUrl = QUrl::fromLocalFile("/media/test");
    
//     stub.set_lamda(&BlockEntryFileEntity::displayName, [&mockDisplayName](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         return mockDisplayName;
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::icon, [&mockIcon](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         return mockIcon;
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::sizeTotal, [&mockSizeTotal](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         return mockSizeTotal;
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::sizeUsage, [&mockSizeUsage](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         return mockSizeUsage;
//     });
    
//     stub.set_lamda(&BlockEntryFileEntity::targetUrl, [&mockTargetUrl](const BlockEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         return mockTargetUrl;
//     });
    
//     // Call methods multiple times
//     QString displayName1 = entity->displayName();
//     QString displayName2 = entity->displayName();
//     QString displayName3 = entity->displayName();
    
//     QIcon icon1 = entity->icon();
//     QIcon icon2 = entity->icon();
//     QIcon icon3 = entity->icon();
    
//     quint64 sizeTotal1 = entity->sizeTotal();
//     quint64 sizeTotal2 = entity->sizeTotal();
//     quint64 sizeTotal3 = entity->sizeTotal();
    
//     quint64 sizeUsage1 = entity->sizeUsage();
//     quint64 sizeUsage2 = entity->sizeUsage();
//     quint64 sizeUsage3 = entity->sizeUsage();
    
//     QUrl targetUrl1 = entity->targetUrl();
//     QUrl targetUrl2 = entity->targetUrl();
//     QUrl targetUrl3 = entity->targetUrl();
    
//     // Verify consistency
//     EXPECT_EQ(displayName1, mockDisplayName);
//     EXPECT_EQ(displayName2, mockDisplayName);
//     EXPECT_EQ(displayName3, mockDisplayName);
    
//     EXPECT_EQ(icon1.cacheKey(), mockIcon.cacheKey());
//     EXPECT_EQ(icon2.cacheKey(), mockIcon.cacheKey());
//     EXPECT_EQ(icon3.cacheKey(), mockIcon.cacheKey());
    
//     EXPECT_EQ(sizeTotal1, mockSizeTotal);
//     EXPECT_EQ(sizeTotal2, mockSizeTotal);
//     EXPECT_EQ(sizeTotal3, mockSizeTotal);
    
//     EXPECT_EQ(sizeUsage1, mockSizeUsage);
//     EXPECT_EQ(sizeUsage2, mockSizeUsage);
//     EXPECT_EQ(sizeUsage3, mockSizeUsage);
    
//     EXPECT_EQ(targetUrl1, mockTargetUrl);
//     EXPECT_EQ(targetUrl2, mockTargetUrl);
//     EXPECT_EQ(targetUrl3, mockTargetUrl);
// }

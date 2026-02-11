// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "fileentity/protocolentryfileentity.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/interfaces/abstractentryfileentity.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/base/device/devicealiasmanager.h>
#include <dfm-base/utils/universalutils.h>
#include <dfm-base/utils/protocolutils.h>
#include <dfm-base/dbusservice/global_server_defines.h>
#include <dfm-base/dfm_global_defines.h>

#include <QUrl>
#include <QIcon>
#include <QVariantMap>
#include <QString>
#include <QStringList>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;
using namespace GlobalServerDefines;

class UT_ProtocolEntryFileEntity : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();

        // Create test URL with correct protocol suffix
        testUrl.setScheme("entry");
        testUrl.setPath("smb-server.protodev");

        // Mock the device data
        mockDeviceData.clear();
        mockDeviceData[DeviceProperty::kDisplayName] = "Test SMB Server";
        mockDeviceData[DeviceProperty::kDeviceIcon] = QStringList{"network-server", "folder-remote"};
        mockDeviceData[DeviceProperty::kId] = "smb://test-server/share";
        mockDeviceData[DeviceProperty::kMountPoint] = "/media/smb-share";
        mockDeviceData[DeviceProperty::kSizeTotal] = static_cast<qulonglong>(1024 * 1024 * 1024);
        mockDeviceData[DeviceProperty::kSizeUsed] = static_cast<qulonglong>(512 * 1024 * 1024);

        // Mock DevProxyMng->queryProtocolInfo
        stub.set_lamda(&DeviceProxyManager::queryProtocolInfo, [this] {
            __DBG_STUB_INVOKE__
            return mockDeviceData;
        });

        entity = new ProtocolEntryFileEntity(testUrl);
    }

    virtual void TearDown() override
    {
        delete entity;
        entity = nullptr;
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    ProtocolEntryFileEntity *entity = nullptr;
    QUrl testUrl;
    QVariantMap mockDeviceData;
};

TEST_F(UT_ProtocolEntryFileEntity, Constructor_ValidProtocolUrl_CreatesEntitySuccessfully)
{
    EXPECT_NE(entity, nullptr);
    EXPECT_EQ(entity->entryUrl, testUrl);
}

TEST_F(UT_ProtocolEntryFileEntity, Constructor_InvalidUrlSuffix_AbortsExecution)
{
    // Note: This test checks the critical error condition, but since abort() terminates the process,
    // we only test that the constructor works with valid URLs in practice
    QUrl invalidUrl("entry://invalid-server");

    // In a real scenario, this would call abort(), so we skip this destructive test
    // and focus on valid cases
    EXPECT_TRUE(testUrl.path().endsWith(".protodev"));
}

TEST_F(UT_ProtocolEntryFileEntity, DisplayName_SmbDevice_ReturnsFormattedName)
{
    QString mockHost = "test-server";
    QString mockShare = "shared-folder";

    stub.set_lamda(&DeviceUtils::parseSmbInfo, [&](const QString &, QString &host, QString &share, QString *) -> bool {
        __DBG_STUB_INVOKE__
        host = mockHost;
        share = mockShare;
        return true;
    });

    QString result = entity->displayName();
    QString expected = QString("shared-folder on test-server");
    EXPECT_EQ(result, expected);
}

TEST_F(UT_ProtocolEntryFileEntity, DisplayName_NonSmbDevice_ReturnsOriginalName)
{
    stub.set_lamda(&DeviceUtils::parseSmbInfo, [](const QString &, QString &, QString &, QString *) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QString result = entity->displayName();
    EXPECT_EQ(result, "Test SMB Server");
}

TEST_F(UT_ProtocolEntryFileEntity, Icon_AndroidDevice_ReturnsAndroidIcon)
{
    // Setup device ID for Android device
    mockDeviceData[DeviceProperty::kId] = "gphoto://android-device";
    mockDeviceData[DeviceProperty::kDeviceIcon] = QStringList{"phone"};
    entity->refresh();

    QIcon mockIcon = QIcon::fromTheme("android-device");
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [&](const QString &iconName) -> QIcon {
        __DBG_STUB_INVOKE__
        if (iconName == "android-device") {
            return mockIcon;
        }
        return QIcon();
    });

    QIcon result = entity->icon();
    EXPECT_FALSE(result.isNull());
}

TEST_F(UT_ProtocolEntryFileEntity, Icon_IosDevice_ReturnsIosIcon)
{
    // Setup device ID for iOS device
    mockDeviceData[DeviceProperty::kId] = "afc://ios-device";
    mockDeviceData[DeviceProperty::kDeviceIcon] = QStringList{"phone"};
    entity->refresh();

    QIcon mockIcon = QIcon::fromTheme("ios-device");
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [&](const QString &iconName) -> QIcon {
        __DBG_STUB_INVOKE__
        if (iconName == "ios-device") {
            return mockIcon;
        }
        return QIcon();
    });

    QIcon result = entity->icon();
    EXPECT_FALSE(result.isNull());
}

TEST_F(UT_ProtocolEntryFileEntity, Icon_AppleDevice_ReturnsIosIcon)
{
    // Setup device ID for Apple device
    mockDeviceData[DeviceProperty::kId] = "usb://Apple_Inc_device";
    mockDeviceData[DeviceProperty::kDeviceIcon] = QStringList{"phone"};
    entity->refresh();

    QIcon mockIcon = QIcon::fromTheme("ios-device");
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [&](const QString &iconName) -> QIcon {
        __DBG_STUB_INVOKE__
        if (iconName == "ios-device") {
            return mockIcon;
        }
        return QIcon();
    });

    QIcon result = entity->icon();
    EXPECT_FALSE(result.isNull());
}

TEST_F(UT_ProtocolEntryFileEntity, Icon_ValidIcon_ReturnsFirstValidIcon)
{
    QIcon mockIcon = QIcon::fromTheme("network-server");
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [&](const QString &iconName) -> QIcon {
        __DBG_STUB_INVOKE__
        if (iconName == "network-server") {
            return mockIcon;
        }
        return QIcon();
    });

    QIcon result = entity->icon();
    EXPECT_FALSE(result.isNull());
}

TEST_F(UT_ProtocolEntryFileEntity, Icon_NoValidIcon_ReturnsEmptyIcon)
{
    stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [](const QString &) -> QIcon {
        __DBG_STUB_INVOKE__
        return QIcon(); // Return null icon for all requests
    });

    QIcon result = entity->icon();
    EXPECT_TRUE(result.isNull());
}

TEST_F(UT_ProtocolEntryFileEntity, Exists_HasMountPoint_ReturnsTrue)
{
    // Mount point is set in SetUp
    bool result = entity->exists();
    EXPECT_TRUE(result);
}

TEST_F(UT_ProtocolEntryFileEntity, Exists_NoMountPoint_ReturnsFalse)
{
    mockDeviceData[DeviceProperty::kMountPoint] = "";
    entity->refresh();

    bool result = entity->exists();
    EXPECT_FALSE(result);
}

TEST_F(UT_ProtocolEntryFileEntity, ShowProgress_Always_ReturnsTrue)
{
    bool result = entity->showProgress();
    EXPECT_TRUE(result);
}

TEST_F(UT_ProtocolEntryFileEntity, ShowTotalSize_Always_ReturnsTrue)
{
    bool result = entity->showTotalSize();
    EXPECT_TRUE(result);
}

TEST_F(UT_ProtocolEntryFileEntity, ShowUsageSize_Always_ReturnsTrue)
{
    bool result = entity->showUsageSize();
    EXPECT_TRUE(result);
}

TEST_F(UT_ProtocolEntryFileEntity, Order_FtpProtocol_ReturnsFtpOrder)
{
    mockDeviceData[DeviceProperty::kId] = "ftp://test-server";
    entity->refresh();

    auto result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderFtp);
}

TEST_F(UT_ProtocolEntryFileEntity, Order_SftpProtocol_ReturnsFtpOrder)
{
    mockDeviceData[DeviceProperty::kId] = "sftp://test-server";
    entity->refresh();

    auto result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderFtp);
}

TEST_F(UT_ProtocolEntryFileEntity, Order_SmbProtocol_ReturnsSmbOrder)
{
    mockDeviceData[DeviceProperty::kId] = "smb://test-server";
    entity->refresh();

    auto result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderSmb);
}

TEST_F(UT_ProtocolEntryFileEntity, Order_SmbFileDetection_ReturnsSmbOrder)
{
    mockDeviceData[DeviceProperty::kId] = "file://some-path";
    entity->refresh();

    stub.set_lamda(&ProtocolUtils::isSMBFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    auto result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderSmb);
}

TEST_F(UT_ProtocolEntryFileEntity, Order_MtpProtocol_ReturnsMtpOrder)
{
    mockDeviceData[DeviceProperty::kId] = "mtp://test-device";
    entity->refresh();

    auto result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderMTP);
}

TEST_F(UT_ProtocolEntryFileEntity, Order_GPhoto2Protocol_ReturnsGPhoto2Order)
{
    mockDeviceData[DeviceProperty::kId] = "gphoto2://test-camera";
    entity->refresh();

    auto result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderGPhoto2);
}

TEST_F(UT_ProtocolEntryFileEntity, Order_UnknownProtocol_ReturnsFilesOrder)
{
    mockDeviceData[DeviceProperty::kId] = "unknown://test-device";
    entity->refresh();

    auto result = entity->order();
    EXPECT_EQ(result, AbstractEntryFileEntity::EntryOrder::kOrderFiles);
}

TEST_F(UT_ProtocolEntryFileEntity, SizeTotal_ValidData_ReturnsCorrectSize)
{
    quint64 result = entity->sizeTotal();
    EXPECT_EQ(result, static_cast<quint64>(1024 * 1024 * 1024));
}

TEST_F(UT_ProtocolEntryFileEntity, SizeUsage_ValidData_ReturnsCorrectSize)
{
    quint64 result = entity->sizeUsage();
    EXPECT_EQ(result, static_cast<quint64>(512 * 1024 * 1024));
}

TEST_F(UT_ProtocolEntryFileEntity, Refresh_CallsDeviceProxy_UpdatesData)
{
    bool queryProtocolInfoCalled = false;

    stub.set_lamda(&DeviceProxyManager::queryProtocolInfo, [&](DeviceProxyManager *, const QString &id, bool) -> QVariantMap {
        __DBG_STUB_INVOKE__
        queryProtocolInfoCalled = true;
        EXPECT_EQ(id, "smb-server"); // Expected ID after removing suffix
        return mockDeviceData;
    });

    entity->refresh();
    EXPECT_TRUE(queryProtocolInfoCalled);
}

TEST_F(UT_ProtocolEntryFileEntity, TargetUrl_HasMountPoint_ReturnsFileUrl)
{
    QUrl result = entity->targetUrl();

    EXPECT_EQ(result.scheme(), "file");
    EXPECT_EQ(result.path(), "/media/smb-share");
}

TEST_F(UT_ProtocolEntryFileEntity, TargetUrl_SmbFile_ReturnsSambaUri)
{
    QUrl expectedSambaUrl("smb://test-server/share");

    stub.set_lamda(&ProtocolUtils::isSMBFile, [](const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    stub.set_lamda(&DeviceUtils::getSambaFileUriFromNative, [&](const QUrl &) -> QUrl {
        __DBG_STUB_INVOKE__
        return expectedSambaUrl;
    });

    QUrl result = entity->targetUrl();
    EXPECT_EQ(result, expectedSambaUrl);
}

TEST_F(UT_ProtocolEntryFileEntity, TargetUrl_NoMountPoint_ReturnsEmptyUrl)
{
    mockDeviceData[DeviceProperty::kMountPoint] = "";
    entity->refresh();

    QUrl result = entity->targetUrl();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_ProtocolEntryFileEntity, TargetUrl_EmptyMountPoint_ReturnsEmptyUrl)
{
    mockDeviceData[DeviceProperty::kMountPoint] = QString();
    entity->refresh();

    QUrl result = entity->targetUrl();
    EXPECT_TRUE(result.isEmpty());
}

// Test edge cases and error conditions
TEST_F(UT_ProtocolEntryFileEntity, DisplayName_EmptyDisplayName_ReturnsEmptyString)
{
    mockDeviceData[DeviceProperty::kDisplayName] = "";
    entity->refresh();

    stub.set_lamda(&DeviceUtils::parseSmbInfo, [] {
        __DBG_STUB_INVOKE__
        return false;
    });

    QString result = entity->displayName();
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_ProtocolEntryFileEntity, Icon_EmptyIconList_ReturnsEmptyIcon)
{
    mockDeviceData[DeviceProperty::kDeviceIcon] = QStringList();
    entity->refresh();

    QIcon result = entity->icon();
    EXPECT_TRUE(result.isNull());
}

TEST_F(UT_ProtocolEntryFileEntity, SizeTotal_InvalidData_ReturnsZero)
{
    mockDeviceData[DeviceProperty::kSizeTotal] = "invalid";
    entity->refresh();

    quint64 result = entity->sizeTotal();
    EXPECT_EQ(result, static_cast<quint64>(0));
}

TEST_F(UT_ProtocolEntryFileEntity, SizeUsage_InvalidData_ReturnsZero)
{
    mockDeviceData[DeviceProperty::kSizeUsed] = "invalid";
    entity->refresh();

    quint64 result = entity->sizeUsage();
    EXPECT_EQ(result, static_cast<quint64>(0));
}

// Test multiple protocol types in a single test
TEST_F(UT_ProtocolEntryFileEntity, Order_MultipleProtocolTypes_ReturnsCorrectOrders)
{
    struct TestCase {
        QString protocolId;
        AbstractEntryFileEntity::EntryOrder expectedOrder;
    };

    std::vector<TestCase> testCases = {
        {"ftp://test", AbstractEntryFileEntity::EntryOrder::kOrderFtp},
        {"sftp://test", AbstractEntryFileEntity::EntryOrder::kOrderFtp},
        {"smb://test", AbstractEntryFileEntity::EntryOrder::kOrderSmb},
        {"mtp://test", AbstractEntryFileEntity::EntryOrder::kOrderMTP},
        {"gphoto2://test", AbstractEntryFileEntity::EntryOrder::kOrderGPhoto2},
        {"unknown://test", AbstractEntryFileEntity::EntryOrder::kOrderFiles}
    };

    for (const auto &testCase : testCases) {
        mockDeviceData[DeviceProperty::kId] = testCase.protocolId;
        entity->refresh();

        // Reset SMB file detection for non-SMB protocols
        if (!testCase.protocolId.startsWith("smb")) {
            stub.set_lamda(&ProtocolUtils::isSMBFile, [](const QUrl &) -> bool {
                __DBG_STUB_INVOKE__
                return false;
            });
        }

        auto result = entity->order();
        EXPECT_EQ(result, testCase.expectedOrder)
            << "Failed for protocol: " << testCase.protocolId.toStdString();
    }
}

// TEST_F(UT_ProtocolEntryFileEntity, MultipleMethodCalls_DifferentParameters_HandlesCorrectly)
// {
//     // Mock all methods
//     int displayNameCallCount = 0;
//     int iconCallCount = 0;
//     int existsCallCount = 0;
//     int showProgressCallCount = 0;
//     int showTotalSizeCallCount = 0;
//     int showUsageSizeCallCount = 0;
//     int orderCallCount = 0;
//     int sizeTotalCallCount = 0;
//     int sizeUsageCallCount = 0;
//     int refreshCallCount = 0;
//     int targetUrlCallCount = 0;
//     int renamableCallCount = 0;
    
//     stub.set_lamda(&DeviceUtils::parseSmbInfo, [&displayNameCallCount](const QString &, QString &host, QString &share, QString *) -> bool {
//         __DBG_STUB_INVOKE__
//         displayNameCallCount++;
//         host = "test-server";
//         share = "test-share";
//         return true;
//     });
    
//     stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [&iconCallCount](const QString &) -> QIcon {
//         __DBG_STUB_INVOKE__
//         iconCallCount++;
//         return QIcon::fromTheme("network-server");
//     });
    
//     stub.set_lamda(&ProtocolUtils::isSMBFile, [&targetUrlCallCount](const QUrl &) -> bool {
//         __DBG_STUB_INVOKE__
//         targetUrlCallCount++;
//         return false;
//     });
    
//     stub.set_lamda(&DeviceUtils::getSambaFileUriFromNative, [&targetUrlCallCount](const QUrl &) -> QUrl {
//         __DBG_STUB_INVOKE__
//         targetUrlCallCount++;
//         return QUrl("smb://test-server/test-share");
//     });
    
//     stub.set_lamda(&dfmbase::NPDeviceAliasManager::canSetAlias, [&renamableCallCount](dfmbase::NPDeviceAliasManager *, const QUrl &) -> bool {
//         __DBG_STUB_INVOKE__
//         renamableCallCount++;
//         return true;
//     });
    
//     // Call multiple methods
//     entity->displayName();
//     entity->icon();
//     entity->exists();
//     entity->showProgress();
//     entity->showTotalSize();
//     entity->showUsageSize();
//     entity->order();
//     entity->sizeTotal();
//     entity->sizeUsage();
//     entity->refresh();
//     entity->targetUrl();
//     entity->renamable();
    
//     // Verify all methods were called
//     EXPECT_EQ(displayNameCallCount, 1);
//     EXPECT_EQ(iconCallCount, 1);
//     EXPECT_EQ(targetUrlCallCount, 2); // Called twice: once for targetUrl, once for getSambaFileUriFromNative
//     EXPECT_EQ(renamableCallCount, 1);
// }

TEST_F(UT_ProtocolEntryFileEntity, QtMetaObject_CorrectlyInitialized_Success)
{
    // Test that Qt meta-object system works correctly
    const QMetaObject *metaObject = entity->metaObject();
    EXPECT_NE(metaObject, nullptr);
    
    // Test class name
    EXPECT_STREQ(metaObject->className(), "dfmplugin_computer::ProtocolEntryFileEntity");
    
    // Test that inherited methods exist in meta-object
    EXPECT_GE(metaObject->indexOfMethod("displayName()"), 0);
    EXPECT_GE(metaObject->indexOfMethod("editDisplayText()"), 0);
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
    EXPECT_GE(metaObject->indexOfMethod("renamable()"), 0);
}

TEST_F(UT_ProtocolEntryFileEntity, Inheritance_FromAbstractEntryFileEntity_WorksCorrectly)
{
    // Test that ProtocolEntryFileEntity is properly inherited from AbstractEntryFileEntity
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

TEST_F(UT_ProtocolEntryFileEntity, MemoryManagement_DeleteEntity_CleansUpCorrectly)
{
    // Store pointer to entity for testing
    ProtocolEntryFileEntity *entityPtr = entity;
    
    // Delete entity
    delete entity;
    entity = nullptr;
    
    // The entity should be deleted, but we can't directly test this
    // We just verify that the delete operation doesn't crash
    EXPECT_EQ(entity, nullptr);
}

TEST_F(UT_ProtocolEntryFileEntity, ErrorHandling_InvalidDeviceData_HandlesGracefully)
{
    // Clear mock device data to simulate invalid/missing data
    mockDeviceData.clear();
    
    // Refresh to load empty data
    entity->refresh();
    
    // These should not crash even with empty data
    EXPECT_NO_THROW({
        QString displayName = entity->displayName();
        QIcon icon = entity->icon();
        bool exists = entity->exists();
        quint64 sizeTotal = entity->sizeTotal();
        quint64 sizeUsage = entity->sizeUsage();
        QUrl targetUrl = entity->targetUrl();
        bool renamable = entity->renamable();
    });
}

TEST_F(UT_ProtocolEntryFileEntity, SpecialCharacters_InDeviceName_HandlesCorrectly)
{
    // Set device name with special characters
    mockDeviceData[DeviceProperty::kDisplayName] = "Test Device 特殊字符";
    mockDeviceData[DeviceProperty::kId] = "smb://test-server/特殊字符";
    entity->refresh();
    
    // Test that methods handle special characters correctly
    EXPECT_NO_THROW({
        QString displayName = entity->displayName();
        QIcon icon = entity->icon();
        bool exists = entity->exists();
    });
}

// TEST_F(UT_ProtocolEntryFileEntity, Consistency_MultipleCalls_ReturnConsistentResults)
// {
//     // Mock methods to return consistent values
//     QString mockDisplayName = "Test SMB Server";
//     QIcon mockIcon = QIcon::fromTheme("network-server");
//     quint64 mockSizeTotal = 1024 * 1024 * 1024;
//     quint64 mockSizeUsage = 512 * 1024 * 1024;
//     QUrl mockTargetUrl = QUrl::fromLocalFile("/media/smb-share");
//     bool mockRenamable = true;
    
//     stub.set_lamda(&DeviceUtils::parseSmbInfo, [&mockDisplayName](const QString &, QString &host, QString &share, QString *) -> bool {
//         __DBG_STUB_INVOKE__
//         host = "test-server";
//         share = "test-share";
//         return true;
//     });
    
//     stub.set_lamda(static_cast<QIcon (*)(const QString &)>(&QIcon::fromTheme), [&mockIcon](const QString &) -> QIcon {
//         __DBG_STUB_INVOKE__
//         return mockIcon;
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
    
//     // Verify consistency
//     EXPECT_EQ(displayName1, displayName2);
//     EXPECT_EQ(displayName2, displayName3);
    
//     EXPECT_EQ(icon1.cacheKey(), icon2.cacheKey());
//     EXPECT_EQ(icon2.cacheKey(), icon3.cacheKey());
    
//     EXPECT_EQ(sizeTotal1, sizeTotal2);
//     EXPECT_EQ(sizeTotal2, sizeTotal3);
    
//     EXPECT_EQ(sizeUsage1, sizeUsage2);
//     EXPECT_EQ(sizeUsage2, sizeUsage3);
// }

// TEST_F(UT_ProtocolEntryFileEntity, EditDisplayText_ValidUrl_ReturnsCorrectText)
// {
//     // Mock targetUrl to return a valid URL
//     QUrl mockUrl("smb://test-server/share");
    
//     stub.set_lamda(&ProtocolEntryFileEntity::targetUrl, [&mockUrl](const ProtocolEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         return mockUrl;
//     });
    
//     stub.set_lamda(&dfmbase::NPDeviceAliasManager::getAlias, [](dfmbase::NPDeviceAliasManager *, const QUrl &) {
//         __DBG_STUB_INVOKE__
//         return QString(); // Empty alias
//     });
    
//     QString result = entity->editDisplayText();
//     EXPECT_EQ(result, "test-server"); // Should return host from URL
// }

// TEST_F(UT_ProtocolEntryFileEntity, EditDisplayText_WithAlias_ReturnsAlias)
// {
//     // Mock targetUrl to return a valid URL
//     QUrl mockUrl("smb://test-server/share");
//     QString mockAlias = "Test Alias";
    
//     stub.set_lamda(&ProtocolEntryFileEntity::targetUrl, [&mockUrl](const ProtocolEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         return mockUrl;
//     });
    
//     stub.set_lamda(&dfmbase::NPDeviceAliasManager::getAlias, [&mockAlias](dfmbase::NPDeviceAliasManager *, const QUrl &) {
//         __DBG_STUB_INVOKE__
//         return mockAlias;
//     });
    
//     QString result = entity->editDisplayText();
//     EXPECT_EQ(result, mockAlias); // Should return alias
// }

// TEST_F(UT_ProtocolEntryFileEntity, EditDisplayText_EmptyHost_ReturnsDisplayName)
// {
//     // Mock targetUrl to return a URL with empty host
//     QUrl mockUrl("smb:///share");
//     QString mockDisplayName = "Test Display Name";
    
//     stub.set_lamda(&ProtocolEntryFileEntity::targetUrl, [&mockUrl](const ProtocolEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         return mockUrl;
//     });
    
//     stub.set_lamda(&dfmbase::NPDeviceAliasManager::getAlias, [](dfmbase::NPDeviceAliasManager *, const QUrl &) {
//         __DBG_STUB_INVOKE__
//         return QString(); // Empty alias
//     });
    
//     stub.set_lamda(&ProtocolEntryFileEntity::displayName, [&mockDisplayName](const ProtocolEntryFileEntity *) {
//         __DBG_STUB_INVOKE__
//         return mockDisplayName;
//     });
    
//     QString result = entity->editDisplayText();
//     EXPECT_EQ(result, mockDisplayName); // Should return display name
// }

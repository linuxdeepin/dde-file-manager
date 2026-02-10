// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/file/entry/entryfileinfo.h>

#include <QUrl>
#include <QWidget>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_ComputerDataStruct : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

TEST_F(UT_ComputerDataStruct, ComputerItemData_DefaultConstruction_InitializesCorrectly)
{
    ComputerItemData data;

    EXPECT_TRUE(data.url.isEmpty());
    EXPECT_EQ(data.shape, ComputerItemData::kSmallItem);
    EXPECT_TRUE(data.itemName.isEmpty());
    EXPECT_EQ(data.groupId, 0);
    EXPECT_EQ(data.widget, nullptr);
    EXPECT_FALSE(data.isEditing);
    EXPECT_FALSE(data.isElided);
    EXPECT_EQ(data.info, nullptr);
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_WithValidData_SetsPropertiesCorrectly)
{
    ComputerItemData data;
    QUrl testUrl("entry://test.blockdev");
    QString testName = "Test Device";
    QWidget *testWidget = new QWidget();
    DFMEntryFileInfoPointer testInfo = nullptr; // Would normally be a valid pointer

    data.url = testUrl;
    data.shape = ComputerItemData::kLargeItem;
    data.itemName = testName;
    data.groupId = 1;
    data.widget = testWidget;
    data.isEditing = true;
    data.isElided = true;
    data.info = testInfo;

    EXPECT_EQ(data.url, testUrl);
    EXPECT_EQ(data.shape, ComputerItemData::kLargeItem);
    EXPECT_EQ(data.itemName, testName);
    EXPECT_EQ(data.groupId, 1);
    EXPECT_EQ(data.widget, testWidget);
    EXPECT_TRUE(data.isEditing);
    EXPECT_TRUE(data.isElided);
    EXPECT_EQ(data.info, testInfo);

    delete testWidget;
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_ShapeTypes_AllValuesValid)
{
    EXPECT_EQ(ComputerItemData::kSmallItem, 0);
    EXPECT_EQ(ComputerItemData::kLargeItem, 1);
    EXPECT_EQ(ComputerItemData::kSplitterItem, 2);
    EXPECT_EQ(ComputerItemData::kWidgetItem, 3);
}

TEST_F(UT_ComputerDataStruct, SuffixInfo_Constants_HaveExpectedValues)
{
    EXPECT_STREQ(SuffixInfo::kCommon, "_common_");
    EXPECT_STREQ(SuffixInfo::kAppEntry, "appentry");
    EXPECT_STREQ(SuffixInfo::kBlock, "blockdev");
    EXPECT_STREQ(SuffixInfo::kProtocol, "protodev");
    EXPECT_STREQ(SuffixInfo::kUserDir, "userdir");
}

TEST_F(UT_ComputerDataStruct, SuffixInfo_Constants_AreNonEmpty)
{
    EXPECT_GT(strlen(SuffixInfo::kCommon), 0);
    EXPECT_GT(strlen(SuffixInfo::kAppEntry), 0);
    EXPECT_GT(strlen(SuffixInfo::kBlock), 0);
    EXPECT_GT(strlen(SuffixInfo::kProtocol), 0);
    EXPECT_GT(strlen(SuffixInfo::kUserDir), 0);
}

TEST_F(UT_ComputerDataStruct, DeviceId_Constants_HaveExpectedValues)
{
    EXPECT_STREQ(DeviceId::kBlockDeviceIdPrefix, "/org/freedesktop/UDisks2/block_devices/");
}

TEST_F(UT_ComputerDataStruct, DeviceId_Constants_AreValidPaths)
{
    QString prefix(DeviceId::kBlockDeviceIdPrefix);
    EXPECT_TRUE(prefix.startsWith("/"));
    EXPECT_TRUE(prefix.endsWith("/"));
    EXPECT_TRUE(prefix.contains("UDisks2"));
}

TEST_F(UT_ComputerDataStruct, ContextMenuAction_Constants_HaveExpectedValues)
{
    EXPECT_STREQ(ContextMenuAction::kOpen, "computer-open");
    EXPECT_STREQ(ContextMenuAction::kOpenInNewWin, "computer-open-in-win");
    EXPECT_STREQ(ContextMenuAction::kOpenInNewTab, "computer-open-in-tab");
    EXPECT_STREQ(ContextMenuAction::kMount, "computer-mount");
    EXPECT_STREQ(ContextMenuAction::kUnmount, "computer-unmount");
    EXPECT_STREQ(ContextMenuAction::kRename, "computer-rename");
    EXPECT_STREQ(ContextMenuAction::kFormat, "computer-format");
    EXPECT_STREQ(ContextMenuAction::kEject, "computer-eject");
    EXPECT_STREQ(ContextMenuAction::kErase, "computer-erase");
    EXPECT_STREQ(ContextMenuAction::kSafelyRemove, "computer-safely-remove");
    EXPECT_STREQ(ContextMenuAction::kLogoutAndForget, "computer-logout-and-forget-passwd");
    EXPECT_STREQ(ContextMenuAction::kProperty, "computer-property");
    EXPECT_STREQ(ContextMenuAction::kActionTriggeredFromSidebar, "trigger-from-sidebar");
}

TEST_F(UT_ComputerDataStruct, ContextMenuAction_Constants_AllStartWithPrefix)
{
    QString open(ContextMenuAction::kOpen);
    QString openWin(ContextMenuAction::kOpenInNewWin);
    QString openTab(ContextMenuAction::kOpenInNewTab);
    QString mount(ContextMenuAction::kMount);
    QString unmount(ContextMenuAction::kUnmount);
    QString rename(ContextMenuAction::kRename);
    QString format(ContextMenuAction::kFormat);
    QString eject(ContextMenuAction::kEject);
    QString erase(ContextMenuAction::kErase);
    QString remove(ContextMenuAction::kSafelyRemove);
    QString logout(ContextMenuAction::kLogoutAndForget);
    QString property(ContextMenuAction::kProperty);

    EXPECT_TRUE(open.startsWith("computer-"));
    EXPECT_TRUE(openWin.startsWith("computer-"));
    EXPECT_TRUE(openTab.startsWith("computer-"));
    EXPECT_TRUE(mount.startsWith("computer-"));
    EXPECT_TRUE(unmount.startsWith("computer-"));
    EXPECT_TRUE(rename.startsWith("computer-"));
    EXPECT_TRUE(format.startsWith("computer-"));
    EXPECT_TRUE(eject.startsWith("computer-"));
    EXPECT_TRUE(erase.startsWith("computer-"));
    EXPECT_TRUE(remove.startsWith("computer-"));
    EXPECT_TRUE(logout.startsWith("computer-"));
    EXPECT_TRUE(property.startsWith("computer-"));
}

TEST_F(UT_ComputerDataStruct, ContextMenuAction_TranslationFunctions_ReturnNonEmptyStrings)
{
    // Test that translation functions exist and return non-empty strings
    QString openText = ContextMenuAction::trOpen();
    QString openWinText = ContextMenuAction::trOpenInNewWin();
    QString openTabText = ContextMenuAction::trOpenInNewTab();
    QString mountText = ContextMenuAction::trMount();
    QString unmountText = ContextMenuAction::trUnmount();
    QString renameText = ContextMenuAction::trRename();
    QString formatText = ContextMenuAction::trFormat();
    QString ejectText = ContextMenuAction::trEject();
    QString eraseText = ContextMenuAction::trErase();
    QString removeText = ContextMenuAction::trSafelyRemove();
    QString logoutText = ContextMenuAction::trLogoutAndClearSavedPasswd();
    QString propertyText = ContextMenuAction::trProperties();

    EXPECT_FALSE(openText.isEmpty());
    EXPECT_FALSE(openWinText.isEmpty());
    EXPECT_FALSE(openTabText.isEmpty());
    EXPECT_FALSE(mountText.isEmpty());
    EXPECT_FALSE(unmountText.isEmpty());
    EXPECT_FALSE(renameText.isEmpty());
    EXPECT_FALSE(formatText.isEmpty());
    EXPECT_FALSE(ejectText.isEmpty());
    EXPECT_FALSE(eraseText.isEmpty());
    EXPECT_FALSE(removeText.isEmpty());
    EXPECT_FALSE(logoutText.isEmpty());
    EXPECT_FALSE(propertyText.isEmpty());
}

TEST_F(UT_ComputerDataStruct, ContextMenuAction_TranslationFunctions_ReturnUniqueStrings)
{
    // Test that different translation functions return different strings
    QString openText = ContextMenuAction::trOpen();
    QString mountText = ContextMenuAction::trMount();
    QString renameText = ContextMenuAction::trRename();
    QString formatText = ContextMenuAction::trFormat();

    EXPECT_NE(openText, mountText);
    EXPECT_NE(openText, renameText);
    EXPECT_NE(openText, formatText);
    EXPECT_NE(mountText, renameText);
    EXPECT_NE(mountText, formatText);
    EXPECT_NE(renameText, formatText);
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_CopyConstruction_CopiesAllFields)
{
    ComputerItemData original;
    original.url = QUrl("entry://test.blockdev");
    original.shape = ComputerItemData::kLargeItem;
    original.itemName = "Test Device";
    original.groupId = 5;
    original.isEditing = true;
    original.isElided = true;

    ComputerItemData copy = original;

    EXPECT_EQ(copy.url, original.url);
    EXPECT_EQ(copy.shape, original.shape);
    EXPECT_EQ(copy.itemName, original.itemName);
    EXPECT_EQ(copy.groupId, original.groupId);
    EXPECT_EQ(copy.isEditing, original.isEditing);
    EXPECT_EQ(copy.isElided, original.isElided);
    EXPECT_EQ(copy.widget, original.widget);
    EXPECT_EQ(copy.info, original.info);
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_Assignment_AssignsAllFields)
{
    ComputerItemData original;
    original.url = QUrl("entry://test.blockdev");
    original.shape = ComputerItemData::kSplitterItem;
    original.itemName = "Splitter";
    original.groupId = 10;
    original.isEditing = false;
    original.isElided = true;

    ComputerItemData assigned;
    assigned = original;

    EXPECT_EQ(assigned.url, original.url);
    EXPECT_EQ(assigned.shape, original.shape);
    EXPECT_EQ(assigned.itemName, original.itemName);
    EXPECT_EQ(assigned.groupId, original.groupId);
    EXPECT_EQ(assigned.isEditing, original.isEditing);
    EXPECT_EQ(assigned.isElided, original.isElided);
    EXPECT_EQ(assigned.widget, original.widget);
    EXPECT_EQ(assigned.info, original.info);
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_VisibleFlag_DefaultsToTrue)
{
    ComputerItemData data;
    EXPECT_TRUE(data.isVisible);
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_VisibleFlag_CanBeSetToFalse)
{
    ComputerItemData data;
    data.isVisible = false;
    EXPECT_FALSE(data.isVisible);
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_Comparison_OperatorsWorkCorrectly)
{
    ComputerItemData data1, data2;
    
    // Set same values
    data1.url = QUrl("entry://test.blockdev");
    data1.shape = ComputerItemData::kLargeItem;
    data1.itemName = "Test Device";
    data1.groupId = 1;
    
    data2.url = QUrl("entry://test.blockdev");
    data2.shape = ComputerItemData::kLargeItem;
    data2.itemName = "Test Device";
    data2.groupId = 1;
    
    // Test equality (implicitly through field comparison)
    EXPECT_EQ(data1.url, data2.url);
    EXPECT_EQ(data1.shape, data2.shape);
    EXPECT_EQ(data1.itemName, data2.itemName);
    EXPECT_EQ(data1.groupId, data2.groupId);
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_DifferentShapeTypes_HaveCorrectValues)
{
    ComputerItemData smallItem, largeItem, splitterItem, widgetItem;
    
    smallItem.shape = ComputerItemData::kSmallItem;
    largeItem.shape = ComputerItemData::kLargeItem;
    splitterItem.shape = ComputerItemData::kSplitterItem;
    widgetItem.shape = ComputerItemData::kWidgetItem;
    
    EXPECT_EQ(smallItem.shape, 0);
    EXPECT_EQ(largeItem.shape, 1);
    EXPECT_EQ(splitterItem.shape, 2);
    EXPECT_EQ(widgetItem.shape, 3);
    
    EXPECT_NE(smallItem.shape, largeItem.shape);
    EXPECT_NE(largeItem.shape, splitterItem.shape);
    EXPECT_NE(splitterItem.shape, widgetItem.shape);
}

TEST_F(UT_ComputerDataStruct, SuffixInfo_StringOperations_WorkCorrectly)
{
    QString common(SuffixInfo::kCommon);
    QString appEntry(SuffixInfo::kAppEntry);
    QString block(SuffixInfo::kBlock);
    QString protocol(SuffixInfo::kProtocol);
    QString userDir(SuffixInfo::kUserDir);
    
    // Test string operations
    EXPECT_TRUE(common.contains("_"));
    EXPECT_TRUE(appEntry.contains("app"));
    EXPECT_TRUE(block.contains("dev"));
    EXPECT_TRUE(protocol.contains("proto"));
    EXPECT_TRUE(userDir.contains("dir"));
    
    // Test that they are different
    EXPECT_NE(common, appEntry);
    EXPECT_NE(appEntry, block);
    EXPECT_NE(block, protocol);
    EXPECT_NE(protocol, userDir);
}

TEST_F(UT_ComputerDataStruct, DeviceId_PrefixOperations_WorkCorrectly)
{
    QString prefix(DeviceId::kBlockDeviceIdPrefix);
    
    // Test prefix operations
    EXPECT_TRUE(prefix.startsWith("/org/"));
    EXPECT_TRUE(prefix.contains("freedesktop"));
    EXPECT_TRUE(prefix.contains("UDisks2"));
    EXPECT_TRUE(prefix.contains("block_devices"));
    
    // Test building a full device ID
    QString deviceId = prefix + "sda1";
    EXPECT_TRUE(deviceId.endsWith("sda1"));
    EXPECT_TRUE(deviceId.contains("block_devices/sda1"));
}

TEST_F(UT_ComputerDataStruct, ContextMenuAction_SpecialConstants_HaveCorrectValues)
{
    EXPECT_STREQ(ContextMenuAction::kActionTriggeredFromSidebar, "trigger-from-sidebar");
    
    QString sidebarTrigger(ContextMenuAction::kActionTriggeredFromSidebar);
    EXPECT_TRUE(sidebarTrigger.contains("sidebar"));
    EXPECT_TRUE(sidebarTrigger.contains("trigger"));
}

TEST_F(UT_ComputerDataStruct, ContextMenuAction_AllConstants_AreUnique)
{
    // Collect all action strings
    QStringList actions;
    actions << ContextMenuAction::kOpen
             << ContextMenuAction::kOpenInNewWin
             << ContextMenuAction::kOpenInNewTab
             << ContextMenuAction::kMount
             << ContextMenuAction::kUnmount
             << ContextMenuAction::kRename
             << ContextMenuAction::kFormat
             << ContextMenuAction::kEject
             << ContextMenuAction::kErase
             << ContextMenuAction::kSafelyRemove
             << ContextMenuAction::kLogoutAndForget
             << ContextMenuAction::kProperty
             << ContextMenuAction::kActionTriggeredFromSidebar;
    
    // Check for uniqueness
    QSet<QString> uniqueActions;
    for (const QString &action : actions) {
        EXPECT_FALSE(uniqueActions.contains(action)) << "Duplicate action found: " << action.toStdString();
        uniqueActions.insert(action);
    }
    
    EXPECT_EQ(uniqueActions.size(), actions.size());
}

TEST_F(UT_ComputerDataStruct, ContextMenuAction_TranslationFunctions_HandleMultipleCalls_Success)
{
    // Test that translation functions can be called multiple times without issues
    for (int i = 0; i < 3; ++i) {
        QString openText = ContextMenuAction::trOpen();
        QString mountText = ContextMenuAction::trMount();
        QString ejectText = ContextMenuAction::trEject();
        
        EXPECT_FALSE(openText.isEmpty());
        EXPECT_FALSE(mountText.isEmpty());
        EXPECT_FALSE(ejectText.isEmpty());
    }
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_MutableFields_CanBeModified)
{
    ComputerItemData data;
    
    // Test mutable fields
    data.itemName = "Initial Name";
    EXPECT_EQ(data.itemName, "Initial Name");
    
    data.itemName = "Modified Name";
    EXPECT_EQ(data.itemName, "Modified Name");
    
    data.isEditing = false;
    EXPECT_FALSE(data.isEditing);
    
    data.isEditing = true;
    EXPECT_TRUE(data.isEditing);
    
    data.isElided = false;
    EXPECT_FALSE(data.isElided);
    
    data.isElided = true;
    EXPECT_TRUE(data.isElided);
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_PointerFields_CanBeNull)
{
    ComputerItemData data;
    
    // Test that pointer fields can be null
    EXPECT_EQ(data.widget, nullptr);
    EXPECT_EQ(data.info, nullptr);
    
    // Test that pointer fields can be set (without actually creating objects)
    QWidget *testWidget = reinterpret_cast<QWidget *>(0x1234);
    DFMEntryFileInfoPointer testInfo = DFMEntryFileInfoPointer(nullptr);
    
    data.widget = testWidget;
    data.info = testInfo;
    
    EXPECT_EQ(data.widget, testWidget);
    EXPECT_EQ(data.info, testInfo);
}

TEST_F(UT_ComputerDataStruct, ComputerItemData_UrlOperations_WorkCorrectly)
{
    ComputerItemData data;
    
    // Test URL operations
    QUrl testUrl1("entry://test1.blockdev");
    QUrl testUrl2("entry://test2.userdir");
    
    data.url = testUrl1;
    EXPECT_EQ(data.url, testUrl1);
    EXPECT_NE(data.url, testUrl2);
    
    data.url = testUrl2;
    EXPECT_EQ(data.url, testUrl2);
    EXPECT_NE(data.url, testUrl1);
    
    // Test URL scheme
    EXPECT_EQ(data.url.scheme(), QString("entry"));
    
    // Test URL path
    EXPECT_TRUE(data.url.path().endsWith(".userdir"));
}
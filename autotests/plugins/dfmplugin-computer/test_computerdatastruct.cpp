// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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
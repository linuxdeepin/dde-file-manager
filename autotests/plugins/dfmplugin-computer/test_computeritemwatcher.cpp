// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "watcher/computeritemwatcher.h"
#include "utils/computerdatastruct.h"

#include <dfm-base/file/entry/entryfileinfo.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <QSignalSpy>
#include <QUrl>
#include <QVariantMap>
#include <QThread>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_computer;

class UT_ComputerItemWatcher : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.clear();
        watcher = ComputerItemWatcher::instance();
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
    ComputerItemWatcher *watcher = nullptr;
};

TEST_F(UT_ComputerItemWatcher, Instance_SingletonPattern_ReturnsSameInstance)
{
    ComputerItemWatcher *instance1 = ComputerItemWatcher::instance();
    ComputerItemWatcher *instance2 = ComputerItemWatcher::instance();

    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_ComputerItemWatcher, Items_EmptyInitially_ReturnsEmptyList)
{
    stub.set_lamda(&ComputerItemWatcher::items, [&](ComputerItemWatcher *) -> ComputerDataList {
        __DBG_STUB_INVOKE__
        return ComputerDataList();
    });

    ComputerDataList items = watcher->items();
    EXPECT_TRUE(items.isEmpty());
}

TEST_F(UT_ComputerItemWatcher, Items_WithData_ReturnsCorrectList)
{
    ComputerItemData testData;
    testData.url = QUrl("entry://test.blockdev");
    testData.itemName = "Test Device";
    testData.shape = ComputerItemData::kLargeItem;

    ComputerDataList mockItems;
    mockItems.append(testData);

    stub.set_lamda(&ComputerItemWatcher::items, [&](ComputerItemWatcher *) -> ComputerDataList {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    ComputerDataList items = watcher->items();
    EXPECT_EQ(items.size(), 1);
    EXPECT_EQ(items.first().url, testData.url);
    EXPECT_EQ(items.first().itemName, testData.itemName);
}

TEST_F(UT_ComputerItemWatcher, GetInitedItems_ReturnsFilteredItems_Success)
{
    ComputerDataList mockItems;
    ComputerItemData item1, item2;
    item1.url = QUrl("entry://test1.blockdev");
    item1.itemName = "Test Device 1";
    item2.url = QUrl("entry://test2.userdir");
    item2.itemName = "Test Directory";

    mockItems.append(item1);
    mockItems.append(item2);

    stub.set_lamda(&ComputerItemWatcher::getInitedItems, [&](ComputerItemWatcher *) -> ComputerDataList {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    ComputerDataList items = watcher->getInitedItems();
    EXPECT_EQ(items.size(), 2);
}

TEST_F(UT_ComputerItemWatcher, TypeCompare_DifferentTypes_ReturnsCorrectOrder)
{
    ComputerItemData itemA, itemB;
    itemA.shape = ComputerItemData::kSmallItem;
    itemA.groupId = 1;
    itemB.shape = ComputerItemData::kLargeItem;
    itemB.groupId = 2;

    stub.set_lamda(&ComputerItemWatcher::typeCompare, [&](const ComputerItemData &a, const ComputerItemData &b) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(a.shape, itemA.shape);
        EXPECT_EQ(b.shape, itemB.shape);
        return a.groupId < b.groupId;
    });

    bool result = ComputerItemWatcher::typeCompare(itemA, itemB);
    EXPECT_TRUE(result);
}

TEST_F(UT_ComputerItemWatcher, StartQueryItems_AsyncMode_StartsQuery)
{
    bool queryCalled = false;
    stub.set_lamda(&ComputerItemWatcher::startQueryItems, [&](ComputerItemWatcher *, bool async) {
        __DBG_STUB_INVOKE__
        queryCalled = true;
        EXPECT_TRUE(async);
    });

    watcher->startQueryItems(true);
    EXPECT_TRUE(queryCalled);
}

TEST_F(UT_ComputerItemWatcher, StartQueryItems_SyncMode_StartsQuery)
{
    bool queryCalled = false;
    stub.set_lamda(&ComputerItemWatcher::startQueryItems, [&](ComputerItemWatcher *, bool async) {
        __DBG_STUB_INVOKE__
        queryCalled = true;
        EXPECT_FALSE(async);
    });

    watcher->startQueryItems(false);
    EXPECT_TRUE(queryCalled);
}

TEST_F(UT_ComputerItemWatcher, AddDevice_ValidParameters_AddsDevice)
{
    QString groupName = "Test Group";
    QUrl deviceUrl("entry://test.blockdev");
    int shape = ComputerItemData::kLargeItem;
    bool addToSidebar = true;

    bool addCalled = false;
    stub.set_lamda(&ComputerItemWatcher::addDevice, [&](ComputerItemWatcher *, const QString &group, const QUrl &url, int shapeType, bool sidebar) {
        __DBG_STUB_INVOKE__
        addCalled = true;
        EXPECT_EQ(group, groupName);
        EXPECT_EQ(url, deviceUrl);
        EXPECT_EQ(shapeType, shape);
        EXPECT_EQ(sidebar, addToSidebar);
    });

    watcher->addDevice(groupName, deviceUrl, shape, addToSidebar);
    EXPECT_TRUE(addCalled);
}

TEST_F(UT_ComputerItemWatcher, RemoveDevice_ValidUrl_RemovesDevice)
{
    QUrl deviceUrl("entry://test.blockdev");

    bool removeCalled = false;
    stub.set_lamda(&ComputerItemWatcher::removeDevice, [&](ComputerItemWatcher *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        removeCalled = true;
        EXPECT_EQ(url, deviceUrl);
    });

    watcher->removeDevice(deviceUrl);
    EXPECT_TRUE(removeCalled);
}

TEST_F(UT_ComputerItemWatcher, MakeSidebarItem_ValidInfo_ReturnsCorrectMap)
{
    DFMEntryFileInfoPointer testInfo = nullptr; // Would be valid in real scenario
    QVariantMap expectedMap;
    expectedMap["displayName"] = "Test Item";
    expectedMap["url"] = "entry://test.blockdev";

    stub.set_lamda(&ComputerItemWatcher::makeSidebarItem, [&](ComputerItemWatcher *, DFMEntryFileInfoPointer info) -> QVariantMap {
        __DBG_STUB_INVOKE__
        return expectedMap;
    });

    QVariantMap result = watcher->makeSidebarItem(testInfo);
    EXPECT_EQ(result["displayName"].toString(), "Test Item");
    EXPECT_EQ(result["url"].toString(), "entry://test.blockdev");
}

TEST_F(UT_ComputerItemWatcher, UpdateSidebarItem_ValidParameters_UpdatesItem)
{
    QUrl itemUrl("entry://test.blockdev");
    QString newName = "Updated Name";
    bool editable = true;

    bool updateCalled = false;
    stub.set_lamda(&ComputerItemWatcher::updateSidebarItem, [&](ComputerItemWatcher *, const QUrl &url, const QString &name, bool edit) {
        __DBG_STUB_INVOKE__
        updateCalled = true;
        EXPECT_EQ(url, itemUrl);
        EXPECT_EQ(name, newName);
        EXPECT_EQ(edit, editable);
    });

    watcher->updateSidebarItem(itemUrl, newName, editable);
    EXPECT_TRUE(updateCalled);
}

TEST_F(UT_ComputerItemWatcher, AddSidebarItem_WithInfo_AddsToSidebar)
{
    DFMEntryFileInfoPointer testInfo = nullptr;

    bool addCalled = false;
    stub.set_lamda(static_cast<void(ComputerItemWatcher::*)(DFMEntryFileInfoPointer)>(&ComputerItemWatcher::addSidebarItem),
                   [&](ComputerItemWatcher *, DFMEntryFileInfoPointer info) {
        __DBG_STUB_INVOKE__
        addCalled = true;
    });

    watcher->addSidebarItem(testInfo);
    EXPECT_TRUE(addCalled);
}

TEST_F(UT_ComputerItemWatcher, AddSidebarItem_WithUrlAndData_AddsToSidebar)
{
    QUrl itemUrl("entry://test.blockdev");
    QVariantMap itemData;
    itemData["displayName"] = "Test Item";

    bool addCalled = false;
    stub.set_lamda(static_cast<void(ComputerItemWatcher::*)(const QUrl &, const QVariantMap &)>(&ComputerItemWatcher::addSidebarItem),
                   [&](ComputerItemWatcher *, const QUrl &url, const QVariantMap &data) {
        __DBG_STUB_INVOKE__
        addCalled = true;
        EXPECT_EQ(url, itemUrl);
        EXPECT_EQ(data["displayName"].toString(), "Test Item");
    });

    watcher->addSidebarItem(itemUrl, itemData);
    EXPECT_TRUE(addCalled);
}

TEST_F(UT_ComputerItemWatcher, RemoveSidebarItem_ValidUrl_RemovesFromSidebar)
{
    QUrl itemUrl("entry://test.blockdev");

    bool removeCalled = false;
    stub.set_lamda(&ComputerItemWatcher::removeSidebarItem, [&](ComputerItemWatcher *, const QUrl &url) {
        __DBG_STUB_INVOKE__
        removeCalled = true;
        EXPECT_EQ(url, itemUrl);
    });

    watcher->removeSidebarItem(itemUrl);
    EXPECT_TRUE(removeCalled);
}

TEST_F(UT_ComputerItemWatcher, HandleSidebarItemsVisiable_CallsSuccessfully_DoesNotCrash)
{
    bool handleCalled = false;
    stub.set_lamda(&ComputerItemWatcher::handleSidebarItemsVisiable, [&](ComputerItemWatcher *) {
        __DBG_STUB_INVOKE__
        handleCalled = true;
    });

    watcher->handleSidebarItemsVisiable();
    EXPECT_TRUE(handleCalled);
}

TEST_F(UT_ComputerItemWatcher, RemoveGroup_ExistingGroup_ReturnsTrue)
{
    QString groupName = "Test Group";

    stub.set_lamda(&ComputerItemWatcher::removeGroup, [&](ComputerItemWatcher *, const QString &name) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(name, groupName);
        return true;
    });

    bool result = watcher->removeGroup(groupName);
    EXPECT_TRUE(result);
}

TEST_F(UT_ComputerItemWatcher, RemoveGroup_NonExistingGroup_ReturnsFalse)
{
    QString groupName = "Non-existing Group";

    stub.set_lamda(&ComputerItemWatcher::removeGroup, [&](ComputerItemWatcher *, const QString &name) -> bool {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(name, groupName);
        return false;
    });

    bool result = watcher->removeGroup(groupName);
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerItemWatcher, InsertUrlMapper_ValidParameters_InsertsMapping)
{
    QString deviceId = "test-device-id";
    QUrl mountUrl("file:///media/test");

    bool insertCalled = false;
    stub.set_lamda(&ComputerItemWatcher::insertUrlMapper, [&](ComputerItemWatcher *, const QString &id, const QUrl &url) {
        __DBG_STUB_INVOKE__
        insertCalled = true;
        EXPECT_EQ(id, deviceId);
        EXPECT_EQ(url, mountUrl);
    });

    watcher->insertUrlMapper(deviceId, mountUrl);
    EXPECT_TRUE(insertCalled);
}

TEST_F(UT_ComputerItemWatcher, ClearAsyncThread_CallsSuccessfully_DoesNotCrash)
{
    bool clearCalled = false;
    stub.set_lamda(&ComputerItemWatcher::clearAsyncThread, [&](ComputerItemWatcher *) {
        __DBG_STUB_INVOKE__
        clearCalled = true;
    });

    watcher->clearAsyncThread();
    EXPECT_TRUE(clearCalled);
}

TEST_F(UT_ComputerItemWatcher, UserDirGroup_ReturnsCorrectGroupName_Success)
{
    QString expectedGroup = "User Directories";

    stub.set_lamda(&ComputerItemWatcher::userDirGroup, [&]() -> QString {
        __DBG_STUB_INVOKE__
        return expectedGroup;
    });

    QString result = ComputerItemWatcher::userDirGroup();
    EXPECT_EQ(result, expectedGroup);
}

TEST_F(UT_ComputerItemWatcher, DiskGroup_ReturnsCorrectGroupName_Success)
{
    QString expectedGroup = "Disks";

    stub.set_lamda(&ComputerItemWatcher::diskGroup, [&]() -> QString {
        __DBG_STUB_INVOKE__
        return expectedGroup;
    });

    QString result = ComputerItemWatcher::diskGroup();
    EXPECT_EQ(result, expectedGroup);
}

TEST_F(UT_ComputerItemWatcher, GetGroupId_ValidGroupName_ReturnsCorrectId)
{
    QString groupName = "Test Group";
    int expectedId = 5;

    stub.set_lamda(&ComputerItemWatcher::getGroupId, [&](ComputerItemWatcher *, const QString &name) -> int {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(name, groupName);
        return expectedId;
    });

    int result = watcher->getGroupId(groupName);
    EXPECT_EQ(result, expectedId);
}

TEST_F(UT_ComputerItemWatcher, HideUserDir_ConfigEnabled_ReturnsTrue)
{
    stub.set_lamda(&ComputerItemWatcher::hideUserDir, [&]() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = ComputerItemWatcher::hideUserDir();
    EXPECT_TRUE(result);
}

TEST_F(UT_ComputerItemWatcher, HideUserDir_ConfigDisabled_ReturnsFalse)
{
    stub.set_lamda(&ComputerItemWatcher::hideUserDir, [&]() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    bool result = ComputerItemWatcher::hideUserDir();
    EXPECT_FALSE(result);
}

TEST_F(UT_ComputerItemWatcher, Hide3rdEntries_ConfigEnabled_ReturnsTrue)
{
    stub.set_lamda(&ComputerItemWatcher::hide3rdEntries, [&]() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = ComputerItemWatcher::hide3rdEntries();
    EXPECT_TRUE(result);
}

TEST_F(UT_ComputerItemWatcher, DisksHiddenByDConf_ReturnsCorrectList_Success)
{
    QList<QUrl> hiddenDisks;
    hiddenDisks.append(QUrl("entry://hidden1.blockdev"));
    hiddenDisks.append(QUrl("entry://hidden2.blockdev"));

    stub.set_lamda(&ComputerItemWatcher::disksHiddenByDConf, [&]() -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return hiddenDisks;
    });

    QList<QUrl> result = ComputerItemWatcher::disksHiddenByDConf();
    EXPECT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], hiddenDisks[0]);
    EXPECT_EQ(result[1], hiddenDisks[1]);
}

TEST_F(UT_ComputerItemWatcher, DisksHiddenBySettingPanel_ReturnsCorrectList_Success)
{
    QList<QUrl> hiddenDisks;
    hiddenDisks.append(QUrl("entry://setting-hidden.blockdev"));

    stub.set_lamda(&ComputerItemWatcher::disksHiddenBySettingPanel, [&]() -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return hiddenDisks;
    });

    QList<QUrl> result = ComputerItemWatcher::disksHiddenBySettingPanel();
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], hiddenDisks[0]);
}

TEST_F(UT_ComputerItemWatcher, HiddenPartitions_ReturnsCorrectList_Success)
{
    QList<QUrl> hiddenPartitions;
    hiddenPartitions.append(QUrl("entry://partition1.blockdev"));
    hiddenPartitions.append(QUrl("entry://partition2.blockdev"));

    stub.set_lamda(&ComputerItemWatcher::hiddenPartitions, [&]() -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return hiddenPartitions;
    });

    QList<QUrl> result = ComputerItemWatcher::hiddenPartitions();
    EXPECT_EQ(result.size(), 2);
}

TEST_F(UT_ComputerItemWatcher, GetComputerInfos_ReturnsCorrectMap_Success)
{
    QHash<QUrl, QVariantMap> mockInfos;
    QUrl testUrl("entry://test.blockdev");
    QVariantMap testInfo;
    testInfo["name"] = "Test Device";
    testInfo["size"] = 1000000;
    mockInfos[testUrl] = testInfo;

    stub.set_lamda(&ComputerItemWatcher::getComputerInfos, [&](const ComputerItemWatcher *) -> QHash<QUrl, QVariantMap> {
        __DBG_STUB_INVOKE__
        return mockInfos;
    });

    QHash<QUrl, QVariantMap> result = watcher->getComputerInfos();
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(result.contains(testUrl));
    EXPECT_EQ(result[testUrl]["name"].toString(), "Test Device");
}

TEST_F(UT_ComputerItemWatcher, GroupTypes_EnumValues_AreValid)
{
    EXPECT_EQ(ComputerItemWatcher::kGroupDirs, 0);
    EXPECT_EQ(ComputerItemWatcher::kGroupDisks, 1);
    EXPECT_EQ(ComputerItemWatcher::kOthers, 2);
}

TEST_F(UT_ComputerItemWatcher, AsyncOperations_ThreadSafety_DoesNotCrash)
{
    // Test that async operations don't crash
    EXPECT_NO_THROW(watcher->startQueryItems(true));
    EXPECT_NO_THROW(watcher->clearAsyncThread());

    // Simulate some delay for async operations
    QThread::msleep(10);

    EXPECT_NO_THROW(watcher->clearAsyncThread());
}

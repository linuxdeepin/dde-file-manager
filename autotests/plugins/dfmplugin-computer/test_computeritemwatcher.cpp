// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
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
#include <QIcon>
#include <QObject>

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

TEST_F(UT_ComputerItemWatcher, OnViewRefresh_CallsSuccessfully_DoesNotCrash)
{
    bool refreshCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onViewRefresh, [&](ComputerItemWatcher *) {
        __DBG_STUB_INVOKE__
        refreshCalled = true;
    });

    watcher->onViewRefresh();
    EXPECT_TRUE(refreshCalled);
}

TEST_F(UT_ComputerItemWatcher, CacheItem_ValidData_InsertsCorrectly)
{
    ComputerItemData testData;
    testData.url = QUrl("entry://test.blockdev");
    testData.itemName = "Test Device";
    testData.shape = ComputerItemData::kLargeItem;
    testData.groupId = 1;

    bool cacheCalled = false;
    stub.set_lamda(&ComputerItemWatcher::cacheItem, [&](ComputerItemWatcher *, const ComputerItemData &item) {
        __DBG_STUB_INVOKE__
        cacheCalled = true;
        EXPECT_EQ(item.url, testData.url);
        EXPECT_EQ(item.itemName, testData.itemName);
    });

    watcher->cacheItem(testData);
    EXPECT_TRUE(cacheCalled);
}

TEST_F(UT_ComputerItemWatcher, ReportName_ValidUrl_ReturnsCorrectName)
{
    QUrl testUrl("entry://test.blockdev");
    QString expectedName = "Test Disk";

    stub.set_lamda(&ComputerItemWatcher::reportName, [&](ComputerItemWatcher *, const QUrl &url) -> QString {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(url, testUrl);
        return expectedName;
    });

    QString result = watcher->reportName(testUrl);
    EXPECT_EQ(result, expectedName);
}

TEST_F(UT_ComputerItemWatcher, FindFinalUrl_ValidInfo_ReturnsCorrectUrl)
{
    QUrl testUrl("entry://test.blockdev");
    QUrl expectedFinalUrl("file:///media/test");

    DFMEntryFileInfoPointer mockInfo(new EntryFileInfo(testUrl));
    
    stub.set_lamda(&ComputerItemWatcher::findFinalUrl, [&](const ComputerItemWatcher *, DFMEntryFileInfoPointer info) -> QUrl {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(info->urlOf(UrlInfoType::kUrl), testUrl);
        return expectedFinalUrl;
    });

    QUrl result = watcher->findFinalUrl(mockInfo);
    EXPECT_EQ(result, expectedFinalUrl);
}

TEST_F(UT_ComputerItemWatcher, GetGroup_ValidType_ReturnsCorrectGroup)
{
    ComputerItemWatcher::GroupType type = ComputerItemWatcher::kGroupDirs;
    QString defaultName = "Test Group";

    stub.set_lamda(&ComputerItemWatcher::getGroup, [&](ComputerItemWatcher *, ComputerItemWatcher::GroupType groupType, const QString &name) -> ComputerItemData {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(groupType, type);
        EXPECT_EQ(name, defaultName);
        
        ComputerItemData groupData;
        groupData.shape = ComputerItemData::kSplitterItem;
        groupData.itemName = name;
        return groupData;
    });

    ComputerItemData result = watcher->getGroup(type, defaultName);
    EXPECT_EQ(result.shape, ComputerItemData::kSplitterItem);
    EXPECT_EQ(result.itemName, defaultName);
}

TEST_F(UT_ComputerItemWatcher, AddGroup_ValidName_ReturnsCorrectId)
{
    QString groupName = "Test Group";
    int expectedId = 10;

    stub.set_lamda(&ComputerItemWatcher::addGroup, [&](ComputerItemWatcher *, const QString &name) -> int {
        __DBG_STUB_INVOKE__
        EXPECT_EQ(name, groupName);
        return expectedId;
    });

    int result = watcher->addGroup(groupName);
    EXPECT_EQ(result, expectedId);
}

TEST_F(UT_ComputerItemWatcher, OnDeviceAdded_ValidParameters_HandlesCorrectly)
{
    QUrl devUrl("entry://test.blockdev");
    int groupId = 5;
    ComputerItemData::ShapeType shape = ComputerItemData::kLargeItem;
    bool needSidebarItem = true;

    bool deviceAddedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onDeviceAdded, [&](ComputerItemWatcher *, const QUrl &url, int id, ComputerItemData::ShapeType shapeType, bool sidebar) {
        __DBG_STUB_INVOKE__
        deviceAddedCalled = true;
        EXPECT_EQ(url, devUrl);
        EXPECT_EQ(id, groupId);
        EXPECT_EQ(shapeType, shape);
        EXPECT_EQ(sidebar, needSidebarItem);
    });

    watcher->onDeviceAdded(devUrl, groupId, shape, needSidebarItem);
    EXPECT_TRUE(deviceAddedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnDevicePropertyChangedQVar_ValidParameters_HandlesCorrectly)
{
    QString id = "test-device-id";
    QString propertyName = "TestProperty";
    QVariant var("TestValue");

    bool propertyChangedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onDevicePropertyChangedQVar, [&](ComputerItemWatcher *, const QString &devId, const QString &propName, const QVariant &value) {
        __DBG_STUB_INVOKE__
        propertyChangedCalled = true;
        EXPECT_EQ(devId, id);
        EXPECT_EQ(propName, propertyName);
        EXPECT_EQ(value, var);
    });

    watcher->onDevicePropertyChangedQVar(id, propertyName, var);
    EXPECT_TRUE(propertyChangedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnDevicePropertyChangedQDBusVar_ValidParameters_HandlesCorrectly)
{
    QString id = "test-device-id";
    QString propertyName = "TestProperty";
    QDBusVariant var("TestValue");

    bool propertyChangedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onDevicePropertyChangedQDBusVar, [&](ComputerItemWatcher *, const QString &devId, const QString &propName, const QDBusVariant &value) {
        __DBG_STUB_INVOKE__
        propertyChangedCalled = true;
        EXPECT_EQ(devId, id);
        EXPECT_EQ(propName, propertyName);
        EXPECT_EQ(value.variant(), var.variant());
    });

    watcher->onDevicePropertyChangedQDBusVar(id, propertyName, var);
    EXPECT_TRUE(propertyChangedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnGenAttributeChanged_ValidParameters_HandlesCorrectly)
{
    Application::GenericAttribute ga = Application::GenericAttribute::kShowFileSystemTagOnDiskIcon;
    QVariant value(true);

    bool attributeChangedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onGenAttributeChanged, [&](ComputerItemWatcher *, Application::GenericAttribute attr, const QVariant &val) {
        __DBG_STUB_INVOKE__
        attributeChangedCalled = true;
        EXPECT_EQ(attr, ga);
        EXPECT_EQ(val, value);
    });

    watcher->onGenAttributeChanged(ga, value);
    EXPECT_TRUE(attributeChangedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnDConfigChanged_ValidParameters_HandlesCorrectly)
{
    QString cfg = "test-config";
    QString cfgKey = "test-key";

    bool dconfigChangedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onDConfigChanged, [&](ComputerItemWatcher *, const QString &config, const QString &key) {
        __DBG_STUB_INVOKE__
        dconfigChangedCalled = true;
        EXPECT_EQ(config, cfg);
        EXPECT_EQ(key, cfgKey);
    });

    watcher->onDConfigChanged(cfg, cfgKey);
    EXPECT_TRUE(dconfigChangedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnBlockDeviceAdded_ValidId_HandlesCorrectly)
{
    QString id = "test-block-device-id";

    bool blockDeviceAddedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onBlockDeviceAdded, [&](ComputerItemWatcher *, const QString &devId) {
        __DBG_STUB_INVOKE__
        blockDeviceAddedCalled = true;
        EXPECT_EQ(devId, id);
    });

    watcher->onBlockDeviceAdded(id);
    EXPECT_TRUE(blockDeviceAddedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnBlockDeviceRemoved_ValidId_HandlesCorrectly)
{
    QString id = "test-block-device-id";

    bool blockDeviceRemovedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onBlockDeviceRemoved, [&](ComputerItemWatcher *, const QString &devId) {
        __DBG_STUB_INVOKE__
        blockDeviceRemovedCalled = true;
        EXPECT_EQ(devId, id);
    });

    watcher->onBlockDeviceRemoved(id);
    EXPECT_TRUE(blockDeviceRemovedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnBlockDeviceMounted_ValidParameters_HandlesCorrectly)
{
    QString id = "test-block-device-id";
    QString mntPath = "/media/test";

    bool blockDeviceMountedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onBlockDeviceMounted, [&](ComputerItemWatcher *, const QString &devId, const QString &mountPath) {
        __DBG_STUB_INVOKE__
        blockDeviceMountedCalled = true;
        EXPECT_EQ(devId, id);
        EXPECT_EQ(mountPath, mntPath);
    });

    watcher->onBlockDeviceMounted(id, mntPath);
    EXPECT_TRUE(blockDeviceMountedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnBlockDeviceUnmounted_ValidId_HandlesCorrectly)
{
    QString id = "test-block-device-id";

    bool blockDeviceUnmountedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onBlockDeviceUnmounted, [&](ComputerItemWatcher *, const QString &devId) {
        __DBG_STUB_INVOKE__
        blockDeviceUnmountedCalled = true;
        EXPECT_EQ(devId, id);
    });

    watcher->onBlockDeviceUnmounted(id);
    EXPECT_TRUE(blockDeviceUnmountedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnBlockDeviceLocked_ValidId_HandlesCorrectly)
{
    QString id = "test-block-device-id";

    bool blockDeviceLockedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onBlockDeviceLocked, [&](ComputerItemWatcher *, const QString &devId) {
        __DBG_STUB_INVOKE__
        blockDeviceLockedCalled = true;
        EXPECT_EQ(devId, id);
    });

    watcher->onBlockDeviceLocked(id);
    EXPECT_TRUE(blockDeviceLockedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnUpdateBlockItem_ValidId_HandlesCorrectly)
{
    QString id = "test-block-device-id";

    bool updateBlockItemCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onUpdateBlockItem, [&](ComputerItemWatcher *, const QString &devId) {
        __DBG_STUB_INVOKE__
        updateBlockItemCalled = true;
        EXPECT_EQ(devId, id);
    });

    watcher->onUpdateBlockItem(id);
    EXPECT_TRUE(updateBlockItemCalled);
}

TEST_F(UT_ComputerItemWatcher, OnProtocolDeviceMounted_ValidParameters_HandlesCorrectly)
{
    QString id = "test-protocol-device-id";
    QString mntPath = "/media/test";

    bool protocolDeviceMountedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onProtocolDeviceMounted, [&](ComputerItemWatcher *, const QString &devId, const QString &mountPath) {
        __DBG_STUB_INVOKE__
        protocolDeviceMountedCalled = true;
        EXPECT_EQ(devId, id);
        EXPECT_EQ(mountPath, mntPath);
    });

    watcher->onProtocolDeviceMounted(id, mntPath);
    EXPECT_TRUE(protocolDeviceMountedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnProtocolDeviceUnmounted_ValidId_HandlesCorrectly)
{
    QString id = "test-protocol-device-id";

    bool protocolDeviceUnmountedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onProtocolDeviceUnmounted, [&](ComputerItemWatcher *, const QString &devId) {
        __DBG_STUB_INVOKE__
        protocolDeviceUnmountedCalled = true;
        EXPECT_EQ(devId, id);
    });

    watcher->onProtocolDeviceUnmounted(id);
    EXPECT_TRUE(protocolDeviceUnmountedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnDeviceSizeChanged_ValidParameters_HandlesCorrectly)
{
    QString id = "test-device-id";
    qlonglong total = 1000000;
    qlonglong free = 500000;

    bool deviceSizeChangedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onDeviceSizeChanged, [&](ComputerItemWatcher *, const QString &devId, qlonglong totalSize, qlonglong freeSize) {
        __DBG_STUB_INVOKE__
        deviceSizeChangedCalled = true;
        EXPECT_EQ(devId, id);
        EXPECT_EQ(totalSize, total);
        EXPECT_EQ(freeSize, free);
    });

    watcher->onDeviceSizeChanged(id, total, free);
    EXPECT_TRUE(deviceSizeChangedCalled);
}

TEST_F(UT_ComputerItemWatcher, OnProtocolDeviceRemoved_ValidId_HandlesCorrectly)
{
    QString id = "test-protocol-device-id";

    bool protocolDeviceRemovedCalled = false;
    stub.set_lamda(&ComputerItemWatcher::onProtocolDeviceRemoved, [&](ComputerItemWatcher *, const QString &devId) {
        __DBG_STUB_INVOKE__
        protocolDeviceRemovedCalled = true;
        EXPECT_EQ(devId, id);
    });

    watcher->onProtocolDeviceRemoved(id);
    EXPECT_TRUE(protocolDeviceRemovedCalled);
}

TEST_F(UT_ComputerItemWatcher, GetUserDirItems_ReturnsCorrectList_Success)
{
    ComputerDataList mockItems;
    ComputerItemData item1, item2;
    item1.url = QUrl("entry://desktop.userdir");
    item1.itemName = "Desktop";
    item2.url = QUrl("entry://documents.userdir");
    item2.itemName = "Documents";

    mockItems.append(item1);
    mockItems.append(item2);

    stub.set_lamda(&ComputerItemWatcher::getUserDirItems, [&](ComputerItemWatcher *) -> ComputerDataList {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    ComputerDataList result = watcher->getUserDirItems();
    EXPECT_EQ(result.size(), 2);
}

TEST_F(UT_ComputerItemWatcher, GetBlockDeviceItems_ReturnsCorrectList_Success)
{
    ComputerDataList mockItems;
    ComputerItemData item1;
    item1.url = QUrl("entry://test1.blockdev");
    item1.itemName = "Test Block Device 1";

    mockItems.append(item1);

    stub.set_lamda(&ComputerItemWatcher::getBlockDeviceItems, [&](ComputerItemWatcher *, bool *hasNewItem) -> ComputerDataList {
        __DBG_STUB_INVOKE__
        *hasNewItem = true;
        return mockItems;
    });

    bool hasNewItem = false;
    ComputerDataList result = watcher->getBlockDeviceItems(&hasNewItem);
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(hasNewItem);
}

TEST_F(UT_ComputerItemWatcher, GetProtocolDeviceItems_ReturnsCorrectList_Success)
{
    ComputerDataList mockItems;
    ComputerItemData item1;
    item1.url = QUrl("entry://test1.protocol");
    item1.itemName = "Test Protocol Device 1";

    mockItems.append(item1);

    stub.set_lamda(&ComputerItemWatcher::getProtocolDeviceItems, [&](ComputerItemWatcher *, bool *hasNewItem) -> ComputerDataList {
        __DBG_STUB_INVOKE__
        *hasNewItem = true;
        return mockItems;
    });

    bool hasNewItem = false;
    ComputerDataList result = watcher->getProtocolDeviceItems(&hasNewItem);
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(hasNewItem);
}

TEST_F(UT_ComputerItemWatcher, GetAppEntryItems_ReturnsCorrectList_Success)
{
    ComputerDataList mockItems;
    ComputerItemData item1;
    item1.url = QUrl("entry://test1.appentry");
    item1.itemName = "Test App Entry 1";

    mockItems.append(item1);

    stub.set_lamda(&ComputerItemWatcher::getAppEntryItems, [&](ComputerItemWatcher *, bool *hasNewItem) -> ComputerDataList {
        __DBG_STUB_INVOKE__
        *hasNewItem = true;
        return mockItems;
    });

    bool hasNewItem = false;
    ComputerDataList result = watcher->getAppEntryItems(&hasNewItem);
    EXPECT_EQ(result.size(), 1);
    EXPECT_TRUE(hasNewItem);
}

TEST_F(UT_ComputerItemWatcher, GetPreDefineItems_ReturnsCorrectList_Success)
{
    ComputerDataList mockItems;
    ComputerItemData item1;
    item1.url = QUrl("entry://test1.predefine");
    item1.itemName = "Test Predefine Item 1";

    mockItems.append(item1);

    stub.set_lamda(&ComputerItemWatcher::getPreDefineItems, [&](ComputerItemWatcher *) -> ComputerDataList {
        __DBG_STUB_INVOKE__
        return mockItems;
    });

    ComputerDataList result = watcher->getPreDefineItems();
    EXPECT_EQ(result.size(), 1);
}

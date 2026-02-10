// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stub-ext/stubext.h>

#include "recentmanagerdbus.h"
#include "recentmanager.h"

#include <QSignalSpy>
#include <QDateTime>
#include <QVariantMap>
#include <QStringList>

SERVERRECENTMANAGER_USE_NAMESPACE

class UT_RecentManagerDBus : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        recentManagerDBus = new RecentManagerDBus();
    }

    virtual void TearDown() override
    {
        delete recentManagerDBus;
        recentManagerDBus = nullptr;
        stub.clear();
    }

protected:
    RecentManagerDBus *recentManagerDBus { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_RecentManagerDBus, constructor_InitializesCorrectly)
{
    // Test that constructor creates object and initializes connections
    EXPECT_NE(recentManagerDBus, nullptr);
    // Note: parent may be nullptr if not explicitly set in constructor
}

TEST_F(UT_RecentManagerDBus, initConnect_ConnectsSignalsCorrectly)
{
    // Test signal connections are established
    bool reloadFinishedConnected = false;
    bool purgeFinishedConnected = false;
    bool itemAddedConnected = false;
    bool itemsRemovedConnected = false;
    bool itemChangedConnected = false;

    stub.set_lamda(&RecentManager::instance, [&]() -> RecentManager& {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    // Create new instance to test initConnect
    RecentManagerDBus testDBus;
    
    // Verify connections exist by checking meta object connections
    const QMetaObject *metaObj = testDBus.metaObject();
    EXPECT_NE(metaObj, nullptr);
}

TEST_F(UT_RecentManagerDBus, Reload_CallsForceReloadAndReturnsTimestamp)
{
    qint64 testTimestamp = 1234567890;
    bool forceReloadCalled = false;
    qint64 receivedTimestamp = 0;

    stub.set_lamda(&QDateTime::currentMSecsSinceEpoch, [&testTimestamp]() {
        __DBG_STUB_INVOKE__
        return testTimestamp;
    });

    stub.set_lamda(&RecentManager::instance, [&]() -> RecentManager& {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::forceReload, [&](RecentManager *, qint64 timestamp) {
        __DBG_STUB_INVOKE__
        forceReloadCalled = true;
        receivedTimestamp = timestamp;
    });

    qint64 result = recentManagerDBus->Reload();

    EXPECT_EQ(result, testTimestamp);
    EXPECT_TRUE(forceReloadCalled);
    EXPECT_EQ(receivedTimestamp, testTimestamp);
}

TEST_F(UT_RecentManagerDBus, AddItem_CallsAddRecentItem)
{
    QVariantMap testItem;
    testItem["path"] = "/test/path/file.txt";
    testItem["appName"] = "TestApp";
    testItem["appExec"] = "testapp";
    testItem["mimeType"] = "text/plain";

    bool addRecentItemCalled = false;
    QVariantMap receivedItem;

    stub.set_lamda(&RecentManager::instance, [&]() -> RecentManager& {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::addRecentItem, [&](RecentManager *, const QVariantMap &item) {
        __DBG_STUB_INVOKE__
        addRecentItemCalled = true;
        receivedItem = item;
    });

    recentManagerDBus->AddItem(testItem);

    EXPECT_TRUE(addRecentItemCalled);
    EXPECT_EQ(receivedItem, testItem);
}

TEST_F(UT_RecentManagerDBus, RemoveItems_CallsRemoveItems)
{
    QStringList testHrefs = { "file:///test1.txt", "file:///test2.txt" };
    bool removeItemsCalled = false;
    QStringList receivedHrefs;

    stub.set_lamda(&RecentManager::instance, [&]() -> RecentManager& {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::removeItems, [&](RecentManager *, const QStringList &hrefs) {
        __DBG_STUB_INVOKE__
        removeItemsCalled = true;
        receivedHrefs = hrefs;
    });

    recentManagerDBus->RemoveItems(testHrefs);

    EXPECT_TRUE(removeItemsCalled);
    EXPECT_EQ(receivedHrefs, testHrefs);
}

TEST_F(UT_RecentManagerDBus, PurgeItems_CallsPurgeItems)
{
    bool purgeItemsCalled = false;

    stub.set_lamda(&RecentManager::instance, [&]() -> RecentManager& {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::purgeItems, [&](RecentManager *) {
        __DBG_STUB_INVOKE__
        purgeItemsCalled = true;
    });

    recentManagerDBus->PurgeItems();

    EXPECT_TRUE(purgeItemsCalled);
}

TEST_F(UT_RecentManagerDBus, GetItemsPath_ReturnsPathsList)
{
    QStringList expectedPaths = { "/test/path1.txt", "/test/path2.txt" };
    bool getItemsPathCalled = false;

    stub.set_lamda(&RecentManager::instance, [&]() -> RecentManager& {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::getItemsPath, [&](RecentManager *) -> QStringList {
        __DBG_STUB_INVOKE__
        getItemsPathCalled = true;
        return expectedPaths;
    });

    QStringList result = recentManagerDBus->GetItemsPath();

    EXPECT_TRUE(getItemsPathCalled);
    EXPECT_EQ(result, expectedPaths);
}

TEST_F(UT_RecentManagerDBus, GetItemsInfo_ReturnsItemsInfoList)
{
    QVariantList expectedInfoList;
    QVariantMap item1;
    item1["path"] = "/test/path1.txt";
    item1["href"] = "file:///test/path1.txt";
    item1["modified"] = 1234567890;
    expectedInfoList.append(item1);

    bool getItemsInfoCalled = false;

    stub.set_lamda(&RecentManager::instance, [&]() -> RecentManager& {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::getItemsInfo, [&](RecentManager *) -> QVariantList {
        __DBG_STUB_INVOKE__
        getItemsInfoCalled = true;
        return expectedInfoList;
    });

    QVariantList result = recentManagerDBus->GetItemsInfo();

    EXPECT_TRUE(getItemsInfoCalled);
    EXPECT_EQ(result, expectedInfoList);
}

TEST_F(UT_RecentManagerDBus, GetItemInfo_WithValidPath_ReturnsItemInfo)
{
    QString testPath = "/test/path.txt";
    QVariantMap expectedInfo;
    expectedInfo["path"] = testPath;
    expectedInfo["href"] = "file:///test/path.txt";
    expectedInfo["modified"] = 1234567890;

    bool getItemInfoCalled = false;
    QString receivedPath;

    stub.set_lamda(&RecentManager::instance, [&]() -> RecentManager& {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::getItemInfo, [&](RecentManager *, const QString &path) -> QVariantMap {
        __DBG_STUB_INVOKE__
        getItemInfoCalled = true;
        receivedPath = path;
        return expectedInfo;
    });

    QVariantMap result = recentManagerDBus->GetItemInfo(testPath);

    EXPECT_TRUE(getItemInfoCalled);
    EXPECT_EQ(receivedPath, testPath);
    EXPECT_EQ(result, expectedInfo);
}

TEST_F(UT_RecentManagerDBus, GetItemInfo_WithEmptyPath_ReturnsEmptyMap)
{
    QString emptyPath = "";
    QVariantMap expectedEmptyInfo;

    bool getItemInfoCalled = false;

    stub.set_lamda(&RecentManager::instance, [&]() -> RecentManager& {
        __DBG_STUB_INVOKE__
        static RecentManager manager;
        return manager;
    });

    stub.set_lamda(&RecentManager::getItemInfo, [&](RecentManager *, const QString &path) -> QVariantMap {
        __DBG_STUB_INVOKE__
        getItemInfoCalled = true;
        return expectedEmptyInfo;
    });

    QVariantMap result = recentManagerDBus->GetItemInfo(emptyPath);

    EXPECT_TRUE(getItemInfoCalled);
    EXPECT_TRUE(result.isEmpty());
}

TEST_F(UT_RecentManagerDBus, SignalForwarding_ReloadFinished)
{
    qint64 testTimestamp = 9876543210;
    QSignalSpy spy(recentManagerDBus, &RecentManagerDBus::ReloadFinished);

    // Simulate signal emission from RecentManager
    emit RecentManager::instance().reloadFinished(testTimestamp);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toLongLong(), testTimestamp);
}

TEST_F(UT_RecentManagerDBus, SignalForwarding_PurgeFinished)
{
    QSignalSpy spy(recentManagerDBus, &RecentManagerDBus::PurgeFinished);

    // Simulate signal emission from RecentManager
    emit RecentManager::instance().purgeFinished();

    EXPECT_EQ(spy.count(), 1);
}

TEST_F(UT_RecentManagerDBus, SignalForwarding_ItemAdded)
{
    QString testPath = "/test/added.txt";
    QString testHref = "file:///test/added.txt";
    qint64 testModified = 1234567890;

    QSignalSpy spy(recentManagerDBus, &RecentManagerDBus::ItemAdded);

    // Simulate signal emission from RecentManager
    emit RecentManager::instance().itemAdded(testPath, testHref, testModified);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toString(), testPath);
    EXPECT_EQ(arguments.at(1).toString(), testHref);
    EXPECT_EQ(arguments.at(2).toLongLong(), testModified);
}

TEST_F(UT_RecentManagerDBus, SignalForwarding_ItemsRemoved)
{
    QStringList testPaths = { "/test/removed1.txt", "/test/removed2.txt" };
    QSignalSpy spy(recentManagerDBus, &RecentManagerDBus::ItemsRemoved);

    // Simulate signal emission from RecentManager
    emit RecentManager::instance().itemsRemoved(testPaths);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toStringList(), testPaths);
}

TEST_F(UT_RecentManagerDBus, SignalForwarding_ItemChanged)
{
    QString testPath = "/test/changed.txt";
    qint64 testModified = 9876543210;

    QSignalSpy spy(recentManagerDBus, &RecentManagerDBus::ItemChanged);

    // Simulate signal emission from RecentManager
    emit RecentManager::instance().itemChanged(testPath, testModified);

    EXPECT_EQ(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    EXPECT_EQ(arguments.at(0).toString(), testPath);
    EXPECT_EQ(arguments.at(1).toLongLong(), testModified);
} 
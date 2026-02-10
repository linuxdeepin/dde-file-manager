// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "utils/sidebarmanager.h"
#include "utils/sidebarhelper.h"
#include "treeviews/sidebaritem.h"
#include "dfmplugin_sidebar_global.h"

#include <QUrl>
#include <QPoint>

using namespace dfmplugin_sidebar;

class UT_SideBarManager : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        manager = SideBarManager::instance();
        ASSERT_NE(manager, nullptr);
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    SideBarManager *manager { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarManager, Instance)
{
    auto ins1 = SideBarManager::instance();
    auto ins2 = SideBarManager::instance();

    EXPECT_NE(ins1, nullptr);
    EXPECT_EQ(ins1, ins2);
}

TEST_F(UT_SideBarManager, RunCd_NullItem)
{
    // Should not crash with null item
    manager->runCd(nullptr, 123);

    // Test passes if no crash occurs
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarManager, RunCd_WithClickedCallback)
{
    bool callbackInvoked = false;
    quint64 capturedWindowId = 0;
    QUrl capturedUrl;

    QUrl testUrl = QUrl::fromLocalFile("/home/test");
    SideBarItem *item = new SideBarItem(testUrl);

    // Mock item->url()
    stub.set_lamda(&SideBarItem::url, [testUrl]() {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    // Mock item->itemInfo() to return ItemInfo with callback
    stub.set_lamda(&SideBarItem::itemInfo, [&callbackInvoked, &capturedWindowId, &capturedUrl]() {
        __DBG_STUB_INVOKE__
        ItemInfo info;
        info.clickedCb = [&callbackInvoked, &capturedWindowId, &capturedUrl](quint64 winId, const QUrl &url) {
            callbackInvoked = true;
            capturedWindowId = winId;
            capturedUrl = url;
        };
        return info;
    });

    manager->runCd(item, 12345);

    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(capturedWindowId, 12345u);
    EXPECT_EQ(capturedUrl, testUrl);

    delete item;
}

TEST_F(UT_SideBarManager, RunCd_WithoutCallback_UseDefault)
{
    bool defaultActionCalled = false;
    quint64 capturedWindowId = 0;
    QUrl capturedUrl;

    QUrl testUrl = QUrl::fromLocalFile("/home/default");
    SideBarItem *item = new SideBarItem(testUrl);

    stub.set_lamda(&SideBarItem::url, [testUrl]() {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    stub.set_lamda(&SideBarItem::itemInfo, []() {
        __DBG_STUB_INVOKE__
        ItemInfo info;
        info.clickedCb = nullptr;
        return info;
    });

    stub.set_lamda(&SideBarHelper::defaultCdAction,
        [&defaultActionCalled, &capturedWindowId, &capturedUrl](quint64 winId, const QUrl &url) {
        __DBG_STUB_INVOKE__
        defaultActionCalled = true;
        capturedWindowId = winId;
        capturedUrl = url;
    });

    manager->runCd(item, 54321);

    EXPECT_TRUE(defaultActionCalled);
    EXPECT_EQ(capturedWindowId, 54321u);
    EXPECT_EQ(capturedUrl, testUrl);

    delete item;
}

TEST_F(UT_SideBarManager, RunContextMenu_ContextMenuDisabled)
{
    SideBarHelper::contextMenuEnabled = false;

    QUrl testUrl = QUrl::fromLocalFile("/home/test");
    SideBarItem *item = new SideBarItem(testUrl);

    // Should return early without calling any menu functions
    manager->runContextMenu(item, 123, QPoint(0, 0));

    // Test passes if no crash
    EXPECT_TRUE(true);

    delete item;

    // Restore default state
    SideBarHelper::contextMenuEnabled = true;
}

TEST_F(UT_SideBarManager, RunContextMenu_NullItem)
{
    SideBarHelper::contextMenuEnabled = true;

    // Should not crash with null item
    manager->runContextMenu(nullptr, 123, QPoint(0, 0));

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarManager, RunContextMenu_SeparatorItem)
{
    SideBarHelper::contextMenuEnabled = true;

    SideBarItemSeparator *separator = new SideBarItemSeparator(DefaultGroup::kCommon);

    // Should not show context menu for separator items
    manager->runContextMenu(separator, 123, QPoint(0, 0));

    EXPECT_TRUE(true);

    delete separator;
}

TEST_F(UT_SideBarManager, RunContextMenu_InvalidUrl)
{
    SideBarHelper::contextMenuEnabled = true;

    QUrl invalidUrl;  // Invalid URL
    SideBarItem *item = new SideBarItem(invalidUrl);

    stub.set_lamda(&SideBarItem::url, [invalidUrl]() {
        __DBG_STUB_INVOKE__
        return invalidUrl;
    });

    // Should return early with invalid URL
    manager->runContextMenu(item, 123, QPoint(10, 20));

    EXPECT_TRUE(true);

    delete item;
}

TEST_F(UT_SideBarManager, RunContextMenu_WithCallback)
{
    SideBarHelper::contextMenuEnabled = true;

    bool callbackInvoked = false;
    quint64 capturedWindowId = 0;
    QUrl capturedUrl;
    QPoint capturedPos;

    QUrl testUrl = QUrl::fromLocalFile("/home/context");
    SideBarItem *item = new SideBarItem(testUrl);

    stub.set_lamda(&SideBarItem::url, [testUrl]() {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    stub.set_lamda(&SideBarItem::itemInfo,
        [&callbackInvoked, &capturedWindowId, &capturedUrl, &capturedPos]() {
        __DBG_STUB_INVOKE__
        ItemInfo info;
        info.contextMenuCb = [&](quint64 winId, const QUrl &url, const QPoint &pos) {
            callbackInvoked = true;
            capturedWindowId = winId;
            capturedUrl = url;
            capturedPos = pos;
        };
        return info;
    });

    QPoint menuPos(100, 200);
    manager->runContextMenu(item, 99999, menuPos);

    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(capturedWindowId, 99999u);
    EXPECT_EQ(capturedUrl, testUrl);
    EXPECT_EQ(capturedPos, menuPos);

    delete item;
}

TEST_F(UT_SideBarManager, RunContextMenu_WithoutCallback_UseDefault)
{
    SideBarHelper::contextMenuEnabled = true;

    bool defaultMenuCalled = false;
    quint64 capturedWindowId = 0;
    QUrl capturedUrl;
    QPoint capturedPos;

    QUrl testUrl = QUrl::fromLocalFile("/home/defaultmenu");
    SideBarItem *item = new SideBarItem(testUrl);

    stub.set_lamda(&SideBarItem::url, [testUrl]() {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    stub.set_lamda(&SideBarItem::itemInfo, []() {
        __DBG_STUB_INVOKE__
        ItemInfo info;
        info.contextMenuCb = nullptr;
        return info;
    });

    stub.set_lamda(&SideBarHelper::defaultContextMenu,
        [&defaultMenuCalled, &capturedWindowId, &capturedUrl, &capturedPos]
        (quint64 winId, const QUrl &url, const QPoint &pos) {
        __DBG_STUB_INVOKE__
        defaultMenuCalled = true;
        capturedWindowId = winId;
        capturedUrl = url;
        capturedPos = pos;
    });

    QPoint menuPos(50, 75);
    manager->runContextMenu(item, 11111, menuPos);

    EXPECT_TRUE(defaultMenuCalled);
    EXPECT_EQ(capturedWindowId, 11111u);
    EXPECT_EQ(capturedUrl, testUrl);
    EXPECT_EQ(capturedPos, menuPos);

    delete item;
}

TEST_F(UT_SideBarManager, RunRename_NullItem)
{
    // Should not crash with null item
    manager->runRename(nullptr, 123, "NewName");

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarManager, RunRename_WithCallback)
{
    bool callbackInvoked = false;
    quint64 capturedWindowId = 0;
    QUrl capturedUrl;
    QString capturedName;

    QUrl testUrl = QUrl::fromLocalFile("/home/rename");
    SideBarItem *item = new SideBarItem(testUrl);

    stub.set_lamda(&SideBarItem::url, [testUrl]() {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    stub.set_lamda(&SideBarItem::itemInfo,
        [&callbackInvoked, &capturedWindowId, &capturedUrl, &capturedName]() {
        __DBG_STUB_INVOKE__
        ItemInfo info;
        info.renameCb = [&](quint64 winId, const QUrl &url, const QString &name) {
            callbackInvoked = true;
            capturedWindowId = winId;
            capturedUrl = url;
            capturedName = name;
        };
        return info;
    });

    QString newName = "RenamedItem";
    manager->runRename(item, 77777, newName);

    EXPECT_TRUE(callbackInvoked);
    EXPECT_EQ(capturedWindowId, 77777u);
    EXPECT_EQ(capturedUrl, testUrl);
    EXPECT_EQ(capturedName, newName);

    delete item;
}

TEST_F(UT_SideBarManager, RunRename_WithoutCallback)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/norename");
    SideBarItem *item = new SideBarItem(testUrl);

    stub.set_lamda(&SideBarItem::url, [testUrl]() {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    stub.set_lamda(&SideBarItem::itemInfo, []() {
        __DBG_STUB_INVOKE__
        ItemInfo info;
        info.renameCb = nullptr;
        return info;
    });

    // Should log warning but not crash
    manager->runRename(item, 88888, "SomeName");

    EXPECT_TRUE(true);

    delete item;
}

TEST_F(UT_SideBarManager, OpenFolderInASeparateProcess)
{
    bool helperCalled = false;
    QUrl capturedUrl;

    QUrl testUrl = QUrl::fromLocalFile("/home/separate");

    stub.set_lamda(&SideBarHelper::openFolderInASeparateProcess,
        [&helperCalled, &capturedUrl](const QUrl &url) {
        __DBG_STUB_INVOKE__
        helperCalled = true;
        capturedUrl = url;
    });

    manager->openFolderInASeparateProcess(testUrl);

    EXPECT_TRUE(helperCalled);
    EXPECT_EQ(capturedUrl, testUrl);
}

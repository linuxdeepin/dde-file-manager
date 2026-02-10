// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "events/sidebareventreceiver.h"
#include "treeviews/sidebarwidget.h"
#include "treeviews/sidebaritem.h"
#include "treemodels/sidebarmodel.h"
#include "utils/sidebarhelper.h"
#include "utils/sidebarinfocachemananger.h"

#include <dfm-base/utils/universalutils.h>
#include <dfm-framework/dpf.h>

#include <QUrl>

using namespace dfmplugin_sidebar;
DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE

class UT_SideBarEventReceiver : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        receiver = SideBarEventReceiver::instance();
        ASSERT_NE(receiver, nullptr);
    }

    virtual void TearDown() override
    {
        stub.clear();
    }

protected:
    SideBarEventReceiver *receiver { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarEventReceiver, Instance)
{
    auto ins1 = SideBarEventReceiver::instance();
    auto ins2 = SideBarEventReceiver::instance();

    EXPECT_NE(ins1, nullptr);
    EXPECT_EQ(ins1, ins2);
}

TEST_F(UT_SideBarEventReceiver, BindEvents)
{
    EXPECT_NO_THROW(receiver->bindEvents());
}

TEST_F(UT_SideBarEventReceiver, HandleSetContextMenuEnable_True)
{
    SideBarHelper::contextMenuEnabled = false;

    receiver->handleSetContextMenuEnable(true);

    EXPECT_TRUE(SideBarHelper::contextMenuEnabled);
}

TEST_F(UT_SideBarEventReceiver, HandleSetContextMenuEnable_False)
{
    SideBarHelper::contextMenuEnabled = true;

    receiver->handleSetContextMenuEnable(false);

    EXPECT_FALSE(SideBarHelper::contextMenuEnabled);
}

TEST_F(UT_SideBarEventReceiver, HandleItemHidden)
{
    bool setVisiableCalled = false;
    QUrl capturedUrl;
    bool capturedVisible = false;

    SideBarWidget *mockWidget = new SideBarWidget();
    QList<SideBarWidget *> widgetList;
    widgetList << mockWidget;

    stub.set_lamda(&SideBarHelper::allSideBar, [widgetList]() -> QList<SideBarWidget *> {
        __DBG_STUB_INVOKE__
        return widgetList;
    });

    stub.set_lamda(&SideBarWidget::setItemVisiable,
                   [&setVisiableCalled, &capturedUrl, &capturedVisible](SideBarWidget *, const QUrl &url, bool visible) {
                       __DBG_STUB_INVOKE__
                       setVisiableCalled = true;
                       capturedUrl = url;
                       capturedVisible = visible;
                   });

    QUrl testUrl = QUrl::fromLocalFile("/home/test");
    receiver->handleItemHidden(testUrl, true);

    EXPECT_TRUE(setVisiableCalled);
    EXPECT_EQ(capturedUrl, testUrl);
    EXPECT_TRUE(capturedVisible);

    delete mockWidget;
}

TEST_F(UT_SideBarEventReceiver, HandleItemTriggerEdit)
{
    bool editCalled = false;
    QUrl capturedUrl;

    quint64 testWindowId = 12345;

    SideBarWidget *mockWidget = new SideBarWidget();
    QList<SideBarWidget *> widgetList;
    widgetList << mockWidget;

    stub.set_lamda(&SideBarHelper::allSideBar, [widgetList]() -> QList<SideBarWidget *> {
        __DBG_STUB_INVOKE__
        return widgetList;
    });

    stub.set_lamda(&SideBarHelper::windowId, [testWindowId](QWidget *) -> quint64 {
        __DBG_STUB_INVOKE__
        return testWindowId;
    });

    stub.set_lamda(&SideBarWidget::editItem,
                   [&editCalled, &capturedUrl](SideBarWidget *, const QUrl &url) {
                       __DBG_STUB_INVOKE__
                       editCalled = true;
                       capturedUrl = url;
                   });

    QUrl testUrl = QUrl::fromLocalFile("/home/edit");
    receiver->handleItemTriggerEdit(testWindowId, testUrl);

    EXPECT_TRUE(editCalled);
    EXPECT_EQ(capturedUrl, testUrl);

    delete mockWidget;
}

TEST_F(UT_SideBarEventReceiver, HandleSidebarUpdateSelection)
{
    bool updateCalled = false;

    quint64 testWindowId = 54321;

    SideBarWidget *mockWidget = new SideBarWidget();
    QList<SideBarWidget *> widgetList;
    widgetList << mockWidget;

    stub.set_lamda(&SideBarHelper::allSideBar, [widgetList]() -> QList<SideBarWidget *> {
        __DBG_STUB_INVOKE__
        return widgetList;
    });

    stub.set_lamda(&SideBarHelper::windowId, [testWindowId](QWidget *) -> quint64 {
        __DBG_STUB_INVOKE__
        return testWindowId;
    });

    stub.set_lamda(&SideBarWidget::updateSelection,
                   [&updateCalled](SideBarWidget *) {
                       __DBG_STUB_INVOKE__
                       updateCalled = true;
                   });

    receiver->handleSidebarUpdateSelection(testWindowId);

    EXPECT_TRUE(updateCalled);

    delete mockWidget;
}

TEST_F(UT_SideBarEventReceiver, HandleItemAdd_AlreadyExists)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/exists");

    stub.set_lamda(static_cast<bool (SideBarInfoCacheMananger::*)(const ItemInfo &) const>(&SideBarInfoCacheMananger::contains),
                   [](SideBarInfoCacheMananger *, const ItemInfo &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;   // Item already exists
                   });

    QVariantMap properties;
    properties[PropertyKey::kGroup] = DefaultGroup::kCommon;

    bool result = receiver->handleItemAdd(testUrl, properties);

    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarEventReceiver, HandleItemAdd_InvalidItem)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/invalid");

    stub.set_lamda(static_cast<bool (SideBarInfoCacheMananger::*)(const ItemInfo &) const>(&SideBarInfoCacheMananger::contains),
                   [](SideBarInfoCacheMananger *, const ItemInfo &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&SideBarHelper::createItemByInfo, [](const ItemInfo &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return nullptr;   // Failed to create item
    });

    QVariantMap properties;
    bool result = receiver->handleItemAdd(testUrl, properties);

    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarEventReceiver, HandleItemAdd_Success)
{
    bool addCacheCalled = false;
    bool addItemCalled = false;

    QUrl testUrl = QUrl::fromLocalFile("/home/newadd");

    SideBarItem *mockItem = new SideBarItem(testUrl);
    SideBarWidget *mockWidget = new SideBarWidget();
    QList<SideBarWidget *> widgetList;
    widgetList << mockWidget;

    stub.set_lamda(static_cast<bool (SideBarInfoCacheMananger::*)(const ItemInfo &) const>(&SideBarInfoCacheMananger::contains),
                   [](SideBarInfoCacheMananger *, const ItemInfo &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&SideBarHelper::createItemByInfo, [mockItem](const ItemInfo &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return mockItem;
    });

    stub.set_lamda(&SideBarItem::group, []() -> QString {
        __DBG_STUB_INVOKE__
        return DefaultGroup::kCommon;
    });

    stub.set_lamda(&SideBarInfoCacheMananger::addItemInfoCache,
                   [&addCacheCalled](SideBarInfoCacheMananger *, const ItemInfo &) -> bool {
                       __DBG_STUB_INVOKE__
                       addCacheCalled = true;
                       return true;
                   });

    stub.set_lamda(&SideBarHelper::allSideBar, [widgetList]() -> QList<SideBarWidget *> {
        __DBG_STUB_INVOKE__
        return widgetList;
    });

    stub.set_lamda(&SideBarWidget::addItem,
                   [&addItemCalled](SideBarWidget *, SideBarItem *, bool) -> int {
                       __DBG_STUB_INVOKE__
                       addItemCalled = true;
                       return 0;   // Success
                   });

    stub.set_lamda(&SideBarItem::url, [testUrl]() -> QUrl {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    stub.set_lamda(VADDR(SideBarWidget, currentUrl), []() -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl();
    });

    stub.set_lamda(&UniversalUtils::urlEquals, [](const QUrl &, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });

    QVariantMap properties;
    properties[PropertyKey::kGroup] = DefaultGroup::kCommon;

    bool result = receiver->handleItemAdd(testUrl, properties);

    EXPECT_TRUE(result);
    EXPECT_TRUE(addCacheCalled);
    EXPECT_TRUE(addItemCalled);

    delete mockWidget;
    // mockItem will be managed by widget, don't delete
}

TEST_F(UT_SideBarEventReceiver, HandleItemRemove_NotFound)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/notfound");

    stub.set_lamda(static_cast<bool (SideBarInfoCacheMananger::*)(const QUrl &) const>(&SideBarInfoCacheMananger::contains),
                   [](SideBarInfoCacheMananger *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    bool result = receiver->handleItemRemove(testUrl);

    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarEventReceiver, HandleItemRemove_Success)
{
    bool removeCacheCalled = false;
    bool removeRowCalled = false;

    QUrl testUrl = QUrl::fromLocalFile("/home/remove");

    stub.set_lamda(static_cast<bool (SideBarInfoCacheMananger::*)(const QUrl &) const>(&SideBarInfoCacheMananger::contains),
                   [](SideBarInfoCacheMananger *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return true;
                   });

    stub.set_lamda(static_cast<bool (SideBarInfoCacheMananger::*)(const QUrl &)>(&SideBarInfoCacheMananger::removeItemInfoCache),
                   [&removeCacheCalled](SideBarInfoCacheMananger *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       removeCacheCalled = true;
                       return true;
                   });

    // Mock kSidebarModelIns to be non-null
    SideBarModel mockModel;
    SideBarWidget::kSidebarModelIns = QSharedPointer<SideBarModel>(&mockModel, [](SideBarModel *) {});

    stub.set_lamda(&SideBarModel::removeRow,
                   [&removeRowCalled](SideBarModel *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       removeRowCalled = true;
                       return true;
                   });

    bool result = receiver->handleItemRemove(testUrl);

    EXPECT_TRUE(result);
    EXPECT_TRUE(removeCacheCalled);
    EXPECT_TRUE(removeRowCalled);

    // Cleanup
    SideBarWidget::kSidebarModelIns.reset();
}

TEST_F(UT_SideBarEventReceiver, HandleItemUpdate_NotFound)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/updatenotfound");

    stub.set_lamda(static_cast<bool (SideBarInfoCacheMananger::*)(const QUrl &) const>(&SideBarInfoCacheMananger::contains),
                   [](SideBarInfoCacheMananger *, const QUrl &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    QVariantMap properties;
    bool result = receiver->handleItemUpdate(testUrl, properties);

    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarEventReceiver, HandleItemInsert_Success)
{
    bool insertCacheCalled = false;

    QUrl testUrl = QUrl::fromLocalFile("/home/insert");

    SideBarItem *mockItem = new SideBarItem(testUrl);

    stub.set_lamda(static_cast<bool (SideBarInfoCacheMananger::*)(const ItemInfo &) const>(&SideBarInfoCacheMananger::contains),
                   [](SideBarInfoCacheMananger *, const ItemInfo &) -> bool {
                       __DBG_STUB_INVOKE__
                       return false;
                   });

    stub.set_lamda(&SideBarHelper::createItemByInfo, [mockItem](const ItemInfo &) -> SideBarItem * {
        __DBG_STUB_INVOKE__
        return mockItem;
    });

    stub.set_lamda(&SideBarItem::group, []() -> QString {
        __DBG_STUB_INVOKE__
        return DefaultGroup::kCommon;
    });

    stub.set_lamda(&SideBarInfoCacheMananger::insertItemInfoCache,
                   [&insertCacheCalled](SideBarInfoCacheMananger *, int, const ItemInfo &) -> bool {
                       __DBG_STUB_INVOKE__
                       insertCacheCalled = true;
                       return true;
                   });

    SideBarWidget *mockWidget = new SideBarWidget();
    QList<SideBarWidget *> widgetList;
    widgetList << mockWidget;

    stub.set_lamda(&SideBarHelper::allSideBar, [widgetList]() -> QList<SideBarWidget *> {
        __DBG_STUB_INVOKE__
        return widgetList;
    });

    stub.set_lamda(&SideBarWidget::insertItem,
                   [](SideBarWidget *, int, SideBarItem *) -> int {
                       __DBG_STUB_INVOKE__
                       return 0;   // Success
                   });
    stub.set_lamda(&SideBarWidget::insertItem, [] {__DBG_STUB_INVOKE__ return true;});

    QVariantMap properties;
    properties[PropertyKey::kGroup] = DefaultGroup::kCommon;

    bool result = receiver->handleItemInsert(0, testUrl, properties);

    EXPECT_TRUE(result);
    EXPECT_TRUE(insertCacheCalled);

    delete mockWidget;
}

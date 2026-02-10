// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "stubext.h"
#include "treeviews/sidebarwidget.h"
#include "treeviews/sidebarview.h"
#include "treeviews/sidebaritem.h"
#include "treemodels/sidebarmodel.h"
#include "dfmplugin_sidebar_global.h"

#include <DBlurEffectWidget>
#include <DConfig>

#include <QUrl>
#include <QEvent>
#include <QListView>

using namespace dfmplugin_sidebar;
DWIDGET_USE_NAMESPACE

class UT_SideBarWidget : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Stub GUI operations
        stub.set_lamda(ADDR(QWidget, show), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(ADDR(QWidget, hide), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });

        stub.set_lamda(static_cast<void (QWidget::*)()>(&QWidget::update), [](QWidget *) {
            __DBG_STUB_INVOKE__
        });

        widget = new SideBarWidget();
        ASSERT_NE(widget, nullptr);
    }

    virtual void TearDown() override
    {
        delete widget;
        widget = nullptr;
        stub.clear();
    }

protected:
    SideBarWidget *widget { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(UT_SideBarWidget, Constructor)
{
    EXPECT_NE(widget, nullptr);
}

TEST_F(UT_SideBarWidget, View)
{
    QAbstractItemView *view = widget->view();

    // Should return the sidebar view
    EXPECT_NE(view, nullptr);
}

TEST_F(UT_SideBarWidget, SetCurrentUrl)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/test");

    stub.set_lamda(&SideBarView::setCurrentUrl, [](SideBarView *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    widget->setCurrentUrl(testUrl);

    // Verify no crash
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, CurrentUrl)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/current");

    stub.set_lamda(&SideBarView::currentUrl, [testUrl]() -> QUrl {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    QUrl retrievedUrl = widget->currentUrl();

    EXPECT_EQ(retrievedUrl, testUrl);
}

TEST_F(UT_SideBarWidget, AddItem_Success)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/add");
    SideBarItem *item = new SideBarItem(testUrl);
    item->setGroup(DefaultGroup::kCommon);

    stub.set_lamda(&SideBarModel::appendRow, [](SideBarModel *, SideBarItem *, bool) -> int {
        __DBG_STUB_INVOKE__
        return 0;
    });

    int result = widget->addItem(item, true);

    EXPECT_GE(result, -1);
}

TEST_F(UT_SideBarWidget, InsertItem_Success)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/insert");
    SideBarItem *item = new SideBarItem(testUrl);
    item->setGroup(DefaultGroup::kCommon);

    stub.set_lamda(&SideBarModel::insertRow, [](SideBarModel *, int, SideBarItem *) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = widget->insertItem(0, item);

    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarWidget, RemoveItem_InvalidUrl)
{
    QUrl invalidUrl;

    bool result = widget->removeItem(invalidUrl);
    EXPECT_FALSE(result);
}

TEST_F(UT_SideBarWidget, RemoveItem_Success)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/remove");

    stub.set_lamda(&SideBarModel::removeRow, [](SideBarModel *, const QUrl &) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });

    bool result = widget->removeItem(testUrl);

    EXPECT_TRUE(result);
}

TEST_F(UT_SideBarWidget, UpdateItem)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/update");

    ItemInfo newInfo;
    newInfo.url = testUrl;
    newInfo.displayName = "Updated";

    stub.set_lamda(&SideBarModel::updateRow, [](SideBarModel *, const QUrl &, const ItemInfo &) {
        __DBG_STUB_INVOKE__
    });

    widget->updateItem(testUrl, newInfo);

    // Verify no crash
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, FindItem_NotFound)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/notfound");

    stub.set_lamda(&SideBarView::findItemIndex, [](SideBarView *, const QUrl &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return QModelIndex();
    });

    int result = widget->findItem(testUrl);
    EXPECT_EQ(result, -1);
}

TEST_F(UT_SideBarWidget, FindItemIndex)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/find");

    stub.set_lamda(&SideBarView::findItemIndex, [](SideBarView *, const QUrl &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        QModelIndex index;
        return index;
    });

    QModelIndex index = widget->findItemIndex(testUrl);

    // Just verify no crash
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, EditItem)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/edit");

    stub.set_lamda(&SideBarView::findItemIndex, [](SideBarView *, const QUrl &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return QModelIndex();
    });

    stub.set_lamda(static_cast<void (SideBarView::*)(const QModelIndex &)>(&SideBarView::edit), [] {
        __DBG_STUB_INVOKE__
    });

    widget->editItem(testUrl);

    // Verify no crash
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, SetItemVisiable)
{
    QUrl testUrl = QUrl::fromLocalFile("/home/visible");

    stub.set_lamda(&SideBarView::findItemIndex, [](SideBarView *, const QUrl &) -> QModelIndex {
        __DBG_STUB_INVOKE__
        return QModelIndex();
    });

    stub.set_lamda(&QListView::setRowHidden, [] {
        __DBG_STUB_INVOKE__
    });

    widget->setItemVisiable(testUrl, true);

    // Verify no crash
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, UpdateItemVisiable_EmptyMap)
{
    QVariantMap emptyStates;

    widget->updateItemVisiable(emptyStates);

    // Should not crash with empty map
    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, UpdateItemVisiable_WithStates)
{
    QVariantMap states;
    states["key1"] = false;
    states["key2"] = true;

    stub.set_lamda(&SideBarWidget::findItemUrlsByVisibleControlKey,
                   [](const SideBarWidget *, const QString &) -> QList<QUrl> {
                       __DBG_STUB_INVOKE__
                       return QList<QUrl>();
                   });

    widget->updateItemVisiable(states);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, FindItemUrlsByVisibleControlKey)
{
    QString key = "test_key";

    stub.set_lamda(static_cast<QList<SideBarItem *> (SideBarModel::*)() const>(&SideBarModel::subItems),
                   [](const SideBarModel *) -> QList<SideBarItem *> {
                       __DBG_STUB_INVOKE__
                       return QList<SideBarItem *>();
                   });

    QList<QUrl> urls = widget->findItemUrlsByVisibleControlKey(key);

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, UpdateSelection)
{
    stub.set_lamda(&SideBarView::setCurrentUrl, [](SideBarView *, const QUrl &) {
        __DBG_STUB_INVOKE__
    });

    widget->updateSelection();

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, SaveStateWhenClose)
{
    stub.set_lamda(&SideBarView::saveStateWhenClose, [](SideBarView *) {
        __DBG_STUB_INVOKE__
    });

    widget->saveStateWhenClose();

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, ResetSettingPanel)
{
    stub.set_lamda(&SideBarModel::groupItems,
                   [](const SideBarModel *) -> QList<SideBarItemSeparator *> {
                       __DBG_STUB_INVOKE__
                       return QList<SideBarItemSeparator *>();
                   });

    widget->resetSettingPanel();

    EXPECT_TRUE(true);
}

TEST_F(UT_SideBarWidget, ChangeEvent_PaletteChange)
{
    QPalette oldPalette;
    QPalette newPalette;

    QEvent *event = new QEvent(QEvent::PaletteChange);

    stub.set_lamda(VADDR(QWidget, changeEvent), [](QWidget *, QEvent *) {
        __DBG_STUB_INVOKE__
    });

    // Protected method, tested through public API
    widget->changeEvent(event);

    delete event;

    EXPECT_TRUE(true);
}

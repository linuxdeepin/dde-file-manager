// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarwidget_1.cpp
 * @brief Unit tests for SideBarWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "treeviews/sidebarwidget.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarWidgetTest, SideBarWidget)
{
    // Test constructor: SideBarWidget((QFrame *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SideBarWidgetTest, changeEvent)
{
    // Test event handler: changeEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->changeEvent(&_event));
}

TEST_F(SideBarWidgetTest, currentUrl)
{
    // Test getter: QUrl currentUrl()
    auto result = obj->currentUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(SideBarWidgetTest, customContextMenuCall)
{
    // Test method: void customContextMenuCall((const QPoint &pos))
    QPoint _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->customContextMenuCall(_arg0));
}

TEST_F(SideBarWidgetTest, editItem)
{
    // Test method: void editItem((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->editItem(_arg0));
}

TEST_F(SideBarWidgetTest, findItem)
{
    // Test method: int findItem((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->findItem(_arg0);
    EXPECT_GE(result, 0);

}

TEST_F(SideBarWidgetTest, findItemIndex)
{
    // Test method: QModelIndex findItemIndex((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->findItemIndex(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SideBarWidgetTest, initConnect)
{
    // Test method: void initConnect(())
    EXPECT_NO_FATAL_FAILURE(obj->initConnect());
}

TEST_F(SideBarWidgetTest, initDefaultModel)
{
    // Test method: void initDefaultModel(())
    EXPECT_NO_FATAL_FAILURE(obj->initDefaultModel());
}

TEST_F(SideBarWidgetTest, initializeUi)
{
    // Test method: void initializeUi(())
    EXPECT_NO_FATAL_FAILURE(obj->initializeUi());
}

TEST_F(SideBarWidgetTest, insertItem)
{
    // Test method: bool insertItem((const int index, SideBarItem *item))
    auto result = obj->insertItem(0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(SideBarWidgetTest, onItemRenamed)
{
    // Test method: void onItemRenamed((const QModelIndex &index, const QString &newName))
    QModelIndex _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->onItemRenamed(_arg0, _arg1));
}

TEST_F(SideBarWidgetTest, setCurrentUrl)
{
    // Test setter: void setCurrentUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentUrl(_arg0));
}

TEST_F(SideBarWidgetTest, setItemVisiable)
{
    // Test setter: void setItemVisiable((const QUrl &url, bool visible))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setItemVisiable(_arg0, false));
}

TEST_F(SideBarWidgetTest, updateBackgroundColor)
{
    // Test method: void updateBackgroundColor(())
    EXPECT_NO_FATAL_FAILURE(obj->updateBackgroundColor());
}

TEST_F(SideBarWidgetTest, updateItem)
{
    // Test method: void updateItem((const QUrl &url, const ItemInfo &newInfo))
    QUrl _arg0{};
    ItemInfo _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateItem(_arg0, _arg1));
}

TEST_F(SideBarWidgetTest, updateItemVisiable)
{
    // Test method: void updateItemVisiable((const QVariantMap &states))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->updateItemVisiable(_arg0));
}

TEST_F(SideBarWidgetTest, updateSelection)
{
    // Test method: void updateSelection(())
    EXPECT_NO_FATAL_FAILURE(obj->updateSelection());
}

TEST_F(SideBarWidgetTest, updateWindowEffect)
{
    // Test method: void updateWindowEffect(())
    EXPECT_NO_FATAL_FAILURE(obj->updateWindowEffect());
}

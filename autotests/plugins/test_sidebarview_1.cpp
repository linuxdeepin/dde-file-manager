// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarview_1.cpp
 * @brief Unit tests for SideBarView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "treeviews/sidebarview.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarViewTest, SideBarView)
{
    // Test constructor: SideBarView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SideBarViewTest, currentUrl)
{
    // Test getter: QUrl currentUrl()
    auto result = obj->currentUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(SideBarViewTest, dragEventUrls)
{
    // Test getter: QString dragEventUrls()
    auto result = obj->dragEventUrls();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarViewTest, dragItemVerticalOffset)
{
    // Test method: int dragItemVerticalOffset((const QModelIndex &index, int rowHeight))
    QModelIndex _arg0{};
    auto result = obj->dragItemVerticalOffset(_arg0, 0);
    EXPECT_GE(result, 0);

}

TEST_F(SideBarViewTest, dragLeaveEvent)
{
    // Test event handler: dragLeaveEvent((QDragLeaveEvent *event))
    QDragLeaveEvent _event(QDragLeaveEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragLeaveEvent(&_event));
}

TEST_F(SideBarViewTest, groupExpandState)
{
    // Test getter: QVariantMap groupExpandState()
    auto result = obj->groupExpandState();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SideBarViewTest, indexAt)
{
    // Test method: QModelIndex indexAt((const QPoint &p))
    QPoint _arg0{};
    auto result = obj->indexAt(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SideBarViewTest, isAccepteDragEvent)
{
    // Test event handler: isAccepteDragEvent((QDropEvent *event))
    QDropEvent _event(QDropEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->isAccepteDragEvent(&_event));
}

TEST_F(SideBarViewTest, isDraggedSource)
{
    // Test method: bool isDraggedSource((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isDraggedSource(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SideBarViewTest, isDropTarget)
{
    // Test method: bool isDropTarget((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isDropTarget(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(SideBarViewTest, isPartitionExpandable)
{
    // Test bool getter: isPartitionExpandable()
    bool result = obj->isPartitionExpandable();
    EXPECT_FALSE(result);

}

TEST_F(SideBarViewTest, isRenderingDragPreview)
{
    // Test bool getter: isRenderingDragPreview()
    bool result = obj->isRenderingDragPreview();
    EXPECT_FALSE(result);

}

TEST_F(SideBarViewTest, isSideBarItemDragged)
{
    // Test bool getter: isSideBarItemDragged()
    bool result = obj->isSideBarItemDragged();
    EXPECT_FALSE(result);

}

TEST_F(SideBarViewTest, itemAt)
{
    // Test method: SideBarItem itemAt((const QPoint &pt))
    QPoint _arg0{};
    auto result = obj->itemAt(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->itemAt(_arg0); });

}

TEST_F(SideBarViewTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}

TEST_F(SideBarViewTest, onRequestCollapseItem)
{
    // Test method: void onRequestCollapseItem((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onRequestCollapseItem(_arg0));
}

TEST_F(SideBarViewTest, previousIndex)
{
    // Test getter: QModelIndex previousIndex()
    auto result = obj->previousIndex();
    EXPECT_FALSE(result.isValid());

}

TEST_F(SideBarViewTest, setPreviousIndex)
{
    // Test setter: void setPreviousIndex((const QModelIndex &index))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setPreviousIndex(_arg0));
}

TEST_F(SideBarViewTest, updateEditTriggers)
{
    // Test method: void updateEditTriggers(())
    EXPECT_NO_FATAL_FAILURE(obj->updateEditTriggers());
}

TEST_F(SideBarViewTest, urlAt)
{
    // Test method: QUrl urlAt((const QPoint &pt))
    QPoint _arg0{};
    auto result = obj->urlAt(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SideBarViewTest, SideBarView_Destructor)
{
    // Test method:  ~SideBarView(())
    EXPECT_NO_FATAL_FAILURE({ SideBarView *tmp = new SideBarView(); delete tmp; });
}

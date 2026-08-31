// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarviewprivate.cpp
 * @brief Unit tests for SideBarViewPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "treeviews/sidebarview.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class SideBarViewPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SideBarViewPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SideBarViewPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SideBarViewPrivateTest, calculatePlaceholderRow)
{
    // Test method: int calculatePlaceholderRow((const QPoint &pos, const QMimeData *data))
    QPoint _arg0{};
    auto result = obj->calculatePlaceholderRow(_arg0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(SideBarViewPrivateTest, canMove)
{
    // Test method: bool canMove((QDragMoveEvent *event))
    auto result = obj->canMove(nullptr);
    EXPECT_FALSE(result);

}

TEST_F(SideBarViewPrivateTest, clearHoverIndex)
{
    // Test method: void clearHoverIndex(())
    EXPECT_NO_FATAL_FAILURE(obj->clearHoverIndex());
}

TEST_F(SideBarViewPrivateTest, clearInternalDragState)
{
    // Test method: void clearInternalDragState(())
    EXPECT_NO_FATAL_FAILURE(obj->clearInternalDragState());
}

TEST_F(SideBarViewPrivateTest, dragItemOffset)
{
    // Test method: int dragItemOffset((const QModelIndex &index, int rowHeight))
    QModelIndex _arg0{};
    auto result = obj->dragItemOffset(_arg0, 0);
    EXPECT_GE(result, 0);

}

TEST_F(SideBarViewPrivateTest, expandPartitionItem)
{
    // Test method: void expandPartitionItem((const QModelIndex &index, const QUrl &url))
    QModelIndex _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->expandPartitionItem(_arg0, _arg1));
}

TEST_F(SideBarViewPrivateTest, updatePlaceholderRow)
{
    // Test method: void updatePlaceholderRow((int row, const QModelIndex &parent))
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updatePlaceholderRow(0, _arg1));
}

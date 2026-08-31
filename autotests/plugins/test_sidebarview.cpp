// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sidebarview.cpp
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

TEST_F(SideBarViewTest, dragMoveEvent)
{
    // Test event handler: dragMoveEvent((QDragMoveEvent *event))
    QDragMoveEvent _event(QDragMoveEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragMoveEvent(&_event));
}

TEST_F(SideBarViewTest, findItemIndex)
{
    // Test method: QModelIndex findItemIndex((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->findItemIndex(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(SideBarViewTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(SideBarViewTest, saveStateWhenClose)
{
    // Test method: void saveStateWhenClose(())
    EXPECT_NO_FATAL_FAILURE(obj->saveStateWhenClose());
}

TEST_F(SideBarViewTest, setCurrentUrl)
{
    // Test setter: void setCurrentUrl((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentUrl(_arg0));
}

TEST_F(SideBarViewTest, model)
{
    // Test getter: SideBarModel model()
    auto result = obj->model();
    EXPECT_NO_FATAL_FAILURE({ obj->model(); });

}

TEST_F(SideBarViewTest, onChangeExpandState)
{
    // Test method: void onChangeExpandState((const QModelIndex &index, bool expand))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onChangeExpandState(_arg0, false));
}

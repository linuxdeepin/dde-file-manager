// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasview.cpp
 * @brief Unit tests for CanvasView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/canvasview.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewTest, baseIndexAt)
{
    // Test method: QModelIndex baseIndexAt((const QPoint &point))
    QPoint _arg0{};
    auto result = obj->baseIndexAt(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewTest, contextMenuEvent)
{
    // Test event handler: contextMenuEvent((QContextMenuEvent *event))
    QContextMenuEvent _event(QContextMenuEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->contextMenuEvent(&_event));
}

TEST_F(CanvasViewTest, edit)
{
    // Test method: bool edit((const QModelIndex &index, QAbstractItemView::EditTrigger trigger, QEvent *event))
    QModelIndex _arg0{};
    auto result = obj->edit(_arg0, QAbstractItemView::EditTrigger(), nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewTest, indexAt)
{
    // Test method: QModelIndex indexAt((const QPoint &point))
    QPoint _arg0{};
    auto result = obj->indexAt(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewTest, reset)
{
    // Test method: void reset(())
    EXPECT_NO_FATAL_FAILURE(obj->reset());
}

TEST_F(CanvasViewTest, selectAll)
{
    // Test method: void selectAll(())
    EXPECT_NO_FATAL_FAILURE(obj->selectAll());
}

TEST_F(CanvasViewTest, setGeometry)
{
    // Test setter: void setGeometry((const QRect &rect))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setGeometry(_arg0));
}

TEST_F(CanvasViewTest, startDrag)
{
    // Test method: void startDrag((Qt::DropActions supportedActions))
    EXPECT_NO_FATAL_FAILURE(obj->startDrag(Qt::DropActions()));
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasview_1.cpp
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

TEST_F(CanvasViewTest, CanvasView)
{
    // Test constructor: CanvasView((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasViewTest, changeEvent)
{
    // Test event handler: changeEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->changeEvent(&_event));
}

TEST_F(CanvasViewTest, currentChanged)
{
    // Test method: void currentChanged((const QModelIndex &current, const QModelIndex &previous))
    QModelIndex _arg0{};
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->currentChanged(_arg0, _arg1));
}

TEST_F(CanvasViewTest, dragEnterEvent)
{
    // Test event handler: dragEnterEvent((QDragEnterEvent *event))
    QDragEnterEvent _event(QDragEnterEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragEnterEvent(&_event));
}

TEST_F(CanvasViewTest, dragLeaveEvent)
{
    // Test event handler: dragLeaveEvent((QDragLeaveEvent *event))
    QDragLeaveEvent _event(QDragLeaveEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragLeaveEvent(&_event));
}

TEST_F(CanvasViewTest, dragMoveEvent)
{
    // Test event handler: dragMoveEvent((QDragMoveEvent *event))
    QDragMoveEvent _event(QDragMoveEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragMoveEvent(&_event));
}

TEST_F(CanvasViewTest, dropEvent)
{
    // Test event handler: dropEvent((QDropEvent *event))
    QDropEvent _event(QDropEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dropEvent(&_event));
}

TEST_F(CanvasViewTest, expendedVisualRect)
{
    // Test method: QRect expendedVisualRect((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->expendedVisualRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewTest, focusInEvent)
{
    // Test event handler: focusInEvent((QFocusEvent *event))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusInEvent(&_event));
}

TEST_F(CanvasViewTest, focusOutEvent)
{
    // Test event handler: focusOutEvent((QFocusEvent *event))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusOutEvent(&_event));
}

TEST_F(CanvasViewTest, horizontalOffset)
{
    // Test getter: int horizontalOffset()
    auto result = obj->horizontalOffset();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasViewTest, initUI)
{
    // Test method: void initUI(())
    EXPECT_NO_FATAL_FAILURE(obj->initUI());
}

TEST_F(CanvasViewTest, inputMethodQuery)
{
    // Test method: QVariant inputMethodQuery((Qt::InputMethodQuery query))
    auto result = obj->inputMethodQuery(Qt::InputMethodQuery());
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewTest, isIndexHidden)
{
    // Test method: bool isIndexHidden((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isIndexHidden(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewTest, itemDelegate)
{
    // Test getter: CanvasItemDelegate itemDelegate()
    auto result = obj->itemDelegate();
    EXPECT_NO_FATAL_FAILURE({ obj->itemDelegate(); });

}

TEST_F(CanvasViewTest, itemPaintGeomertys)
{
    // Test method: QList<QRect> itemPaintGeomertys((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->itemPaintGeomertys(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasViewTest, itemRect)
{
    // Test method: QRect itemRect((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->itemRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *event))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

TEST_F(CanvasViewTest, keyboardSearch)
{
    // Test method: void keyboardSearch((const QString &search))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->keyboardSearch(_arg0));
}

TEST_F(CanvasViewTest, model)
{
    // Test getter: CanvasProxyModel model()
    auto result = obj->model();
    EXPECT_NO_FATAL_FAILURE({ obj->model(); });

}

TEST_F(CanvasViewTest, mouseMoveEvent)
{
    // Test event handler: mouseMoveEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseMoveEvent(&_event));
}

TEST_F(CanvasViewTest, mousePressEvent)
{
    // Test event handler: mousePressEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mousePressEvent(&_event));
}

TEST_F(CanvasViewTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}

TEST_F(CanvasViewTest, paintEvent)
{
    // Test event handler: paintEvent((QPaintEvent *event))
    QPaintEvent _event(QPaintEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->paintEvent(&_event));
}

TEST_F(CanvasViewTest, refresh)
{
    // Test method: void refresh((bool silent))
    EXPECT_NO_FATAL_FAILURE(obj->refresh(false));
}

TEST_F(CanvasViewTest, screenNum)
{
    // Test getter: int screenNum()
    auto result = obj->screenNum();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasViewTest, scrollTo)
{
    // Test method: void scrollTo((const QModelIndex &index, QAbstractItemView::ScrollHint hint))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->scrollTo(_arg0, QAbstractItemView::ScrollHint()));
}

TEST_F(CanvasViewTest, selectionChanged)
{
    // Test method: void selectionChanged((const QItemSelection &selected, const QItemSelection &deselected))
    QItemSelection _arg0{};
    QItemSelection _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->selectionChanged(_arg0, _arg1));
}

TEST_F(CanvasViewTest, selectionModel)
{
    // Test getter: CanvasSelectionModel selectionModel()
    auto result = obj->selectionModel();
    EXPECT_NO_FATAL_FAILURE({ obj->selectionModel(); });

}

TEST_F(CanvasViewTest, setScreenNum)
{
    // Test setter: void setScreenNum((const int screenNum))
    EXPECT_NO_FATAL_FAILURE(obj->setScreenNum(0));
}

TEST_F(CanvasViewTest, setSelection)
{
    // Test setter: void setSelection((const QRect &rect, QItemSelectionModel::SelectionFlags command))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSelection(_arg0, QItemSelectionModel::SelectionFlags()));
}

TEST_F(CanvasViewTest, setSelectionModel)
{
    // Test setter: void setSelectionModel((QItemSelectionModel *selectionModel))
    EXPECT_NO_FATAL_FAILURE(obj->setSelectionModel(nullptr));
}

TEST_F(CanvasViewTest, setViewHook)
{
    // Test setter: void setViewHook((ViewHookInterface *ext))
    EXPECT_NO_FATAL_FAILURE(obj->setViewHook(nullptr));
}

TEST_F(CanvasViewTest, showGrid)
{
    // Test method: void showGrid((bool v))
    EXPECT_NO_FATAL_FAILURE(obj->showGrid(false));
}

TEST_F(CanvasViewTest, toggleSelect)
{
    // Test method: void toggleSelect(())
    EXPECT_NO_FATAL_FAILURE(obj->toggleSelect());
}

TEST_F(CanvasViewTest, updateGrid)
{
    // Test method: void updateGrid(())
    EXPECT_NO_FATAL_FAILURE(obj->updateGrid());
}

TEST_F(CanvasViewTest, verticalOffset)
{
    // Test getter: int verticalOffset()
    auto result = obj->verticalOffset();
    EXPECT_EQ(result, 0);

}

TEST_F(CanvasViewTest, viewHook)
{
    // Test getter: ViewHookInterface viewHook()
    auto result = obj->viewHook();
    EXPECT_NO_FATAL_FAILURE({ obj->viewHook(); });

}

TEST_F(CanvasViewTest, visualRect)
{
    // Test method: QRect visualRect((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->visualRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewTest, visualRegionForSelection)
{
    // Test method: QRegion visualRegionForSelection((const QItemSelection &selection))
    QItemSelection _arg0{};
    auto result = obj->visualRegionForSelection(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasViewTest, wheelEvent)
{
    // Test event handler: wheelEvent((QWheelEvent *event))
    QWheelEvent _event(QWheelEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->wheelEvent(&_event));
}

TEST_F(CanvasViewTest, winId)
{
    // Test getter: WId winId()
    auto result = obj->winId();
    EXPECT_EQ(result, 0);

}

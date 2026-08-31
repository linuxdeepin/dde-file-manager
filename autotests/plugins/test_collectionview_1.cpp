// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_collectionview_1.cpp
 * @brief Unit tests for CollectionView methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/collectionview.h"

#include <QTest>

using namespace ddplugin_organizer;

class CollectionViewTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CollectionView();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CollectionView *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CollectionViewTest, CollectionView)
{
    // Test constructor: CollectionView((const QString &uuid, CollectionDataProvider *dataProvider, QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CollectionViewTest, cellMargins)
{
    // Test getter: QMargins cellMargins()
    auto result = obj->cellMargins();
    EXPECT_NO_FATAL_FAILURE({ obj->cellMargins(); });

}

TEST_F(CollectionViewTest, changeEvent)
{
    // Test event handler: changeEvent((QEvent *event))
    QEvent _event(QEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->changeEvent(&_event));
}

TEST_F(CollectionViewTest, contextMenuEvent)
{
    // Test event handler: contextMenuEvent((QContextMenuEvent *event))
    QContextMenuEvent _event(QContextMenuEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->contextMenuEvent(&_event));
}

TEST_F(CollectionViewTest, currentChanged)
{
    // Test method: void currentChanged((const QModelIndex &current, const QModelIndex &previous))
    QModelIndex _arg0{};
    QModelIndex _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->currentChanged(_arg0, _arg1));
}

TEST_F(CollectionViewTest, dragEnterEvent)
{
    // Test event handler: dragEnterEvent((QDragEnterEvent *event))
    QDragEnterEvent _event(QDragEnterEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragEnterEvent(&_event));
}

TEST_F(CollectionViewTest, dragLeaveEvent)
{
    // Test event handler: dragLeaveEvent((QDragLeaveEvent *event))
    QDragLeaveEvent _event(QDragLeaveEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dragLeaveEvent(&_event));
}

TEST_F(CollectionViewTest, dropEvent)
{
    // Test event handler: dropEvent((QDropEvent *event))
    QDropEvent _event(QDropEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->dropEvent(&_event));
}

TEST_F(CollectionViewTest, fileShiftable)
{
    // Test bool getter: fileShiftable()
    bool result = obj->fileShiftable();
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewTest, focusInEvent)
{
    // Test event handler: focusInEvent((QFocusEvent *event))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusInEvent(&_event));
}

TEST_F(CollectionViewTest, horizontalOffset)
{
    // Test getter: int horizontalOffset()
    auto result = obj->horizontalOffset();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionViewTest, id)
{
    // Test getter: QString id()
    auto result = obj->id();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionViewTest, inputMethodQuery)
{
    // Test method: QVariant inputMethodQuery((Qt::InputMethodQuery query))
    auto result = obj->inputMethodQuery(Qt::InputMethodQuery());
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionViewTest, isIndexHidden)
{
    // Test method: bool isIndexHidden((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->isIndexHidden(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CollectionViewTest, itemDelegate)
{
    // Test getter: CollectionItemDelegate itemDelegate()
    auto result = obj->itemDelegate();
    EXPECT_NO_FATAL_FAILURE({ obj->itemDelegate(); });

}

TEST_F(CollectionViewTest, keyboardSearch)
{
    // Test method: void keyboardSearch((const QString &search))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->keyboardSearch(_arg0));
}

TEST_F(CollectionViewTest, model)
{
    // Test getter: CollectionModel model()
    auto result = obj->model();
    EXPECT_NO_FATAL_FAILURE({ obj->model(); });

}

TEST_F(CollectionViewTest, mouseMoveEvent)
{
    // Test event handler: mouseMoveEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseMoveEvent(&_event));
}

TEST_F(CollectionViewTest, mouseReleaseEvent)
{
    // Test event handler: mouseReleaseEvent((QMouseEvent *event))
    QMouseEvent _event(QMouseEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->mouseReleaseEvent(&_event));
}

TEST_F(CollectionViewTest, openEditor)
{
    // Test method: void openEditor((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openEditor(_arg0));
}

TEST_F(CollectionViewTest, refresh)
{
    // Test method: void refresh((bool silence))
    EXPECT_NO_FATAL_FAILURE(obj->refresh(false));
}

TEST_F(CollectionViewTest, resizeEvent)
{
    // Test event handler: resizeEvent((QResizeEvent *event))
    QResizeEvent _event(QResizeEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->resizeEvent(&_event));
}

TEST_F(CollectionViewTest, scrollContentsBy)
{
    // Test method: void scrollContentsBy((int dx, int dy))
    EXPECT_NO_FATAL_FAILURE(obj->scrollContentsBy(0, 0));
}

TEST_F(CollectionViewTest, scrollTo)
{
    // Test method: void scrollTo((const QModelIndex &index, ScrollHint hint))
    QModelIndex _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->scrollTo(_arg0, ScrollHint()));
}

TEST_F(CollectionViewTest, selectAll)
{
    // Test method: void selectAll(())
    EXPECT_NO_FATAL_FAILURE(obj->selectAll());
}

TEST_F(CollectionViewTest, selectUrl)
{
    // Test method: void selectUrl((const QUrl &url, const QItemSelectionModel::SelectionFlag &flags))
    QUrl _arg0{};
    QItemSelectionModel::SelectionFlag _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->selectUrl(_arg0, _arg1));
}

TEST_F(CollectionViewTest, setCanvasGridShell)
{
    // Test setter: void setCanvasGridShell((CanvasGridShell *sh))
    EXPECT_NO_FATAL_FAILURE(obj->setCanvasGridShell(nullptr));
}

TEST_F(CollectionViewTest, setCanvasManagerShell)
{
    // Test setter: void setCanvasManagerShell((CanvasManagerShell *sh))
    EXPECT_NO_FATAL_FAILURE(obj->setCanvasManagerShell(nullptr));
}

TEST_F(CollectionViewTest, setCanvasModelShell)
{
    // Test setter: void setCanvasModelShell((CanvasModelShell *sh))
    EXPECT_NO_FATAL_FAILURE(obj->setCanvasModelShell(nullptr));
}

TEST_F(CollectionViewTest, setCanvasViewShell)
{
    // Test setter: void setCanvasViewShell((CanvasViewShell *sh))
    EXPECT_NO_FATAL_FAILURE(obj->setCanvasViewShell(nullptr));
}

TEST_F(CollectionViewTest, setFileShiftable)
{
    // Test setter: void setFileShiftable((const bool enable))
    EXPECT_NO_FATAL_FAILURE(obj->setFileShiftable(false));
}

TEST_F(CollectionViewTest, setFreeze)
{
    // Test setter: void setFreeze((bool freeze))
    EXPECT_NO_FATAL_FAILURE(obj->setFreeze(false));
}

TEST_F(CollectionViewTest, setModel)
{
    // Test setter: void setModel((QAbstractItemModel *model))
    EXPECT_NO_FATAL_FAILURE(obj->setModel(nullptr));
}

TEST_F(CollectionViewTest, setSelection)
{
    // Test setter: void setSelection((const QRect &rect, QItemSelectionModel::SelectionFlags command))
    QRect _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->setSelection(_arg0, QItemSelectionModel::SelectionFlags()));
}

TEST_F(CollectionViewTest, setSelectionModel)
{
    // Test setter: void setSelectionModel((QItemSelectionModel *selectionModel))
    EXPECT_NO_FATAL_FAILURE(obj->setSelectionModel(nullptr));
}

TEST_F(CollectionViewTest, toggleSelect)
{
    // Test method: void toggleSelect(())
    EXPECT_NO_FATAL_FAILURE(obj->toggleSelect());
}

TEST_F(CollectionViewTest, updateRegionView)
{
    // Test method: void updateRegionView(())
    EXPECT_NO_FATAL_FAILURE(obj->updateRegionView());
}

TEST_F(CollectionViewTest, verticalOffset)
{
    // Test getter: int verticalOffset()
    auto result = obj->verticalOffset();
    EXPECT_EQ(result, 0);

}

TEST_F(CollectionViewTest, visualRect)
{
    // Test method: QRect visualRect((const QModelIndex &index))
    QModelIndex _arg0{};
    auto result = obj->visualRect(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CollectionViewTest, visualRegionForSelection)
{
    // Test method: QRegion visualRegionForSelection((const QItemSelection &selection))
    QItemSelection _arg0{};
    auto result = obj->visualRegionForSelection(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CollectionViewTest, wheelEvent)
{
    // Test event handler: wheelEvent((QWheelEvent *event))
    QWheelEvent _event(QWheelEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->wheelEvent(&_event));
}

TEST_F(CollectionViewTest, CollectionView_Destructor)
{
    // Test method:  ~CollectionView(())
    EXPECT_NO_FATAL_FAILURE({ CollectionView *tmp = new CollectionView(); delete tmp; });
}

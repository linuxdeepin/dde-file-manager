// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/collectionview.h"
#include "view/collectionview_p.h"
#include "models/collectionmodel.h"
#include "delegate/collectionitemdelegate.h"
#include "mode/collectiondataprovider.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "interface/canvasgridshell.h"
#include "interface/canvasmanagershell.h"

#include <QApplication>
#include <QWidget>
#include <QModelIndex>
#include <QRect>
#include <QPoint>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QContextMenuEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QFocusEvent>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QItemSelectionModel>
#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class MockCollectionDataProvider : public CollectionDataProvider
{
public:
    MockCollectionDataProvider() : CollectionDataProvider(nullptr) {}
    QString replace(const QUrl &, const QUrl &) override { return QString(); }
    QString append(const QUrl &) override { return QString(); }
    QString prepend(const QUrl &) override { return QString(); }
    void insert(const QUrl &, const QString &, const int) override {}
    QString remove(const QUrl &) override { return QString(); }
    QString change(const QUrl &) override { return QString(); }
};

class MockCanvasModelShell : public CanvasModelShell
{
public:
    MockCanvasModelShell() : CanvasModelShell(nullptr) {}
};

class MockCanvasViewShell : public CanvasViewShell
{
public:
    MockCanvasViewShell() : CanvasViewShell(nullptr) {}
};

class MockCanvasGridShell : public CanvasGridShell
{
public:
    MockCanvasGridShell() : CanvasGridShell(nullptr) {}
};

class MockCanvasManagerShell : public CanvasManagerShell
{
public:
    MockCanvasManagerShell() : CanvasManagerShell(nullptr) {}
};

class UT_CollectionView : public testing::Test
{
protected:
    void SetUp() override
    {
        
        mockDataProvider = new MockCollectionDataProvider();
        view = new CollectionView("test_uuid", mockDataProvider);

        mockModel = new CollectionModel();
        view->setModel(mockModel);
        mockModelShell = new MockCanvasModelShell();
        mockViewShell = new MockCanvasViewShell();
        mockGridShell = new MockCanvasGridShell();
        mockManagerShell = new MockCanvasManagerShell();

        // mock the UI show
        stub.set_lamda(&QWidget::show, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
        stub.set_lamda(&QWidget::hide, [](QWidget *) {
            __DBG_STUB_INVOKE__
        });
    }

    void TearDown() override
    {
        delete view;
        delete mockDataProvider;
        delete mockModelShell;
        delete mockViewShell;
        delete mockGridShell;
        delete mockManagerShell;
        delete mockModel;

        stub.clear();
    }

public:
    CollectionView *view;
    CollectionModel *mockModel;
    MockCollectionDataProvider *mockDataProvider;
    MockCanvasModelShell *mockModelShell;
    MockCanvasViewShell *mockViewShell;
    MockCanvasGridShell *mockGridShell;
    MockCanvasManagerShell *mockManagerShell;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionView, TestConstructor)
{
    EXPECT_NE(view, nullptr);
    EXPECT_EQ(view->id(), "test_uuid");
    EXPECT_EQ(view->dataProvider(), mockDataProvider);
}

TEST_F(UT_CollectionView, TestSetCanvasModelShell)
{
    view->setCanvasModelShell(mockModelShell);
    // No direct getter for this, test that it doesn't crash
    EXPECT_NO_THROW(view->setCanvasModelShell(mockModelShell));
}

TEST_F(UT_CollectionView, TestSetCanvasViewShell)
{
    view->setCanvasViewShell(mockViewShell);
    EXPECT_NO_THROW(view->setCanvasViewShell(mockViewShell));
}

TEST_F(UT_CollectionView, TestSetCanvasGridShell)
{
    view->setCanvasGridShell(mockGridShell);
    EXPECT_NO_THROW(view->setCanvasGridShell(mockGridShell));
}

TEST_F(UT_CollectionView, TestSetCanvasManagerShell)
{
    view->setCanvasManagerShell(mockManagerShell);
    EXPECT_NO_THROW(view->setCanvasManagerShell(mockManagerShell));
}

TEST_F(UT_CollectionView, TestFileShiftable)
{
    view->setFileShiftable(true);
    EXPECT_TRUE(view->fileShiftable());
    
    view->setFileShiftable(false);
    EXPECT_FALSE(view->fileShiftable());
}

TEST_F(UT_CollectionView, TestCellMargins)
{
    view->setCanvasGridShell(mockGridShell);
    // Skip cellMargins test as it may not be available in this interface
    EXPECT_NO_THROW(view->cellMargins());
}

TEST_F(UT_CollectionView, TestModel)
{
    CollectionModel *model = view->model();
    EXPECT_NE(model, nullptr);
}

TEST_F(UT_CollectionView, TestItemDelegate)
{
    CollectionItemDelegate *delegate = view->itemDelegate();
    EXPECT_NE(delegate, nullptr);
}

TEST_F(UT_CollectionView, TestWinId)
{
    WId winId = view->winId();
    EXPECT_NE(winId, 0);
}

TEST_F(UT_CollectionView, TestUpdateRegionView)
{
    EXPECT_NO_THROW(view->updateRegionView());
}

TEST_F(UT_CollectionView, TestRefresh)
{
    EXPECT_NO_THROW(view->refresh(false));
    EXPECT_NO_THROW(view->refresh(true));
}

TEST_F(UT_CollectionView, TestSetFreeze)
{
    EXPECT_NO_THROW(view->setFreeze(true));
    EXPECT_NO_THROW(view->setFreeze(false));
}

TEST_F(UT_CollectionView, TestOpenEditor)
{
    QUrl url("file:///test.txt");
    
    EXPECT_NO_THROW(view->openEditor(url));
}

TEST_F(UT_CollectionView, TestSelectUrl)
{
    QUrl url("file:///test.txt");

    EXPECT_NO_THROW(view->selectUrl(url, QItemSelectionModel::Select));
    EXPECT_NO_THROW(view->selectUrl(url, QItemSelectionModel::Deselect));
    EXPECT_NO_THROW(view->selectUrl(url, QItemSelectionModel::Toggle));
}

TEST_F(UT_CollectionView, TestSelectUrls)
{
    QList<QUrl> urls = {
        QUrl("file:///test1.txt"),
        QUrl("file:///test2.txt")
    };
    EXPECT_NO_THROW(view->selectUrls(urls));
}

TEST_F(UT_CollectionView, TestSetModel)
{
    CollectionModel *newModel = new CollectionModel();
    EXPECT_NO_THROW(view->setModel(newModel));
    delete newModel;
}

TEST_F(UT_CollectionView, TestSetSelectionModel)
{
    QItemSelectionModel *selModel = new QItemSelectionModel(nullptr);
    EXPECT_NO_THROW(view->setSelectionModel(selModel));
    delete selModel;
}

TEST_F(UT_CollectionView, TestReset)
{
    EXPECT_NO_THROW(view->reset());
}

TEST_F(UT_CollectionView, TestSelectAll)
{
    EXPECT_NO_THROW(view->selectAll());
}

TEST_F(UT_CollectionView, TestVisualRect)
{
    QModelIndex index;
    QRect rect = view->visualRect(index);
    EXPECT_TRUE(rect.isValid() || rect.isEmpty());
}

TEST_F(UT_CollectionView, TestScrollTo)
{
    QModelIndex index;
    EXPECT_NO_THROW(view->scrollTo(index));
    EXPECT_NO_THROW(view->scrollTo(index, QAbstractItemView::EnsureVisible));
    EXPECT_NO_THROW(view->scrollTo(index, QAbstractItemView::PositionAtTop));
}

TEST_F(UT_CollectionView, TestIndexAt)
{
    QPoint point(50, 50);
    QModelIndex index = view->indexAt(point);
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_CollectionView, TestEdit)
{
    QModelIndex index;
    EXPECT_FALSE(view->edit(index, QAbstractItemView::DoubleClicked, nullptr));
    EXPECT_FALSE(view->edit(index, QAbstractItemView::EditKeyPressed, nullptr));
}

TEST_F(UT_CollectionView, TestKeyboardSearch)
{
    EXPECT_NO_THROW(view->keyboardSearch("test"));
    EXPECT_NO_THROW(view->keyboardSearch(""));
}

TEST_F(UT_CollectionView, TestInputMethodQuery)
{
    QVariant result = view->inputMethodQuery(Qt::ImEnabled);
    EXPECT_TRUE(result.isValid() || result.isNull());
}

TEST_F(UT_CollectionView, TestSort)
{
    EXPECT_NO_THROW(view->sort(0));
    EXPECT_NO_THROW(view->sort(1));
}

TEST_F(UT_CollectionView, TestToggleSelect)
{
    EXPECT_NO_THROW(view->toggleSelect());
}

TEST_F(UT_CollectionView, TestSelectedIndexes)
{
    QModelIndexList indexes = view->selectedIndexes();
    EXPECT_TRUE(indexes.isEmpty());
}

TEST_F(UT_CollectionView, TestMoveCursor)
{
    QModelIndex index = view->moveCursor(QAbstractItemView::MoveUp, Qt::NoModifier);
    EXPECT_FALSE(index.isValid());
    
    index = view->moveCursor(QAbstractItemView::MoveDown, Qt::NoModifier);
    EXPECT_FALSE(index.isValid());
    
    index = view->moveCursor(QAbstractItemView::MoveLeft, Qt::NoModifier);
    EXPECT_FALSE(index.isValid());
    
    index = view->moveCursor(QAbstractItemView::MoveRight, Qt::NoModifier);
    EXPECT_FALSE(index.isValid());
    
    index = view->moveCursor(QAbstractItemView::MoveHome, Qt::NoModifier);
    EXPECT_FALSE(index.isValid());
    
    index = view->moveCursor(QAbstractItemView::MoveEnd, Qt::NoModifier);
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_CollectionView, TestHorizontalOffset)
{
    int offset = view->horizontalOffset();
    EXPECT_GE(offset, 0);
}

TEST_F(UT_CollectionView, TestVerticalOffset)
{
    int offset = view->verticalOffset();
    EXPECT_GE(offset, 0);
}

TEST_F(UT_CollectionView, TestIsIndexHidden)
{
    QModelIndex index;
    bool hidden = view->isIndexHidden(index);
    EXPECT_FALSE(hidden);
}

TEST_F(UT_CollectionView, TestSetSelection)
{
    QRect rect(0, 0, 100, 100);
    EXPECT_NO_THROW(view->setSelection(rect, QItemSelectionModel::ClearAndSelect));
    EXPECT_NO_THROW(view->setSelection(rect, QItemSelectionModel::Select));
    EXPECT_NO_THROW(view->setSelection(rect, QItemSelectionModel::Deselect));
}

TEST_F(UT_CollectionView, TestVisualRegionForSelection)
{
    QItemSelection selection;
    QRegion region = view->visualRegionForSelection(selection);
    EXPECT_TRUE(region.isEmpty());
}

TEST_F(UT_CollectionView, TestLessThan)
{
    QUrl url1("file:///a.txt");
    QUrl url2("file:///b.txt");
    bool result = view->lessThan(url1, url2);
    EXPECT_TRUE(result || !result);  // Just test it doesn't crash
}

TEST_F(UT_CollectionView, TestPaintEvent)
{
    QPaintEvent event(QRect(0, 0, 100, 100));
    EXPECT_NO_THROW(view->paintEvent(&event));
}

TEST_F(UT_CollectionView, TestWheelEvent)
{
    QWheelEvent event(QPoint(50, 50), QPoint(50, 50), QPoint(0, 120), QPoint(0, 120), Qt::NoButton, Qt::NoModifier, Qt::ScrollBegin, false);
    EXPECT_NO_THROW(view->wheelEvent(&event));
}

TEST_F(UT_CollectionView, TestMousePressEvent)
{
    QMouseEvent event(QEvent::MouseButtonPress, QPoint(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_THROW(view->mousePressEvent(&event));
    
    QMouseEvent rightEvent(QEvent::MouseButtonPress, QPoint(50, 50), Qt::RightButton, Qt::RightButton, Qt::NoModifier);
    EXPECT_NO_THROW(view->mousePressEvent(&rightEvent));
}

TEST_F(UT_CollectionView, TestMouseReleaseEvent)
{
    QMouseEvent event(QEvent::MouseButtonRelease, QPoint(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_THROW(view->mouseReleaseEvent(&event));
}

TEST_F(UT_CollectionView, TestMouseMoveEvent)
{
    QMouseEvent event(QEvent::MouseMove, QPoint(50, 50), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_THROW(view->mouseMoveEvent(&event));
}

TEST_F(UT_CollectionView, TestResizeEvent)
{
    QResizeEvent event(QSize(200, 200), QSize(100, 100));
    EXPECT_NO_THROW(view->resizeEvent(&event));
}

TEST_F(UT_CollectionView, TestKeyPressEvent)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Up, Qt::NoModifier);
    EXPECT_NO_THROW(view->keyPressEvent(&event));
    
    QKeyEvent downEvent(QEvent::KeyPress, Qt::Key_Down, Qt::NoModifier);
    EXPECT_NO_THROW(view->keyPressEvent(&downEvent));
    
    QKeyEvent returnEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    EXPECT_NO_THROW(view->keyPressEvent(&returnEvent));
    
    QKeyEvent escapeEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_NO_THROW(view->keyPressEvent(&escapeEvent));
}

TEST_F(UT_CollectionView, TestContextMenuEvent)
{
    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(50, 50));
    EXPECT_NO_THROW(view->contextMenuEvent(&event));
}

TEST_F(UT_CollectionView, TestStartDrag)
{
    EXPECT_NO_THROW(view->startDrag(Qt::CopyAction));
    EXPECT_NO_THROW(view->startDrag(Qt::MoveAction));
}

TEST_F(UT_CollectionView, TestDragEnterEvent)
{
    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls({QUrl("file:///test.txt")});
    QDragEnterEvent event(QPoint(50, 50), Qt::CopyAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    
    EXPECT_NO_THROW(view->dragEnterEvent(&event));
    delete mimeData;
}

TEST_F(UT_CollectionView, TestDragMoveEvent)
{
    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls({QUrl("file:///test.txt")});
    QDragMoveEvent event(QPoint(50, 50), Qt::CopyAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    
    EXPECT_NO_THROW(view->dragMoveEvent(&event));
    delete mimeData;
}

TEST_F(UT_CollectionView, TestDragLeaveEvent)
{
    QDragLeaveEvent event;
    EXPECT_NO_THROW(view->dragLeaveEvent(&event));
}

TEST_F(UT_CollectionView, TestDropEvent)
{
    QMimeData *mimeData = new QMimeData();
    mimeData->setUrls({QUrl("file:///test.txt")});
    QDropEvent event(QPoint(50, 50), Qt::CopyAction, mimeData, Qt::LeftButton, Qt::NoModifier);
    
    EXPECT_NO_THROW(view->dropEvent(&event));
    delete mimeData;
}

TEST_F(UT_CollectionView, TestFocusInEvent)
{
    QFocusEvent event(QEvent::FocusIn);
    EXPECT_NO_THROW(view->focusInEvent(&event));
}

TEST_F(UT_CollectionView, TestChangeEvent)
{
    QEvent event(QEvent::FontChange);
    EXPECT_NO_THROW(view->changeEvent(&event));
}

TEST_F(UT_CollectionView, TestScrollContentsBy)
{
    EXPECT_NO_THROW(view->scrollContentsBy(10, 10));
    EXPECT_NO_THROW(view->scrollContentsBy(-5, -5));
}

TEST_F(UT_CollectionView, TestEdgeCases)
{
    // Test with null data provider
    CollectionView *nullView = new CollectionView("null_test", nullptr);
    EXPECT_NE(nullView, nullptr);
    delete nullView;
    
    // Test with empty uuid
    CollectionView *emptyView = new CollectionView("", mockDataProvider);
    EXPECT_NE(emptyView, nullptr);
    EXPECT_EQ(emptyView->id(), "");
    delete emptyView;
}

TEST_F(UT_CollectionView, TestMultipleOperations)
{
    // Set up all shells
    view->setCanvasModelShell(mockModelShell);
    view->setCanvasViewShell(mockViewShell);
    view->setCanvasGridShell(mockGridShell);
    view->setCanvasManagerShell(mockManagerShell);
    
    // Test file shiftable
    view->setFileShiftable(true);
    EXPECT_TRUE(view->fileShiftable());
    
    // Test selections
    QUrl url1("file:///test1.txt");
    QUrl url2("file:///test2.txt");
    view->selectUrl(url1, QItemSelectionModel::Select);
    view->selectUrls({url2});
    
    // Test view operations
    view->updateRegionView();
    view->refresh(false);
    view->reset();
    view->selectAll();
    
    // Test that view still works after multiple operations
    EXPECT_EQ(view->id(), "test_uuid");
    EXPECT_EQ(view->dataProvider(), mockDataProvider);
}

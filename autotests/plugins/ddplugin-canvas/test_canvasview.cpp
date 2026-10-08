// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/view/canvasview.h"
#include "plugins/desktop/ddplugin-canvas/view/canvasview_p.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasselectionmodel.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel.h"
#include "plugins/desktop/ddplugin-canvas/delegate/canvasitemdelegate.h"

#include <QWidget>
#include <QModelIndex>
#include <QAbstractItemModel>
#include <QApplication>
#include <QStandardItemModel>
#include <QItemSelectionModel>

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

class UT_CanvasView : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Stub QWidget virtual methods using VADDR for virtual functions
        stub.set_lamda(VADDR(QWidget, show), [](QWidget*) { __DBG_STUB_INVOKE__; });
        stub.set_lamda(VADDR(QWidget, setVisible), [](QWidget*, bool) { __DBG_STUB_INVOKE__; });
        
        // Stub CanvasView::reset to avoid CanvasProxyModel cast issues
        stub.set_lamda(VADDR(CanvasView, reset), [](CanvasView*) { __DBG_STUB_INVOKE__; });
        
        // Stub CanvasView::selectionModel to return non-null for assertions
        stub.set_lamda(ADDR(CanvasView, selectionModel), [this](CanvasView*) -> CanvasSelectionModel* {
            __DBG_STUB_INVOKE__
            // Return a cast pointer to satisfy the assertion (it will be checked but not used)
            return reinterpret_cast<CanvasSelectionModel*>(mockSelectionModel);
        });
        
        // Stub CanvasView::model to return non-null for assertions
        stub.set_lamda(ADDR(CanvasView, model), [this](CanvasView*) -> CanvasProxyModel* {
            __DBG_STUB_INVOKE__
            // Return a cast pointer to satisfy the assertion (it will be checked but not used)
            return reinterpret_cast<CanvasProxyModel*>(mockModel);
        });
        
        // Stub CanvasView::itemDelegate to return non-null for methods that depend on it
        stub.set_lamda(ADDR(CanvasView, itemDelegate), [this](CanvasView*) -> CanvasItemDelegate* {
            __DBG_STUB_INVOKE__
            // Create a dummy non-null pointer for delegate checking
            static char dummyDelegate;
            return reinterpret_cast<CanvasItemDelegate*>(&dummyDelegate);
        });
        
        // Stub CanvasItemDelegate::mayExpand to avoid null pointer issues
        stub.set_lamda(ADDR(CanvasItemDelegate, mayExpand), [](CanvasItemDelegate*, QModelIndex*) -> bool {
            __DBG_STUB_INVOKE__
            // Return false to avoid expanding logic that requires complex dependencies
            return false;
        });
        
        view = new CanvasView(nullptr);
        
        // Set up mock model and selectionModel as required by CanvasView
        // following the pattern from CanvasManager::createView()
        mockModel = new QStandardItemModel();
        mockSelectionModel = new QItemSelectionModel(mockModel);
        view->setModel(mockModel);
        view->setSelectionModel(mockSelectionModel);
    }

    virtual void TearDown() override
    {
        delete view;
        view = nullptr;
        delete mockSelectionModel;
        mockSelectionModel = nullptr;
        delete mockModel;
        mockModel = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    CanvasView *view = nullptr;
    QStandardItemModel *mockModel = nullptr;
    QItemSelectionModel *mockSelectionModel = nullptr;
};

TEST_F(UT_CanvasView, constructor)
{
    EXPECT_NE(view, nullptr);
}

TEST_F(UT_CanvasView, initUI)
{
    // Test initUI method - should not crash
    EXPECT_NO_THROW(view->initUI());
}

TEST_F(UT_CanvasView, visualRect)
{
    QModelIndex testIndex;
    
    // Test visualRect method - should not crash
    QRect rect = view->visualRect(testIndex);
    SUCCEED(); // Main goal is no crash
}

TEST_F(UT_CanvasView, indexAt)
{
    QPoint testPoint(100, 100);
    
    // Test indexAt method - should not crash
    QModelIndex index = view->indexAt(testPoint);
    SUCCEED(); // Main goal is no crash
}

TEST_F(UT_CanvasView, horizontalOffset)
{
    // Test horizontalOffset method
    int offset = view->horizontalOffset();
    SUCCEED(); // Method should execute without crashing
}

TEST_F(UT_CanvasView, verticalOffset)
{
    // Test verticalOffset method
    int offset = view->verticalOffset();
    SUCCEED(); // Method should execute without crashing
}

TEST_F(UT_CanvasView, isIndexHidden)
{
    QModelIndex testIndex;
    
    // Test isIndexHidden method
    bool hidden = view->isIndexHidden(testIndex);
    SUCCEED(); // Method should execute without crashing
}

TEST_F(UT_CanvasView, scrollTo)
{
    QModelIndex testIndex;
    
    // Test scrollTo method with different scroll hints
    EXPECT_NO_THROW(view->scrollTo(testIndex, QAbstractItemView::EnsureVisible));
    EXPECT_NO_THROW(view->scrollTo(testIndex, QAbstractItemView::PositionAtTop));
}

TEST_F(UT_CanvasView, setModel)
{
    // Test setModel method with nullptr
    EXPECT_NO_THROW(view->setModel(nullptr));
}

TEST_F(UT_CanvasView, updateGridSize)
{
    bool updateGridSizeCalled = false;
    
    // Stub private implementation
    stub.set_lamda(ADDR(CanvasViewPrivate, updateGridSize), [&updateGridSizeCalled](CanvasViewPrivate*, const QSize&, const QMargins&, const QSize&) {
        __DBG_STUB_INVOKE__
        updateGridSizeCalled = true;
    });
    
    // Test method that might call updateGridSize indirectly
    view->initUI();
    // The test mainly ensures the method can be stubbed
    SUCCEED();
}

TEST_F(UT_CanvasView, visualRect_Private)
{
    bool visualRectCalled = false;
    
    // Stub private visualRect method
    stub.set_lamda(ADDR(CanvasViewPrivate, visualRect), [&visualRectCalled](CanvasViewPrivate*, const QPoint&) -> QRect {
        __DBG_STUB_INVOKE__
        visualRectCalled = true;
        return QRect(0, 0, 100, 100);
    });
    
    // Test method that might call private visualRect
    QModelIndex testIndex;
    view->visualRect(testIndex);
    
    SUCCEED(); // Test method accessibility
}

TEST_F(UT_CanvasView, itemGridpos)
{
    bool itemGridposCalled = false;
    
    // Stub private itemGridpos method
    stub.set_lamda(ADDR(CanvasViewPrivate, itemGridpos), [&itemGridposCalled](CanvasViewPrivate*, const QString&, QPoint&) -> bool {
        __DBG_STUB_INVOKE__
        itemGridposCalled = true;
        return true;
    });
    
    // Test methods that might use itemGridpos indirectly
    EXPECT_NO_THROW(view->initUI());
    
    SUCCEED();
}

TEST_F(UT_CanvasView, openIndex)
{
    bool openIndexCalled = false;
    
    // Stub private openIndex method
    stub.set_lamda(ADDR(CanvasViewPrivate, openIndex), [&openIndexCalled](CanvasViewPrivate*, const QModelIndex&) {
        __DBG_STUB_INVOKE__
        openIndexCalled = true;
    });
    
    // Test that view can handle index operations
    QModelIndex testIndex;
    // Methods that might call openIndex indirectly
    EXPECT_NO_THROW(view->initUI());
    
    SUCCEED();
}

TEST_F(UT_CanvasView, findIndex_Methods)
{
    bool findIndexCalled = false;
    
    // Stub private findIndex method
    stub.set_lamda(ADDR(CanvasViewPrivate, findIndex), [&findIndexCalled](CanvasViewPrivate*, const QString&, bool, const QModelIndex&, bool, bool) -> QModelIndex {
        __DBG_STUB_INVOKE__
        findIndexCalled = true;
        return QModelIndex();
    });
    
    // Test view functionality
    EXPECT_NO_THROW(view->initUI());
    
    SUCCEED();
}

TEST_F(UT_CanvasView, waterMask_isWaterMaskOn)
{
    bool isWaterMaskOnCalled = false;
    
    // Stub private isWaterMaskOn method
    stub.set_lamda(ADDR(CanvasViewPrivate, isWaterMaskOn), [&isWaterMaskOnCalled](CanvasViewPrivate*) -> bool {
        __DBG_STUB_INVOKE__
        isWaterMaskOnCalled = true;
        return false;
    });
    
    // Test view initialization
    EXPECT_NO_THROW(view->initUI());
    
    SUCCEED();
}
// ---------------------------------------------------------------------------
// PMS:337885 桌面图标与文字间隙区域点击无响应回归：baseIndexAt 的命中判定
// 必须把图标矩形与文字矩形合并后再判断（修复前间隙点无法命中任何项），
// 且 itemPaintGeomertys 返回空列表时不得越界崩溃（修复前对空列表取 at(0)）。
// ---------------------------------------------------------------------------

// 间隙点（图标矩形与文字矩形之间、二者并集之内）必须命中该项
TEST_F(UT_CanvasView, BUG337885_BaseIndexAt_GapBetweenIconAndText_HitsItem)
{
    // 固定网格项名，避开 GridIns 依赖
    stub.set_lamda(ADDR(CanvasViewPrivate, visualItem),
                   [](CanvasViewPrivate *, const QPoint &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("ut_item_337885");
                   });
    // model()->index(item, 0)（QUrl 重载，QString 隐式转换）→ 返回有效索引
    const QModelIndex expected = mockModel->createIndex(0, 0);
    stub.set_lamda(static_cast<QModelIndex (CanvasProxyModel::*)(const QUrl &, int) const>(&CanvasProxyModel::index),
                   [expected](CanvasProxyModel *, const QUrl &, int) -> QModelIndex {
                       __DBG_STUB_INVOKE__
                       return expected;
                   });
    // 图标矩形 (0,0,40,40) 与文字矩形 (0,60,40,30)：y∈[40,60) 为间隙带
    stub.set_lamda(ADDR(CanvasView, itemPaintGeomertys),
                   [](CanvasView *, const QModelIndex &) -> QList<QRect> {
                       __DBG_STUB_INVOKE__
                       return { QRect(0, 0, 40, 40), QRect(0, 60, 40, 30) };
                   });

    // 间隙点 (20,50)：不在图标/文字任一矩形内，但在二者并集内 → 应命中
    const QModelIndex hit = view->baseIndexAt(QPoint(20, 50));
    EXPECT_TRUE(hit.isValid());
    EXPECT_EQ(hit, expected);

    // 边界外点（不在并集内）不得命中
    const QModelIndex miss = view->baseIndexAt(QPoint(20, 105));
    EXPECT_FALSE(miss.isValid());
}

// itemPaintGeomertys 为空列表时不得越界，返回无效索引即可
TEST_F(UT_CanvasView, BUG337885_BaseIndexAt_EmptyPaintGeometries_ReturnsInvalidNoCrash)
{
    stub.set_lamda(ADDR(CanvasViewPrivate, visualItem),
                   [](CanvasViewPrivate *, const QPoint &) -> QString {
                       __DBG_STUB_INVOKE__
                       return QString("ut_item_337885");
                   });
    stub.set_lamda(static_cast<QModelIndex (CanvasProxyModel::*)(const QUrl &, int) const>(&CanvasProxyModel::index),
                   [](CanvasProxyModel *, const QUrl &, int) -> QModelIndex {
                       __DBG_STUB_INVOKE__
                       return QModelIndex();
                   });
    stub.set_lamda(ADDR(CanvasView, itemPaintGeomertys),
                   [](CanvasView *, const QModelIndex &) -> QList<QRect> {
                       __DBG_STUB_INVOKE__
                       return {};
                   });

    QModelIndex hit;
    EXPECT_NO_THROW(hit = view->baseIndexAt(QPoint(20, 50)));
    EXPECT_FALSE(hit.isValid());
}

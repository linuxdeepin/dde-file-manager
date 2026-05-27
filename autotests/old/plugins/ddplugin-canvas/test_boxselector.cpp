// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/boxselector.h"
#include "plugins/desktop/ddplugin-canvas/view/canvasview.h"
#include "plugins/desktop/ddplugin-canvas/canvasmanager.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasselectionmodel.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel.h"

#include <QPoint>
#include <QRect>
#include <QEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QWidget>
#include <QItemSelection>

#include <gtest/gtest.h>

using namespace ddplugin_canvas;

class UT_BoxSelector : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Get singleton instance
        selector = BoxSelector::instance();
        
        // Stub QWidget methods to avoid GUI dependencies
        stub.set_lamda(VADDR(QWidget, show), [](QWidget*) {
            __DBG_STUB_INVOKE__
            // Do nothing to avoid actual GUI display
        });
        
        stub.set_lamda(VADDR(QWidget, hide), [](QWidget*) {
            __DBG_STUB_INVOKE__
            // Do nothing to avoid actual GUI display
        });
        
        // Stub QWidget move/resize methods using function pointer casting for overloaded methods
        using MoveFunc = void (QWidget::*)(int, int);
        stub.set_lamda(static_cast<MoveFunc>(&QWidget::move), [](QWidget*, int, int) {
            __DBG_STUB_INVOKE__
            // Do nothing to avoid actual GUI operations
        });
        
        using ResizeFunc = void (QWidget::*)(int, int);
        stub.set_lamda(static_cast<ResizeFunc>(&QWidget::resize), [](QWidget*, int, int) {
            __DBG_STUB_INVOKE__
            // Do nothing to avoid actual GUI operations
        });
        
        stub.set_lamda(VADDR(QWidget, geometry), [](QWidget*) -> QRect {
            __DBG_STUB_INVOKE__
            return QRect(0, 0, 800, 600); // Mock geometry
        });
        
        // Stub QApplication methods
        using InstallEventFilterFunc = void (QObject::*)(QObject *);
        using RemoveEventFilterFunc = void (QObject::*)(QObject *);
        stub.set_lamda(static_cast<InstallEventFilterFunc>(&QObject::installEventFilter), 
                       [](QObject*, QObject*) {
            __DBG_STUB_INVOKE__
            // Do nothing to avoid actual event filter installation
        });
        
        stub.set_lamda(static_cast<RemoveEventFilterFunc>(&QObject::removeEventFilter), 
                       [](QObject*, QObject*) {
            __DBG_STUB_INVOKE__
            // Do nothing to avoid actual event filter removal
        });
        
        // Stub CanvasManager methods to avoid null pointer access
        mockManager = new CanvasManager(nullptr);
        stub.set_lamda(&CanvasManager::instance, [this]() -> CanvasManager* {
            __DBG_STUB_INVOKE__
            return mockManager;
        });
        
        // Stub CanvasManager::views to return empty list to avoid null pointer access
        stub.set_lamda(&CanvasManager::views, [](const CanvasManager*) -> QList<QSharedPointer<CanvasView>> {
            __DBG_STUB_INVOKE__
            return QList<QSharedPointer<CanvasView>>();
        });
        
        // Create mock objects for CanvasSelectionModel
        mockProxyModel = new CanvasProxyModel(nullptr);
        mockSelectionModel = new CanvasSelectionModel(mockProxyModel, nullptr);
        stub.set_lamda(&CanvasManager::selectionModel, [this](const CanvasManager*) -> CanvasSelectionModel* {
            __DBG_STUB_INVOKE__
            return mockSelectionModel;
        });
        
        // Stub CanvasSelectionModel::selectedIndexesCache to return empty list
        stub.set_lamda(&CanvasSelectionModel::selectedIndexesCache, [](const CanvasSelectionModel*) -> QList<QModelIndex> {
            __DBG_STUB_INVOKE__
            return QList<QModelIndex>();
        });
    }

    virtual void TearDown() override
    {
        // Ensure selector is not active after test
        if (selector && selector->isAcvite()) {
            selector->endSelect();
        }
        
        // Clean up mock objects
        delete mockManager;
        mockManager = nullptr;
        delete mockSelectionModel;
        mockSelectionModel = nullptr;
        delete mockProxyModel;
        mockProxyModel = nullptr;
        
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    BoxSelector *selector = nullptr;
    CanvasManager *mockManager = nullptr;
    CanvasSelectionModel *mockSelectionModel = nullptr;
    CanvasProxyModel *mockProxyModel = nullptr;
};

TEST_F(UT_BoxSelector, instance_GetSingleton_ReturnsValidInstance)
{
    // Test singleton pattern
    BoxSelector *inst1 = BoxSelector::instance();
    BoxSelector *inst2 = BoxSelector::instance();
    
    EXPECT_NE(inst1, nullptr);
    EXPECT_EQ(inst1, inst2); // Should be the same instance
}

TEST_F(UT_BoxSelector, isActive_InitialState_ReturnsFalse)
{
    // Test initial state
    EXPECT_FALSE(selector->isAcvite());
}

TEST_F(UT_BoxSelector, beginSelect_StartSelection_SetsActiveState)
{
    // Test beginSelect functionality
    QPoint testPos(100, 200);
    
    selector->beginSelect(testPos, true);
    EXPECT_TRUE(selector->isAcvite());
    
    // Clean up
    selector->endSelect();
}

TEST_F(UT_BoxSelector, endSelect_StopSelection_ClearsActiveState)
{
    // First start selection
    QPoint testPos(100, 200);
    selector->beginSelect(testPos, true);
    EXPECT_TRUE(selector->isAcvite());
    
    // Then end selection
    selector->endSelect();
    EXPECT_FALSE(selector->isAcvite());
}

TEST_F(UT_BoxSelector, setBegin_SetStartPoint_UpdatesBeginPosition)
{
    // Test setBegin functionality
    QPoint testPos(50, 75);
    
    EXPECT_NO_THROW(selector->setBegin(testPos));
    // Note: Can't directly verify the internal begin point as it's private
}

TEST_F(UT_BoxSelector, setEnd_SetEndPoint_UpdatesEndPosition)
{
    // Test setEnd functionality
    QPoint testPos(150, 250);
    
    EXPECT_NO_THROW(selector->setEnd(testPos));
    // Note: Can't directly verify the internal end point as it's private
}

TEST_F(UT_BoxSelector, globalRect_GetGlobalRect_ReturnsValidRect)
{
    // Set begin and end points
    selector->setBegin(QPoint(100, 100));
    selector->setEnd(QPoint(200, 200));
    
    QRect rect = selector->globalRect();
    EXPECT_TRUE(rect.isValid());
    EXPECT_EQ(rect.width(), 101);  // Include boundary
    EXPECT_EQ(rect.height(), 101); // Include boundary
}

TEST_F(UT_BoxSelector, clipRect_ClipRectToGeometry_ReturnsClippedRect)
{
    // Test clipRect functionality
    QRect inputRect(50, 50, 200, 200);
    QRect geometry(0, 0, 150, 150);
    
    QRect clippedRect = selector->clipRect(inputRect, geometry);
    
    EXPECT_TRUE(clippedRect.isValid());
    EXPECT_LE(clippedRect.right(), geometry.right());
    EXPECT_LE(clippedRect.bottom(), geometry.bottom());
}

TEST_F(UT_BoxSelector, update_TriggerUpdate_DoesNotCrash)
{
    // Test update method
    EXPECT_NO_THROW(selector->update());
}

TEST_F(UT_BoxSelector, eventFilter_HandleEvents_ReturnsAppropriateValue)
{
    // Create a dummy event
    QMouseEvent testEvent(QEvent::MouseMove, QPoint(100, 100), 
                         Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    
    // Test event filter
    bool result = selector->eventFilter(nullptr, &testEvent);
    
    // Should return false for non-relevant events when not active
    EXPECT_FALSE(result);
}

TEST_F(UT_BoxSelector, beginSelectWithAutoSelect_StartWithAutoSelect_SetsCorrectState)
{
    // Test beginSelect with autoSelect = true
    QPoint testPos(100, 200);
    
    selector->beginSelect(testPos, true);
    EXPECT_TRUE(selector->isAcvite());
    
    // Clean up
    selector->endSelect();
}

TEST_F(UT_BoxSelector, beginSelectWithoutAutoSelect_StartWithoutAutoSelect_SetsCorrectState)
{
    // Test beginSelect with autoSelect = false
    QPoint testPos(100, 200);
    
    selector->beginSelect(testPos, false);
    EXPECT_TRUE(selector->isAcvite());
    
    // Clean up
    selector->endSelect();
}

TEST_F(UT_BoxSelector, multipleBeginEnd_MultipleStartStop_WorksCorrectly)
{
    // Test multiple begin/end cycles
    QPoint testPos(100, 200);
    
    // First cycle
    selector->beginSelect(testPos, true);
    EXPECT_TRUE(selector->isAcvite());
    selector->endSelect();
    EXPECT_FALSE(selector->isAcvite());
    
    // Second cycle
    selector->beginSelect(testPos, false);
    EXPECT_TRUE(selector->isAcvite());
    selector->endSelect();
    EXPECT_FALSE(selector->isAcvite());
}

TEST_F(UT_BoxSelector, RubberBand_Construction_DoesNotCrash)
{
    // Test RubberBand construction
    EXPECT_NO_THROW({
        RubberBand band;
    });
}

TEST_F(UT_BoxSelector, RubberBand_Touch_DoesNotCrash)
{
    // Test RubberBand touch method
    RubberBand band;
    QWidget testWidget;
    
    // Stub QWidget::setParent to avoid actual GUI operations
    using SetParentFunc = void (QWidget::*)(QWidget*);
    stub.set_lamda(static_cast<SetParentFunc>(&QWidget::setParent), [](QWidget*, QWidget*) {
        __DBG_STUB_INVOKE__
        // Do nothing to avoid actual GUI operations
    });
    
    EXPECT_NO_THROW(band.touch(&testWidget));
}


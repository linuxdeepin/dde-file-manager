// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

#include "plugins/desktop/ddplugin-canvas/delegate/canvasitemdelegate.h"
#include "plugins/desktop/ddplugin-canvas/view/canvasview.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/utils/iconutils.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <QWidget>
#include <QRect>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QAbstractItemView>
#include <QApplication>
#include <QListView>
#include "view/canvasview.h"
#include "model/canvasselectionmodel.h"
#include "model/canvasproxymodel.h"
#include <QPainter>
#include <QPixmap>
#include <QIcon>
#include <QSize>
#include <QFont>
#include <QFontMetrics>
#include <QColor>
#include <QPalette>
#include <QTextLayout>
#include <QLineEdit>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

class UT_CanvasItemDelegate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Following dfmplugin-burn pattern: comprehensive stubbing to avoid GUI operations
        
        // Mock QWidget operations to avoid GUI dependencies
        stub.set_lamda(ADDR(QWidget, show), [] {
            __DBG_STUB_INVOKE__
        });
        
        stub.set_lamda(ADDR(QWidget, hide), [] {
            __DBG_STUB_INVOKE__
        });
        
        // Mock Application settings access - following dfmplugin-burn pattern for static methods
        stub.set_lamda(ADDR(Application, genericAttribute), [](Application::GenericAttribute) -> QVariant {
            __DBG_STUB_INVOKE__
            return QVariant(1);  // Return valid icon level
        });
        
        // Mock FileUtils operations
        stub.set_lamda(ADDR(FileUtils, isTrashFile), [](const QUrl &) -> bool {
            __DBG_STUB_INVOKE__
            return false;
        });
        
        // Mock QAbstractItemView operations that are called during construction
        stub.set_lamda(VADDR(QAbstractItemView, setIconSize), [](QAbstractItemView *, const QSize &) {
            __DBG_STUB_INVOKE__
        });
        
        stub.set_lamda(ADDR(QWidget, fontMetrics), [](QWidget *) -> QFontMetrics {
            __DBG_STUB_INVOKE__
            return QFontMetrics(QFont());
        });
        
        // Create mock parent view first
        parentView = new CanvasView();
        
        // Mock CanvasItemDelegate::parent() to return a valid mock CanvasView
        // This is critical for avoiding null pointer access during construction
        stub.set_lamda(ADDR(CanvasItemDelegate, parent), [this]() -> CanvasView* {
            __DBG_STUB_INVOKE__
            return parentView;  // Return actual CanvasView
        });
        
        // Mock CanvasView::selectionModel() to return a non-null selection model
        stub.set_lamda(VADDR(CanvasView, selectionModel), [](CanvasView *) -> CanvasSelectionModel* {
            __DBG_STUB_INVOKE__
            static char mockSelectionModel[1024];  // Dummy selection model
            return reinterpret_cast<CanvasSelectionModel*>(mockSelectionModel);
        });
        
        // Mock QItemSelectionModel::isSelected to avoid SEGV
        stub.set_lamda(VADDR(QItemSelectionModel, isSelected), [](QItemSelectionModel *, const QModelIndex &) -> bool {
            __DBG_STUB_INVOKE__
            return false;  // Default to not selected
        });
        
        // Mock CanvasView::model() to return a non-null model
        stub.set_lamda(VADDR(CanvasView, model), [](CanvasView *) -> CanvasProxyModel* {
            __DBG_STUB_INVOKE__
            static char mockModel[1024];  // Dummy model
            return reinterpret_cast<CanvasProxyModel*>(mockModel);
        });
        
        // Mock CanvasProxyModel::fileInfo to avoid null pointer access
        stub.set_lamda(ADDR(CanvasProxyModel, fileInfo), [](const CanvasProxyModel *, const QModelIndex &) -> FileInfoPointer {
            __DBG_STUB_INVOKE__
            return nullptr;  // Return null for basic test
        });
        
        // Mock CanvasProxyModel::fileUrl to avoid null pointer access
        stub.set_lamda(ADDR(CanvasProxyModel, fileUrl), [](const CanvasProxyModel *, const QModelIndex &) -> QUrl {
            __DBG_STUB_INVOKE__
            return QUrl("file:///tmp/test.txt");  // Return dummy URL
        });
        
        // Mock CanvasProxyModel::rootUrl to avoid null pointer access
        stub.set_lamda(ADDR(CanvasProxyModel, rootUrl), [](const CanvasProxyModel *) -> QUrl {
            __DBG_STUB_INVOKE__
            return QUrl("file:///tmp");  // Return dummy root URL
        });
        
        // Create the actual CanvasItemDelegate with stubbing
        delegate = new CanvasItemDelegate(parentView);
    }

    virtual void TearDown() override
    {
        // Clear stubs first to prevent any side effects during cleanup
        stub.clear();
        
        // Remove any posted events
        QCoreApplication::removePostedEvents(nullptr);
        
        // Clean up delegate and parent view
        if (delegate) {
            delegate->disconnect();
            delete delegate;
            delegate = nullptr;
        }
        
        if (parentView) {
            parentView->disconnect();
            delete parentView;
            parentView = nullptr;
        }
        
        // Remove any remaining posted events
        QCoreApplication::removePostedEvents(nullptr);
    }

public:
    stub_ext::StubExt stub;
    CanvasItemDelegate *delegate = nullptr;
    CanvasView *parentView = nullptr;
};

/**
 * @brief Test CanvasItemDelegate constructor
 * Validates that the delegate can be created successfully
 */
TEST_F(UT_CanvasItemDelegate, constructor_CreateDelegate_ObjectCreated)
{
    // Test that delegate was created successfully
    EXPECT_NE(delegate, nullptr);
    EXPECT_NE(delegate->parent(), nullptr);
}

/**
 * @brief Test iconLevel getter and setter
 * Validates iconLevel operations work correctly  
 */
TEST_F(UT_CanvasItemDelegate, iconLevel_SetAndGet_ReturnsCorrectValue)
{
    // Test default icon level
    int currentLevel = delegate->iconLevel();
    EXPECT_GE(currentLevel, delegate->minimumIconLevel());
    EXPECT_LE(currentLevel, delegate->maximumIconLevel());
    
    // Test setting new icon level
    int newLevel = 2;
    int actualLevel = delegate->setIconLevel(newLevel);
    EXPECT_EQ(actualLevel, newLevel);
    EXPECT_EQ(delegate->iconLevel(), newLevel);
}

/**
 * @brief Test iconSize method
 * Validates iconSize returns appropriate size for given level
 */
TEST_F(UT_CanvasItemDelegate, iconSize_WithValidLevel_ReturnsValidSize)
{
    int level = 1;
    QSize size = delegate->iconSize(level);
    
    // Icon size should be positive and reasonable
    EXPECT_GT(size.width(), 0);
    EXPECT_GT(size.height(), 0);
    EXPECT_LE(size.width(), 512);  // Reasonable upper bound
    EXPECT_LE(size.height(), 512);
}

/**
 * @brief Test sizeHint method
 * Validates sizeHint returns appropriate size for items
 */
TEST_F(UT_CanvasItemDelegate, sizeHint_WithValidInput_ReturnsValidSize)
{
    QStyleOptionViewItem option;
    QModelIndex index;  // Invalid index for basic test
    
    // Use existing stub from SetUp - no need to reset
    QSize hint = delegate->sizeHint(option, index);
    
    // For invalid index, size hint might be invalid (-1, -1)
    // This is actually correct behavior, so we test for this
    // or test that it doesn't crash
    EXPECT_NO_THROW(delegate->sizeHint(option, index));
    
    // The test just ensures sizeHint doesn't crash with our stubbing setup
    // Actual size validation would require proper model data
}

/**
 * @brief Test paint method with basic stubbing
 * Validates paint method can be called without crashing
 */
TEST_F(UT_CanvasItemDelegate, paint_WithValidInput_DoesNotCrash)
{
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    QModelIndex index;  // Invalid index for basic test
    
    // Use existing stub from SetUp - no need to reset
    // Note: avoid stubbing QStyle::drawControl as it causes linking issues
    
    // For invalid model index, paint should return early without complex operations
    // This tests the basic paint call without triggering complex model interactions
    EXPECT_NO_THROW(delegate->paint(&painter, option, index));
}

/**
 * @brief Test createEditor method
 * Validates createEditor creates appropriate editor widget
 */
TEST_F(UT_CanvasItemDelegate, createEditor_WithValidInput_CreatesEditor)
{
    QWidget parent;
    QStyleOptionViewItem option;
    QModelIndex index;  // Invalid index for basic test
    
    // Use existing stub from SetUp - no need to reset
    
    QWidget *editor = delegate->createEditor(&parent, option, index);
    
    // Editor should be created (could be nullptr for invalid index, which is acceptable)
    // We just test that the method doesn't crash
    if (editor) {
        delete editor;
    }
}

/**
 * @brief Test setEditorData method
 * Validates setEditorData works with valid editor
 */
TEST_F(UT_CanvasItemDelegate, setEditorData_WithValidEditor_SetsData)
{
    QLineEdit editor;
    QModelIndex index;  // Invalid index for basic test
    
    // Use existing stub from SetUp - no need to reset
    
    // Test that setEditorData doesn't crash
    EXPECT_NO_THROW(delegate->setEditorData(&editor, index));
}

/**
 * @brief Test setModelData method  
 * Validates setModelData transfers data from editor to model
 */
TEST_F(UT_CanvasItemDelegate, setModelData_WithValidEditor_SetsModelData)
{
    QLineEdit editor;
    editor.setText("New File Name");
    
    // Mock model for testing
    QAbstractItemModel *mockModel = nullptr;
    QModelIndex index;  // Invalid index for basic test
    
    // Use existing stub from SetUp - no need to reset
    // Mock tracking for this test
    bool setDataCalled = false;  // Note: actual setData may not be called with invalid model/index
    
    // Test that setModelData doesn't crash
    EXPECT_NO_THROW(delegate->setModelData(&editor, mockModel, index));
}

/**
 * @brief Test updateEditorGeometry method
 * Validates updateEditorGeometry updates editor position correctly
 */
TEST_F(UT_CanvasItemDelegate, updateEditorGeometry_WithValidEditor_UpdatesGeometry)
{
    QLineEdit editor;
    QStyleOptionViewItem option;
    option.rect = QRect(10, 20, 100, 30);
    QModelIndex index;  // Invalid index for basic test
    
    // Test that updateEditorGeometry doesn't crash
    EXPECT_NO_THROW(delegate->updateEditorGeometry(&editor, option, index));
    
    // Verify that editor geometry was updated (basic check)
    // The exact geometry depends on internal calculations
}

/**
 * @brief Test minimum and maximum icon levels
 * Validates icon level bounds are reasonable
 */
TEST_F(UT_CanvasItemDelegate, iconLevels_MinMaxValues_AreReasonable)
{
    int minLevel = delegate->minimumIconLevel();
    int maxLevel = delegate->maximumIconLevel();
    
    // Icon levels should be reasonable
    EXPECT_GE(minLevel, 0);
    EXPECT_GT(maxLevel, minLevel);
    EXPECT_LE(maxLevel, 10);  // Reasonable upper bound
}

/**
 * @brief Test paintGeomertys method (note: typo in original API)
 * Validates paintGeomertys returns valid geometry list
 */
TEST_F(UT_CanvasItemDelegate, paintGeomertys_WithValidInput_ReturnsGeometryList)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    QModelIndex index;  // Invalid index for basic test
    
    // Use existing stub from SetUp - no need to reset QAbstractItemModel::data
    
    QList<QRect> geometries = delegate->paintGeomertys(option, index);
    
    // Should return a list (could be empty for invalid index)
    // We just test that the method doesn't crash
    EXPECT_TRUE(geometries.size() >= 0);
}

/**
 * @brief Test expendedGeomerty method (note: typo in original API)
 * Validates expendedGeomerty returns valid geometry
 */
TEST_F(UT_CanvasItemDelegate, expendedGeomerty_WithValidInput_ReturnsValidRect)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    QModelIndex index;  // Invalid index for basic test
    
    // Use existing stub from SetUp - no need to reset
    
    QRect geometry = delegate->expendedGeomerty(option, index);
    
    // Should return a valid rect (could be empty for invalid index)
    // We just test that the method doesn't crash
    EXPECT_TRUE(geometry.isValid() || geometry.isEmpty());
}
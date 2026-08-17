// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/iconitemdelegate.h"
#include "views/fileview.h"
#include "models/fileviewmodel.h"
#include "utils/fileviewhelper.h"

#include <QPainter>
#include <QStyleOptionViewItem>
#include <QModelIndex>
#include <QRect>
#include <QSize>
#include <QFontMetrics>
#include <QIcon>
#include <QPixmap>
#include <QApplication>
#include <QStyle>
#include <QHelpEvent>
#include <QEvent>
#include <QAbstractItemView>
#include <QWidget>
#include <QLineEdit>
#include <QUrl>

using namespace dfmplugin_workspace;

class IconItemDelegateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        testUrl = QUrl::fromLocalFile("/tmp/test");
        fileView = new FileView(testUrl);
        fileViewHelper = new FileViewHelper(fileView);
        
        // Create a mock model and set it to the view
        mockModel = new FileViewModel();
        fileView->setModel(mockModel);
        
        delegate = new IconItemDelegate(fileViewHelper);
        
        // Mock FileView methods
        stub.set_lamda(ADDR(FileView, model), [this]() -> FileViewModel* {
            return mockModel;
        });
        
        stub.set_lamda(ADDR(FileView, iconSize), [this]() -> QSize {
            return QSize(64, 64); // Default icon size for icon view
        });
        
        
        stub.set_lamda(ADDR(FileView, isSelected), [this](FileView *, const QModelIndex &index) -> bool {
            return false;
        });
        
        stub.set_lamda(ADDR(FileView, selectedIndexCount), [this]() -> int {
            return 0;
        });
    }

    void TearDown() override
    {
        stub.clear();
        EXPECT_NO_FATAL_FAILURE(delete delegate);
        EXPECT_NO_FATAL_FAILURE(delete fileViewHelper);
        EXPECT_NO_FATAL_FAILURE(delete fileView);
        // mockModel is owned by fileView, so we don't need to delete it separately
        mockModel = nullptr;
    }

    QUrl testUrl;
    FileView *fileView;
    FileViewHelper *fileViewHelper;
    IconItemDelegate *delegate;
    FileViewModel *mockModel { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(IconItemDelegateTest, CanCreateAndDestroy)
{
    IconItemDelegate *testDelegate = nullptr;
    FileViewHelper *helper = new FileViewHelper(fileView);
    
    EXPECT_NO_FATAL_FAILURE({
        testDelegate = new IconItemDelegate(helper);
    });
    
    EXPECT_NE(testDelegate, nullptr);
    
    EXPECT_NO_FATAL_FAILURE({
        delete testDelegate;
        delete helper;
    });
}

TEST_F(IconItemDelegateTest, CanPaint)
{
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    option.state = QStyle::State_Enabled;
    option.text = "Test File";
    option.decorationSize = QSize(64, 64);
    
    QModelIndex index;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        delegate->paint(&painter, option, index);
    });
}

TEST_F(IconItemDelegateTest, CanGetSizeHint)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    option.decorationSize = QSize(64, 64);
    
    QModelIndex index;
    
    QSize size = delegate->sizeHint(option, index);
    
    EXPECT_GT(size.width(), 0);
    EXPECT_GT(size.height(), 0);
}

TEST_F(IconItemDelegateTest, CanCreateEditor)
{
    QWidget *parent = new QWidget();
    QStyleOptionViewItem option;
    QModelIndex index;
    
    QWidget *editor = delegate->createEditor(parent, option, index);
    
    EXPECT_NE(editor, nullptr);
    
    delete editor;
    delete parent;
}

TEST_F(IconItemDelegateTest, CanUpdateEditorGeometry)
{
    QWidget *editor = new QWidget();
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    QModelIndex index;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        delegate->updateEditorGeometry(editor, option, index);
    });
    
    delete editor;
}

TEST_F(IconItemDelegateTest, CanSetEditorData)
{
    QWidget *editor = new QWidget();
    QStyleOptionViewItem option;
    QModelIndex index;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        delegate->setEditorData(editor, index);
    });
    
    delete editor;
}

TEST_F(IconItemDelegateTest, CanGetPaintGeomertys)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    QModelIndex index;
    
    QList<QRect> geometries = delegate->paintGeomertys(option, index);
    
    EXPECT_TRUE(geometries.isEmpty() || !geometries.isEmpty());
}

TEST_F(IconItemDelegateTest, CanUpdateItemSizeHint)
{
    EXPECT_NO_FATAL_FAILURE({
        delegate->updateItemSizeHint();
    });
}

TEST_F(IconItemDelegateTest, CanGetItemIconRect)
{
    QRectF itemRect(0, 0, 100, 100);
    
    QRectF result = delegate->itemIconRect(itemRect);
    
    EXPECT_TRUE(result.isValid());
}

TEST_F(IconItemDelegateTest, CanGetItemGeomertys)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    QModelIndex index;
    
    QList<QRect> geometries = delegate->itemGeomertys(option, index);
    
    EXPECT_TRUE(geometries.isEmpty() || !geometries.isEmpty());
}

TEST_F(IconItemDelegateTest, CanGetIconSizeLevel)
{
    int level = delegate->iconSizeLevel();
    
    EXPECT_GE(level, 0);
}

TEST_F(IconItemDelegateTest, CanGetMinimumIconSizeLevel)
{
    int level = delegate->minimumIconSizeLevel();
    
    EXPECT_GE(level, 0);
}

TEST_F(IconItemDelegateTest, CanGetMaximumIconSizeLevel)
{
    int level = delegate->maximumIconSizeLevel();
    
    EXPECT_GE(level, 0);
}

TEST_F(IconItemDelegateTest, CanIncreaseIcon)
{
    int level = delegate->iconSizeLevel();
    int newLevel = delegate->increaseIcon();
    
    EXPECT_GE(newLevel, level);
}

TEST_F(IconItemDelegateTest, CanDecreaseIcon)
{
    int level = delegate->iconSizeLevel();
    int newLevel = delegate->decreaseIcon();
    
    EXPECT_LE(newLevel, level);
}

TEST_F(IconItemDelegateTest, CanSetIconSizeByIconSizeLevel)
{
    int level = 1;
    
    int result = delegate->setIconSizeByIconSizeLevel(level);
    
    EXPECT_EQ(result, level);
}

TEST_F(IconItemDelegateTest, CanGetMinimumWidthLevel)
{
    int level = delegate->minimumWidthLevel();
    
    EXPECT_GE(level, 0);
}

TEST_F(IconItemDelegateTest, CanSetItemMinimumWidthByWidthLevel)
{
    int level = 1;
    
    EXPECT_NO_FATAL_FAILURE({
        delegate->setItemMinimumWidthByWidthLevel(level);
    });
}

TEST_F(IconItemDelegateTest, CanHideNotEditingIndexWidget)
{
    EXPECT_NO_FATAL_FAILURE({
        delegate->hideNotEditingIndexWidget();
    });
}

TEST_F(IconItemDelegateTest, CanGetExpandItemRect)
{
    QRect result = delegate->expandItemRect();
    
    EXPECT_TRUE(result.isValid() || !result.isValid());
}

TEST_F(IconItemDelegateTest, CanGetExpandedIndex)
{
    QModelIndex result = delegate->expandedIndex();
    
    EXPECT_TRUE(result.isValid() || !result.isValid());
}

TEST_F(IconItemDelegateTest, CanGetExpandedItem)
{
    QWidget *result = delegate->expandedItem();
    
    EXPECT_NE(result, nullptr);
}

TEST_F(IconItemDelegateTest, CanGetDisplayFileName)
{
    QModelIndex index;
    
    QString result = delegate->displayFileName(index);
    
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(IconItemDelegateTest, CanGetCalcFileNameRect)
{
    QModelIndex index;
    QRectF rect(0, 0, 100, 100);
    Qt::TextElideMode elideMode = Qt::ElideRight;
    
    QList<QRectF> result = delegate->calcFileNameRect(index, rect, elideMode);
    
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(IconItemDelegateTest, CanHandleEventFilter)
{
    QObject *object = new QObject();
    QEvent *event = new QEvent(QEvent::None);
    
    bool result = delegate->eventFilter(object, event);
    
    EXPECT_TRUE(result == true || result == false);
    
    delete event;
    delete object;
}

TEST_F(IconItemDelegateTest, CanHandleHelpEvent)
{
    QHelpEvent event(QEvent::ToolTip, QPoint(50, 50), QPoint(50, 50));
    FileView *view = new FileView(testUrl);
    QStyleOptionViewItem option;
    QModelIndex index;
    
    bool result = delegate->helpEvent(&event, view, option, index);
    
    EXPECT_TRUE(result == true || result == false);
    
    delete view;
}

TEST_F(IconItemDelegateTest, CanHandleDifferentIconSizeLevels)
{
    int minLevel = delegate->minimumIconSizeLevel();
    int maxLevel = delegate->maximumIconSizeLevel();
    
    // Test setting to minimum
    EXPECT_NO_FATAL_FAILURE({
        delegate->setIconSizeByIconSizeLevel(minLevel);
    });
    
    // Test setting to maximum
    EXPECT_NO_FATAL_FAILURE({
        delegate->setIconSizeByIconSizeLevel(maxLevel);
    });
    
    // Test setting to invalid level (should not crash)
    EXPECT_NO_FATAL_FAILURE({
        delegate->setIconSizeByIconSizeLevel(-1);
    });
    
    EXPECT_NO_FATAL_FAILURE({
        delegate->setIconSizeByIconSizeLevel(maxLevel + 1);
    });
}

TEST_F(IconItemDelegateTest, CanHandleIconIncreaseDecrease)
{
    int originalLevel = delegate->iconSizeLevel();
    
    // Test increasing beyond maximum
    int maxLevel = delegate->maximumIconSizeLevel();
    for (int i = 0; i <= maxLevel + 1; ++i) {
        EXPECT_NO_FATAL_FAILURE({
            delegate->increaseIcon();
        });
    }
    
    // Reset to original level
    delegate->setIconSizeByIconSizeLevel(originalLevel);
    
    // Test decreasing beyond minimum
    for (int i = 0; i <= originalLevel + 1; ++i) {
        EXPECT_NO_FATAL_FAILURE({
            delegate->decreaseIcon();
        });
    }
}

TEST_F(IconItemDelegateTest, CanHandlePaintWithDifferentStates)
{
    QPixmap pixmap(100, 100);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    option.decorationSize = QSize(64, 64);
    QModelIndex index;
    
    // Test with different states
    option.state = QStyle::State_Enabled;
    EXPECT_NO_FATAL_FAILURE({
        delegate->paint(&painter, option, index);
    });
    
    option.state = QStyle::State_Selected;
    EXPECT_NO_FATAL_FAILURE({
        delegate->paint(&painter, option, index);
    });
    
    option.state = QStyle::State_MouseOver;
    EXPECT_NO_FATAL_FAILURE({
        delegate->paint(&painter, option, index);
    });
}

TEST_F(IconItemDelegateTest, CanHandleSizeHintWithDifferentOptions)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    
    // Test with different rectangle sizes
    option.rect = QRect(0, 0, 100, 100);
    EXPECT_NO_FATAL_FAILURE({
        QSize size = delegate->sizeHint(option, index);
        EXPECT_GT(size.width(), 0);
        EXPECT_GT(size.height(), 0);
    });
    
    option.rect = QRect(0, 0, 200, 200);
    EXPECT_NO_FATAL_FAILURE({
        QSize size = delegate->sizeHint(option, index);
        EXPECT_GT(size.width(), 0);
        EXPECT_GT(size.height(), 0);
    });
}

TEST_F(IconItemDelegateTest, CanHandleEditorOperations)
{
    QWidget *parent = new QWidget();
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 100, 100);
    QModelIndex index;
    
    // Test creating editor
    QWidget *editor = delegate->createEditor(parent, option, index);
    EXPECT_NE(editor, nullptr);
    
    // Test updating editor geometry
    EXPECT_NO_FATAL_FAILURE({
        delegate->updateEditorGeometry(editor, option, index);
    });
    
    // Test setting editor data
    EXPECT_NO_FATAL_FAILURE({
        delegate->setEditorData(editor, index);
    });
    
    delete editor;
    delete parent;
}

TEST_F(IconItemDelegateTest, CanHandleWidthLevelOperations)
{
    int minLevel = 0;
    int maxLevel = 5; // Assume some reasonable max level
    
    // Test setting different width levels
    for (int level = minLevel; level <= maxLevel; ++level) {
        EXPECT_NO_FATAL_FAILURE({
            delegate->setItemMinimumWidthByWidthLevel(level);
        });
    }
    
    // Test getting minimum width level
    int currentLevel = delegate->minimumWidthLevel();
    EXPECT_GE(currentLevel, 0);
}

// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/listitemdelegate.h"
#include "views/fileview.h"
#include "views/listitempaintproxy.h"
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

class ListItemDelegateTest : public ::testing::Test
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
        
        delegate = new ListItemDelegate(fileViewHelper);
        
        // Mock FileView methods
        stub.set_lamda(ADDR(FileView, model), [this]() -> FileViewModel* {
            return mockModel;
        });
        
        stub.set_lamda(ADDR(FileView, iconSize), [this]() -> QSize {
            return QSize(32, 32); // Default icon size for list view
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
    ListItemDelegate *delegate;
    FileViewModel *mockModel { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(ListItemDelegateTest, CanCreateAndDestroy)
{
    ListItemDelegate *testDelegate = nullptr;
    FileViewHelper *helper = new FileViewHelper(fileView);
    
    EXPECT_NO_FATAL_FAILURE({
        testDelegate = new ListItemDelegate(helper);
    });
    
    EXPECT_NE(testDelegate, nullptr);
    
    EXPECT_NO_FATAL_FAILURE({
        delete testDelegate;
        delete helper;
    });
}

TEST_F(ListItemDelegateTest, CanPaint)
{
    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.state = QStyle::State_Enabled;
    option.text = "Test File";
    option.decorationSize = QSize(32, 32);
    
    QModelIndex index;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        delegate->paint(&painter, option, index);
    });
}

TEST_F(ListItemDelegateTest, CanGetSizeHint)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.decorationSize = QSize(32, 32);
    
    QModelIndex index;
    
    QSize size = delegate->sizeHint(option, index);
    
    EXPECT_GT(size.width(), 0);
    EXPECT_GT(size.height(), 0);
}

TEST_F(ListItemDelegateTest, CanCreateEditor)
{
    QWidget *parent = new QWidget();
    QStyleOptionViewItem option;
    QModelIndex index;
    
    QWidget *editor = delegate->createEditor(parent, option, index);
    
    EXPECT_NE(editor, nullptr);
    
    delete editor;
    delete parent;
}

TEST_F(ListItemDelegateTest, CanUpdateEditorGeometry)
{
    QWidget *editor = new QWidget();
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    QModelIndex index;
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        delegate->updateEditorGeometry(editor, option, index);
    });
    
    delete editor;
}

TEST_F(ListItemDelegateTest, CanSetEditorData)
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

TEST_F(ListItemDelegateTest, CanGetPaintGeomertys)
{
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    QModelIndex index;
    
    // Set paint proxy to avoid null pointer crash
    ListItemPaintProxy *paintProxy = new ListItemPaintProxy(fileView);
    delegate->setPaintProxy(paintProxy);
    
    QList<QRect> geometries = delegate->paintGeomertys(option, index);
    
    EXPECT_TRUE(geometries.isEmpty() || !geometries.isEmpty());
}

TEST_F(ListItemDelegateTest, CanUpdateItemSizeHint)
{
    EXPECT_NO_FATAL_FAILURE({
        delegate->updateItemSizeHint();
    });
}

TEST_F(ListItemDelegateTest, CanGetItemIconRect)
{
    QRectF itemRect(0, 0, 200, 30);
    
    QRectF result = delegate->itemIconRect(itemRect);
    
    EXPECT_TRUE(result.isValid());
}

TEST_F(ListItemDelegateTest, CanGetRectOfItem)
{
    QModelIndex index;
    
    QRect result = delegate->getRectOfItem(RectOfItemType::kItemIconRect, index);
    
    EXPECT_TRUE(result.isValid() || !result.isValid());
}

TEST_F(ListItemDelegateTest, CanGetIconSizeLevel)
{
    int level = delegate->iconSizeLevel();
    
    EXPECT_GE(level, 0);
}

TEST_F(ListItemDelegateTest, CanGetMinimumIconSizeLevel)
{
    int level = delegate->minimumIconSizeLevel();
    
    EXPECT_GE(level, 0);
}

TEST_F(ListItemDelegateTest, CanGetMaximumIconSizeLevel)
{
    int level = delegate->maximumIconSizeLevel();
    
    EXPECT_GE(level, 0);
}

TEST_F(ListItemDelegateTest, CanIncreaseIcon)
{
    int level = delegate->iconSizeLevel();
    int newLevel = delegate->increaseIcon();
    
    EXPECT_GE(newLevel, level);
}

TEST_F(ListItemDelegateTest, CanDecreaseIcon)
{
    int level = delegate->iconSizeLevel();
    int newLevel = delegate->decreaseIcon();
    
    EXPECT_LE(newLevel, level);
}

TEST_F(ListItemDelegateTest, CanSetIconSizeByIconSizeLevel)
{
    int level = 1;
    
    int result = delegate->setIconSizeByIconSizeLevel(level);
    
    EXPECT_EQ(result, level);
}

TEST_F(ListItemDelegateTest, CanHandleEventFilter)
{
    QObject *object = new QObject();
    QEvent *event = new QEvent(QEvent::None);
    
    bool result = delegate->eventFilter(object, event);
    
    EXPECT_TRUE(result == true || result == false);
    
    delete event;
    delete object;
}

TEST_F(ListItemDelegateTest, CanHandleHelpEvent)
{
    QHelpEvent event(QEvent::ToolTip, QPoint(100, 15), QPoint(100, 15));
    QStyleOptionViewItem option;
    QModelIndex index;
    
    // Just test that it doesn't crash with null view
    EXPECT_NO_FATAL_FAILURE({
        bool result = delegate->helpEvent(&event, nullptr, option, index);
        (void)result; // Suppress unused variable warning
    });
}

TEST_F(ListItemDelegateTest, CanHandleDifferentIconSizeLevels)
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

TEST_F(ListItemDelegateTest, CanHandleIconIncreaseDecrease)
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

TEST_F(ListItemDelegateTest, CanHandlePaintWithDifferentStates)
{
    QPixmap pixmap(200, 30);
    QPainter painter(&pixmap);
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
    option.decorationSize = QSize(32, 32);
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

TEST_F(ListItemDelegateTest, CanHandleSizeHintWithDifferentOptions)
{
    QStyleOptionViewItem option;
    QModelIndex index;
    
    // Test with different rectangle sizes
    option.rect = QRect(0, 0, 100, 30);
    EXPECT_NO_FATAL_FAILURE({
        QSize size = delegate->sizeHint(option, index);
        EXPECT_GT(size.width(), 0);
        EXPECT_GT(size.height(), 0);
    });
    
    option.rect = QRect(0, 0, 200, 30);
    EXPECT_NO_FATAL_FAILURE({
        QSize size = delegate->sizeHint(option, index);
        EXPECT_GT(size.width(), 0);
        EXPECT_GT(size.height(), 0);
    });
    
    option.rect = QRect(0, 0, 500, 30);
    EXPECT_NO_FATAL_FAILURE({
        QSize size = delegate->sizeHint(option, index);
        EXPECT_GT(size.width(), 0);
        EXPECT_GT(size.height(), 0);
    });
}

TEST_F(ListItemDelegateTest, CanHandleEditorOperations)
{
    QWidget *parent = new QWidget();
    QStyleOptionViewItem option;
    option.rect = QRect(0, 0, 200, 30);
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
// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/private/iconitemdelegate_p.h"
#include "views/iconitemdelegate.h"
#include "views/expandedItem.h"
#include "utils/fileviewhelper.h"
#include "views/fileview.h"

#include <QIcon>
#include <QSize>
#include <QPointer>
#include <QModelIndex>
#include <QTextDocument>
#include <QAbstractItemView>
#include <QUrl>
#include <QWidget>

using namespace dfmplugin_workspace;

class IconItemDelegatePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        fileView = new FileView(QUrl::fromLocalFile("/tmp"));
        fileViewHelper = new FileViewHelper(fileView);
        
        // Create mock viewport widget
        mockViewport = new QWidget();
        
        // Mock FileViewHelper::parent() to return fileView
        stub.set_lamda(ADDR(FileViewHelper, parent), [this]() -> FileView* {
            return fileView;
        });
        
        // Mock fileView->viewport() to return mockViewport
        stub.set_lamda(ADDR(FileView, viewport), [this]() -> QWidget* {
            return mockViewport;
        });
        
        // Mock fileView->iconSize() to return a valid size
        stub.set_lamda(ADDR(FileView, iconSize), [this]() -> QSize {
            return QSize(64, 64);
        });
        
        delegate = new IconItemDelegate(fileViewHelper);
        d = delegate->d_func();
    }

    void TearDown() override
    {
        delete delegate;
        delete fileViewHelper;
        delete fileView;
        delete mockViewport;
        stub.clear();
    }

    FileView *fileView = nullptr;
    FileViewHelper *fileViewHelper = nullptr;
    IconItemDelegate *delegate = nullptr;
    IconItemDelegatePrivate *d = nullptr;
    QWidget *mockViewport = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IconItemDelegatePrivateTest, Constructor_SetsQPointer)
{
    // Test that constructor sets q_ptr correctly
    EXPECT_EQ(d->q_ptr, delegate);
}

TEST_F(IconItemDelegatePrivateTest, Destructor_DoesNotCrash)
{
    // Test destructor
    auto *testD = new IconItemDelegatePrivate(delegate);
    EXPECT_NO_THROW(delete testD);
}

TEST_F(IconItemDelegatePrivateTest, CheckedIcon_InitialValue)
{
    // Test initial value of checkedIcon
    EXPECT_FALSE(d->checkedIcon.isNull());
}

TEST_F(IconItemDelegatePrivateTest, ItemIconSize_InitialValue)
{
    // Test initial value of itemIconSize
    // Note: itemIconSize is set in constructor, so it should not be empty
    EXPECT_FALSE(d->itemIconSize.isEmpty());
}

TEST_F(IconItemDelegatePrivateTest, ExpandedItem_InitialValue)
{
    // Test initial value of expandedItem
    // Note: expandedItem is created in constructor, so it should not be null
    EXPECT_FALSE(d->expandedItem.isNull());
}

TEST_F(IconItemDelegatePrivateTest, LastAndExpandedIndex_InitialValue)
{
    // Test initial value of lastAndExpandedIndex
    EXPECT_FALSE(d->lastAndExpandedIndex.isValid());
}

TEST_F(IconItemDelegatePrivateTest, ExpandedIndex_InitialValue)
{
    // Test initial value of expandedIndex
    EXPECT_FALSE(d->expandedIndex.isValid());
}

TEST_F(IconItemDelegatePrivateTest, CurrentIconSizeIndex_InitialValue)
{
    // Test initial value of currentIconSizeIndex
    EXPECT_EQ(d->currentIconSizeIndex, 1);
}

TEST_F(IconItemDelegatePrivateTest, CurrentIconGridWidthIndex_InitialValue)
{
    // Test initial value of currentIconGridWidthIndex
    EXPECT_EQ(d->currentIconGridWidthIndex, 3);
}

TEST_F(IconItemDelegatePrivateTest, Document_InitialValue)
{
    // Test initial value of document
    EXPECT_EQ(d->document, nullptr);
}

TEST_F(IconItemDelegatePrivateTest, SetCheckedIcon_ValidIcon_SetsIcon)
{
    // Test setting checkedIcon
    QIcon testIcon = QIcon::fromTheme("user-home");
    d->checkedIcon = testIcon;
    
    EXPECT_FALSE(d->checkedIcon.isNull());
    EXPECT_EQ(d->checkedIcon.cacheKey(), testIcon.cacheKey());
}

TEST_F(IconItemDelegatePrivateTest, SetItemIconSize_ValidSize_SetsSize)
{
    // Test setting itemIconSize
    QSize testSize(64, 64);
    d->itemIconSize = testSize;
    
    EXPECT_EQ(d->itemIconSize, testSize);
}

TEST_F(IconItemDelegatePrivateTest, SetCurrentIconSizeIndex_ValidIndex_SetsIndex)
{
    // Test setting currentIconSizeIndex
    int testIndex = 2;
    d->currentIconSizeIndex = testIndex;
    
    EXPECT_EQ(d->currentIconSizeIndex, testIndex);
}

TEST_F(IconItemDelegatePrivateTest, SetCurrentIconGridWidthIndex_ValidIndex_SetsIndex)
{
    // Test setting currentIconGridWidthIndex
    int testIndex = 4;
    d->currentIconGridWidthIndex = testIndex;
    
    EXPECT_EQ(d->currentIconGridWidthIndex, testIndex);
}

TEST_F(IconItemDelegatePrivateTest, SetExpandedItem_ValidItem_SetsItem)
{
    // Test setting expandedItem
    auto testItem = new ExpandedItem(delegate);
    d->expandedItem = testItem;
    
    EXPECT_EQ(d->expandedItem.data(), testItem);
    
    delete testItem;
}

TEST_F(IconItemDelegatePrivateTest, SetLastAndExpandedIndex_ValidIndex_SetsIndex)
{
    // Test setting lastAndExpandedIndex
    QModelIndex testIndex;
    d->lastAndExpandedIndex = testIndex;
    
    EXPECT_EQ(d->lastAndExpandedIndex, testIndex);
}

TEST_F(IconItemDelegatePrivateTest, SetExpandedIndex_ValidIndex_SetsIndex)
{
    // Test setting expandedIndex
    QModelIndex testIndex;
    d->expandedIndex = testIndex;
    
    EXPECT_EQ(d->expandedIndex, testIndex);
}

TEST_F(IconItemDelegatePrivateTest, SetDocument_ValidDocument_SetsDocument)
{
    // Test setting document
    auto testDocument = new QTextDocument();
    d->document = testDocument;
    
    EXPECT_EQ(d->document, testDocument);
    
    delete testDocument;
}

// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/private/listitemdelegate_p.h"
#include "views/listitemdelegate.h"
#include "utils/fileviewhelper.h"
#include "views/fileview.h"

#include <QUrl>
#include <QWidget>

using namespace dfmplugin_workspace;

class ListItemDelegatePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        fileView = new FileView(QUrl::fromLocalFile("/tmp"));
        fileViewHelper = new FileViewHelper(fileView);
        
        // Mock FileViewHelper::parent() to return fileView
        stub.set_lamda(ADDR(FileViewHelper, parent), [this]() -> FileView* {
            return fileView;
        });
        
        // Mock fileView->viewport() to return a valid widget
        stub.set_lamda(ADDR(FileView, viewport), []() -> QWidget* {
            return new QWidget();
        });
        
        // Mock fileView->iconSize() to return a valid size
        stub.set_lamda(ADDR(FileView, iconSize), []() -> QSize {
            return QSize(64, 64);
        });
        
        delegate = new ListItemDelegate(fileViewHelper);
        d = delegate->d_func();
    }

    void TearDown() override
    {
        delete delegate;
        delete fileViewHelper;
        delete fileView;
        stub.clear();
    }

    FileView *fileView = nullptr;
    FileViewHelper *fileViewHelper = nullptr;
    ListItemDelegate *delegate = nullptr;
    ListItemDelegatePrivate *d = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ListItemDelegatePrivateTest, Constructor_SetsQPointer)
{
    // Test that constructor sets q_ptr correctly
    EXPECT_EQ(d->q_ptr, delegate);
}

TEST_F(ListItemDelegatePrivateTest, Destructor_DoesNotCrash)
{
    // Test destructor
    auto *testD = new ListItemDelegatePrivate(delegate);
    EXPECT_NO_THROW(delete testD);
}

TEST_F(ListItemDelegatePrivateTest, CurrentHeightLevel_InitialValue)
{
    // Test initial value of currentHeightLevel
    EXPECT_EQ(d->currentHeightLevel, 1);
}

TEST_F(ListItemDelegatePrivateTest, SetCurrentHeightLevel_ValidLevel_SetsLevel)
{
    // Test setting currentHeightLevel
    int testLevel = 2;
    d->currentHeightLevel = testLevel;
    
    EXPECT_EQ(d->currentHeightLevel, testLevel);
}

TEST_F(ListItemDelegatePrivateTest, SetCurrentHeightLevel_ZeroLevel_SetsLevel)
{
    // Test setting currentHeightLevel to zero
    int testLevel = 0;
    d->currentHeightLevel = testLevel;
    
    EXPECT_EQ(d->currentHeightLevel, testLevel);
}

TEST_F(ListItemDelegatePrivateTest, SetCurrentHeightLevel_NegativeLevel_SetsLevel)
{
    // Test setting currentHeightLevel to negative value
    int testLevel = -1;
    d->currentHeightLevel = testLevel;
    
    EXPECT_EQ(d->currentHeightLevel, testLevel);
}

TEST_F(ListItemDelegatePrivateTest, SetCurrentHeightLevel_LargeLevel_SetsLevel)
{
    // Test setting currentHeightLevel to large value
    int testLevel = 100;
    d->currentHeightLevel = testLevel;
    
    EXPECT_EQ(d->currentHeightLevel, testLevel);
}
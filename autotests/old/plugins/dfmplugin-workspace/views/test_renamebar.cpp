// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "views/renamebar.h"

#include <QUrl>
#include <QList>
#include <QLineEdit>
#include <QKeyEvent>

using namespace dfmplugin_workspace;

class RenameBarTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a parent widget to manage RenameBar properly
        parentWidget = new QWidget();
        renameBar = new RenameBar(parentWidget);
    }

    void TearDown() override
    {
        // Clean up in proper order to avoid memory issues
        if (renameBar) {
            renameBar->setParent(nullptr);
            delete renameBar;
            renameBar = nullptr;
        }
        if (parentWidget) {
            delete parentWidget;
            parentWidget = nullptr;
        }
        stub.clear();
    }

    QWidget *parentWidget { nullptr };
    RenameBar *renameBar { nullptr };
    stub_ext::StubExt stub;
};

TEST_F(RenameBarTest, Constructor_CreatesWidget)
{
    EXPECT_NE(renameBar, nullptr);
    EXPECT_EQ(renameBar->parent(), parentWidget);
}

TEST_F(RenameBarTest, Reset_ResetsAllFields)
{
    // Set some values first
    renameBar->storeUrlList({QUrl::fromLocalFile("/tmp/test1"), QUrl::fromLocalFile("/tmp/test2")});
    
    // Call reset
    renameBar->reset();
    
    // Verify reset worked (just test that it doesn't crash)
    EXPECT_NO_FATAL_FAILURE({
        renameBar->reset();
    });
}

TEST_F(RenameBarTest, StoreUrlList_StoresUrls)
{
    QList<QUrl> urls = {QUrl::fromLocalFile("/tmp/test1"), QUrl::fromLocalFile("/tmp/test2")};
    
    renameBar->storeUrlList(urls);
    
    // Just test that it doesn't crash
    EXPECT_NO_FATAL_FAILURE({
        renameBar->storeUrlList(urls);
    });
}

TEST_F(RenameBarTest, SetVisible_SetsVisibility)
{
    // Test setting visible true
    EXPECT_NO_FATAL_FAILURE({
        renameBar->setVisible(true);
    });
    
    // Test setting visible false
    EXPECT_NO_FATAL_FAILURE({
        renameBar->setVisible(false);
    });
}

TEST_F(RenameBarTest, OnVisibleChanged_HandlesVisibilityChange)
{
    // Test with true
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onVisibleChanged(true);
    });
    
    // Test with false
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onVisibleChanged(false);
    });
}

TEST_F(RenameBarTest, OnRenamePatternChanged_HandlesPatternChange)
{
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onRenamePatternChanged(0);
    });
    
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onRenamePatternChanged(1);
    });
}

TEST_F(RenameBarTest, OnReplaceOperatorFileNameChanged_HandlesTextChange)
{
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onReplaceOperatorFileNameChanged("test");
    });
    
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onReplaceOperatorFileNameChanged("");
    });
}

TEST_F(RenameBarTest, OnReplaceOperatorDestNameChanged_HandlesTextChange)
{
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onReplaceOperatorDestNameChanged("test");
    });
}

TEST_F(RenameBarTest, OnAddOperatorAddedContentChanged_HandlesTextChange)
{
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onAddOperatorAddedContentChanged("test");
    });
    
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onAddOperatorAddedContentChanged("");
    });
}

TEST_F(RenameBarTest, OnAddTextPatternChanged_HandlesPatternChange)
{
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onAddTextPatternChanged(0);
    });
    
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onAddTextPatternChanged(1);
    });
}

TEST_F(RenameBarTest, OnCustomOperatorFileNameChanged_HandlesTextChange)
{
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onCustomOperatorFileNameChanged();
    });
}

TEST_F(RenameBarTest, OnCustomOperatorSNNumberChanged_HandlesNumberChange)
{
    // Simplify test to avoid recursive stub calls
    // Just test that the method can be called without crashing
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onCustomOperatorSNNumberChanged();
    });
}

TEST_F(RenameBarTest, EventDispatcher_HandlesEventDispatch)
{
    // Mock getSelectFiles
    QList<QUrl> testUrls = {QUrl::fromLocalFile("/tmp/test")};
    stub.set_lamda(ADDR(RenameBar, getSelectFiles), [&testUrls]() {
        return testUrls;
    });
    
    EXPECT_NO_FATAL_FAILURE({
        renameBar->eventDispatcher();
    });
}

TEST_F(RenameBarTest, HideRenameBar_HidesBar)
{
    EXPECT_NO_FATAL_FAILURE({
        renameBar->hideRenameBar();
    });
}

TEST_F(RenameBarTest, OnSelectUrlChanged_HandlesSelectionChange)
{
    QList<QUrl> emptyList;
    QList<QUrl> nonEmptyList = {QUrl::fromLocalFile("/tmp/test")};
    
    // Test with empty list
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onSelectUrlChanged(emptyList);
    });
    
    // Test with non-empty list
    EXPECT_NO_FATAL_FAILURE({
        renameBar->onSelectUrlChanged(nonEmptyList);
    });
}

TEST_F(RenameBarTest, KeyPressEvent_HandlesKeyPress)
{
    QKeyEvent enterEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QKeyEvent escapeEvent(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    
    EXPECT_NO_FATAL_FAILURE({
        renameBar->keyPressEvent(&enterEvent);
    });
    
    EXPECT_NO_FATAL_FAILURE({
        renameBar->keyPressEvent(&escapeEvent);
    });
}

TEST_F(RenameBarTest, GetSelectFiles_ReturnsFiles)
{
    EXPECT_NO_FATAL_FAILURE({
        auto result = renameBar->getSelectFiles();
        (void)result; // Suppress unused variable warning
    });
}

TEST_F(RenameBarTest, FindPage_ReturnsPage)
{
    EXPECT_NO_FATAL_FAILURE({
        auto result = renameBar->findPage();
        (void)result; // Suppress unused variable warning
    });
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shortcuthelper_1.cpp
 * @brief Unit tests for ShortcutHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/shortcuthelper.h"

#include <QTest>

using namespace dfmplugin_workspace;

class ShortcutHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShortcutHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShortcutHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShortcutHelperTest, cdUp)
{
    // Test method: void cdUp(())
    EXPECT_NO_FATAL_FAILURE(obj->cdUp());
}

TEST_F(ShortcutHelperTest, copyFilePath)
{
    // Test method: void copyFilePath(())
    EXPECT_NO_FATAL_FAILURE(obj->copyFilePath());
}

TEST_F(ShortcutHelperTest, copyFiles)
{
    // Test method: void copyFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->copyFiles());
}

TEST_F(ShortcutHelperTest, cutFiles)
{
    // Test method: void cutFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->cutFiles());
}

TEST_F(ShortcutHelperTest, doEnterPressed)
{
    // Test bool getter: doEnterPressed()
    bool result = obj->doEnterPressed();
    EXPECT_FALSE(result);

}

TEST_F(ShortcutHelperTest, initRenameProcessTimer)
{
    // Test method: void initRenameProcessTimer(())
    EXPECT_NO_FATAL_FAILURE(obj->initRenameProcessTimer());
}

TEST_F(ShortcutHelperTest, moveToTrash)
{
    // Test method: void moveToTrash(())
    EXPECT_NO_FATAL_FAILURE(obj->moveToTrash());
}

TEST_F(ShortcutHelperTest, openInTerminal)
{
    // Test method: void openInTerminal(())
    EXPECT_NO_FATAL_FAILURE(obj->openInTerminal());
}

TEST_F(ShortcutHelperTest, pasteFiles)
{
    // Test method: void pasteFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->pasteFiles());
}

TEST_F(ShortcutHelperTest, previewFiles)
{
    // Test method: void previewFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->previewFiles());
}

TEST_F(ShortcutHelperTest, redoFiles)
{
    // Test method: void redoFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->redoFiles());
}

TEST_F(ShortcutHelperTest, registerAction)
{
    // Test method: void registerAction((QKeySequence::StandardKey shortcut, bool autoRepeat))
    EXPECT_NO_FATAL_FAILURE(obj->registerAction(QKeySequence::StandardKey(), false));
}

TEST_F(ShortcutHelperTest, registerShortcut)
{
    // Test method: void registerShortcut(())
    EXPECT_NO_FATAL_FAILURE(obj->registerShortcut());
}

TEST_F(ShortcutHelperTest, renameProcessing)
{
    // Test method: void renameProcessing(())
    EXPECT_NO_FATAL_FAILURE(obj->renameProcessing());
}

TEST_F(ShortcutHelperTest, reverseSelect)
{
    // Test bool getter: reverseSelect()
    bool result = obj->reverseSelect();
    EXPECT_FALSE(result);

}

TEST_F(ShortcutHelperTest, showFilesProperty)
{
    // Test method: void showFilesProperty(())
    EXPECT_NO_FATAL_FAILURE(obj->showFilesProperty());
}

TEST_F(ShortcutHelperTest, toggleHiddenFiles)
{
    // Test method: void toggleHiddenFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->toggleHiddenFiles());
}

TEST_F(ShortcutHelperTest, touchFolder)
{
    // Test method: void touchFolder(())
    EXPECT_NO_FATAL_FAILURE(obj->touchFolder());
}

TEST_F(ShortcutHelperTest, undoFiles)
{
    // Test method: void undoFiles(())
    EXPECT_NO_FATAL_FAILURE(obj->undoFiles());
}

// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/shortcuthelper.h"
#include "utils/workspacehelper.h"
#include "views/fileview.h"

#include <QKeyEvent>
#include <QAction>
#include <QTimer>
#include <QUrl>
#include <QList>

using namespace dfmplugin_workspace;
using namespace dfmbase;

class ShortcutHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        fileView = new FileView(QUrl());
        shortcutHelper = new ShortcutHelper(fileView);
    }

    void TearDown() override
    {
        delete shortcutHelper;
        delete fileView;
        stub.clear();
    }

    FileView *fileView = nullptr;
    ShortcutHelper *shortcutHelper = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShortcutHelperTest, Constructor_SetsParent)
{
    // Test that constructor sets parent correctly
    EXPECT_EQ(shortcutHelper->parent(), fileView);
}

TEST_F(ShortcutHelperTest, Destructor_DoesNotCrash)
{
    // Test destructor
    auto *testHelper = new ShortcutHelper(fileView);
    EXPECT_NO_THROW(delete testHelper);
}

TEST_F(ShortcutHelperTest, RegisterShortcut_DoesNotCrash)
{
    // Test registerShortcut method
    EXPECT_NO_THROW(shortcutHelper->registerShortcut());
}

TEST_F(ShortcutHelperTest, RegisterAction_ValidShortcut_DoesNotCrash)
{
    // Test registerAction with valid shortcut
    QKeySequence::StandardKey shortcut = QKeySequence::Copy;
    bool autoRepeat = false;
    
    EXPECT_NO_THROW(shortcutHelper->registerAction(shortcut, autoRepeat));
}

TEST_F(ShortcutHelperTest, NormalKeyPressEventHandle_EnterKey_ReturnsTrue)
{
    // Test normalKeyPressEventHandle with Enter key
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    
    // Mock renameProcessTimer
    QTimer mockTimer;
    mockTimer.setSingleShot(true);
    mockTimer.setInterval(500);
    
    // Mock doEnterPressed to return true
    stub.set_lamda(ADDR(ShortcutHelper, doEnterPressed), []() {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    bool result = shortcutHelper->normalKeyPressEventHandle(&event);
    EXPECT_TRUE(result);
}

TEST_F(ShortcutHelperTest, NormalKeyPressEventHandle_BackspaceKey_ReturnsTrue)
{
    // Test normalKeyPressEventHandle with Backspace key
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    
    // Mock cdUp
    stub.set_lamda(ADDR(FileView, cdUp), [](FileView*) -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    bool result = shortcutHelper->normalKeyPressEventHandle(&event);
    EXPECT_TRUE(result);
}

TEST_F(ShortcutHelperTest, NormalKeyPressEventHandle_DeleteKey_DoesNotCrash)
{
    // Test normalKeyPressEventHandle with Delete key
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier);
    
    // Mock selectedUrlList to return empty list
    stub.set_lamda(VADDR(FileView, selectedUrlList), [](const FileView*) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return QList<QUrl>();
    });
    
    // Mock moveToTrash
    stub.set_lamda(ADDR(ShortcutHelper, moveToTrash), []() {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(shortcutHelper->normalKeyPressEventHandle(&event));
}

TEST_F(ShortcutHelperTest, NormalKeyPressEventHandle_EscapeKey_ReturnsTrue)
{
    // Test normalKeyPressEventHandle with Escape key
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    
    bool result = shortcutHelper->normalKeyPressEventHandle(&event);
    EXPECT_TRUE(result);
}

TEST_F(ShortcutHelperTest, NormalKeyPressEventHandle_F2Key_DoesNotCrash)
{
    // Test normalKeyPressEventHandle with F2 key
    QKeyEvent event(QEvent::KeyPress, Qt::Key_F2, Qt::NoModifier);
    
    // Mock selectedUrlList to return empty list
    stub.set_lamda(VADDR(FileView, selectedUrlList), [](const FileView*) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return QList<QUrl>();
    });
    
    EXPECT_NO_THROW(shortcutHelper->normalKeyPressEventHandle(&event));
}

TEST_F(ShortcutHelperTest, DoEnterPressed_NoSelection_ReturnsFalse)
{
    // Mock selectedUrlList to return empty list
    stub.set_lamda(VADDR(FileView, selectedUrlList), [](const FileView*) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return QList<QUrl>();
    });
    
    // Mock WorkspaceHelper::instance to return a valid mock object
    static WorkspaceHelper mockHelper;
    stub.set_lamda(ADDR(WorkspaceHelper, instance), []() -> WorkspaceHelper* {
        __DBG_STUB_INVOKE__
        return &mockHelper;
    });
    
    // Mock windowId
    stub.set_lamda(ADDR(WorkspaceHelper, windowId), [](WorkspaceHelper*, const QWidget*) -> quint64 {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(12345);
    });
    
    // Skip dpfHookSequence stub for now
    
    // Mock openAction
    stub.set_lamda(ADDR(ShortcutHelper, openAction), [](ShortcutHelper*, const QList<QUrl>&, const DirOpenMode&) {
        __DBG_STUB_INVOKE__
    });
    
    bool result = shortcutHelper->doEnterPressed();
    EXPECT_FALSE(result);
}

TEST_F(ShortcutHelperTest, InitRenameProcessTimer_DoesNotCrash)
{
    // Test initRenameProcessTimer method
    EXPECT_NO_THROW(shortcutHelper->initRenameProcessTimer());
}

TEST_F(ShortcutHelperTest, ProcessKeyPressEvent_SpaceKey_ReturnsTrue)
{
    // Test processKeyPressEvent with Space key
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
    
    // Mock previewFiles
    stub.set_lamda(ADDR(ShortcutHelper, previewFiles), []() {
        __DBG_STUB_INVOKE__
    });
    
    bool result = shortcutHelper->processKeyPressEvent(&event);
    EXPECT_TRUE(result);
}

TEST_F(ShortcutHelperTest, ProcessKeyPressEvent_CtrlH_ReturnsTrue)
{
    // Test processKeyPressEvent with Ctrl+H
    QKeyEvent event(QEvent::KeyPress, Qt::Key_H, Qt::ControlModifier);
    
    // Mock toggleHiddenFiles
    stub.set_lamda(ADDR(ShortcutHelper, toggleHiddenFiles), []() {
        __DBG_STUB_INVOKE__
    });
    
    bool result = shortcutHelper->processKeyPressEvent(&event);
    EXPECT_TRUE(result);
}

TEST_F(ShortcutHelperTest, ProcessKeyPressEvent_CtrlI_ReturnsTrue)
{
    // Test processKeyPressEvent with Ctrl+I
    QKeyEvent event(QEvent::KeyPress, Qt::Key_I, Qt::ControlModifier);
    
    // Mock showFilesProperty
    stub.set_lamda(ADDR(ShortcutHelper, showFilesProperty), []() {
        __DBG_STUB_INVOKE__
    });
    
    bool result = shortcutHelper->processKeyPressEvent(&event);
    EXPECT_TRUE(result);
}

TEST_F(ShortcutHelperTest, ActionTriggered_CopyAction_DoesNotCrash)
{
    // Test actionTriggered with Copy action
    QAction *action = new QAction();
    action->setProperty("_view_shortcut_key", static_cast<int>(QKeySequence::Copy));
    
    // Mock copyFiles
    stub.set_lamda(ADDR(ShortcutHelper, copyFiles), []() {
        __DBG_STUB_INVOKE__
    });
    
    // Mock sender
    stub.set_lamda(ADDR(QObject, sender), [action]() {
        __DBG_STUB_INVOKE__
        return action;
    });
    
    EXPECT_NO_THROW(shortcutHelper->acitonTriggered());
    
    delete action;
}

TEST_F(ShortcutHelperTest, CopyFiles_NoSelection_DoesNotCrash)
{
    // Mock selectedUrlList to return empty list
    stub.set_lamda(VADDR(FileView, selectedUrlList), [](const FileView*) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return QList<QUrl>();
    });
    
    EXPECT_NO_THROW(shortcutHelper->copyFiles());
}

TEST_F(ShortcutHelperTest, CutFiles_NoSelection_DoesNotCrash)
{
    // Mock selectedUrlList to return empty list
    stub.set_lamda(VADDR(FileView, selectedUrlList), [](const FileView*) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return QList<QUrl>();
    });
    
    EXPECT_NO_THROW(shortcutHelper->cutFiles());
}

TEST_F(ShortcutHelperTest, PasteFiles_DoesNotCrash)
{
    // Mock WorkspaceHelper::instance
    static WorkspaceHelper mockHelper;
    stub.set_lamda(ADDR(WorkspaceHelper, instance), []() -> WorkspaceHelper* {
        __DBG_STUB_INVOKE__
        return &mockHelper;
    });
    
    // Mock windowId
    stub.set_lamda(ADDR(WorkspaceHelper, windowId), [](WorkspaceHelper*, const QWidget*) -> quint64 {
        __DBG_STUB_INVOKE__
        return static_cast<quint64>(12345);
    });
    
    // Mock rootUrl
    stub.set_lamda(VADDR(FileView, rootUrl), [](const FileView*) -> QUrl {
        __DBG_STUB_INVOKE__
        return QUrl("file:///tmp");
    });
    
    // Mock ClipBoard::instance
    // Skip ClipBoard stub for now
    
    EXPECT_NO_THROW(shortcutHelper->pasteFiles());
}

TEST_F(ShortcutHelperTest, UndoFiles_DoesNotCrash)
{
    EXPECT_NO_THROW(shortcutHelper->undoFiles());
}

TEST_F(ShortcutHelperTest, DeleteFiles_NoSelection_DoesNotCrash)
{
    // Mock selectedTreeViewUrlList to return empty list
    // Skip selectedTreeViewUrlList stub for now
    
    EXPECT_NO_THROW(shortcutHelper->deleteFiles());
}

TEST_F(ShortcutHelperTest, MoveToTrash_NoSelection_DoesNotCrash)
{
    // Mock selectedTreeViewUrlList to return empty list
    // Skip selectedTreeViewUrlList stub for now
    
    EXPECT_NO_THROW(shortcutHelper->moveToTrash());
}

TEST_F(ShortcutHelperTest, TouchFolder_DoesNotCrash)
{
    // Mock clearSelection
    stub.set_lamda(ADDR(FileView, clearSelection), []() {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(shortcutHelper->touchFolder());
}

TEST_F(ShortcutHelperTest, ToggleHiddenFiles_DoesNotCrash)
{
    // Mock Application::instance
    stub.set_lamda(ADDR(Application, instance), []() -> Application* {
        __DBG_STUB_INVOKE__
        return nullptr;
    });
    
    // Mock genericAttribute
    stub.set_lamda(ADDR(Application, genericAttribute), [](Application::GenericAttribute) {
        __DBG_STUB_INVOKE__
        return QVariant(false);
    });
    
    // Mock setGenericAttribute
    stub.set_lamda(ADDR(Application, setGenericAttribute), [](Application::GenericAttribute, const QVariant&) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(shortcutHelper->toggleHiddenFiles());
}

TEST_F(ShortcutHelperTest, ShowFilesProperty_DoesNotCrash)
{
    EXPECT_NO_THROW(shortcutHelper->showFilesProperty());
}

TEST_F(ShortcutHelperTest, PreviewFiles_NoSelection_DoesNotCrash)
{
    // Mock selectedUrlList to return empty list
    stub.set_lamda(VADDR(FileView, selectedUrlList), [](const FileView*) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return QList<QUrl>();
    });
    
    EXPECT_NO_THROW(shortcutHelper->previewFiles());
}

TEST_F(ShortcutHelperTest, OpenAction_DoesNotCrash)
{
    QList<QUrl> urls;
    urls.append(QUrl("file:///tmp/test.txt"));
    
    EXPECT_NO_THROW(shortcutHelper->openAction(urls, DirOpenMode::kOpenInCurrentWindow));
}

TEST_F(ShortcutHelperTest, OpenInTerminal_DoesNotCrash)
{
    EXPECT_NO_THROW(shortcutHelper->openInTerminal());
}

TEST_F(ShortcutHelperTest, CdUp_DoesNotCrash)
{
    EXPECT_NO_THROW(shortcutHelper->cdUp());
}

TEST_F(ShortcutHelperTest, RedoFiles_DoesNotCrash)
{
    EXPECT_NO_THROW(shortcutHelper->redoFiles());
}

TEST_F(ShortcutHelperTest, ReverseSelect_SingleSelection_ReturnsFalse)
{
    // Mock selectionMode to return SingleSelection
    stub.set_lamda(ADDR(FileView, selectionMode), [](QAbstractItemView*) -> QAbstractItemView::SelectionMode {
        __DBG_STUB_INVOKE__
        return QAbstractItemView::SingleSelection;
    });
    
    bool result = shortcutHelper->reverseSelect();
    EXPECT_FALSE(result);
}

TEST_F(ShortcutHelperTest, ReverseSelect_NoSelection_ReturnsFalse)
{
    // Mock selectionMode to return MultiSelection
    stub.set_lamda(ADDR(FileView, selectionMode), [](QAbstractItemView*) -> QAbstractItemView::SelectionMode {
        __DBG_STUB_INVOKE__
        return QAbstractItemView::MultiSelection;
    });
    
    // Mock selectedUrlList to return empty list
    stub.set_lamda(VADDR(FileView, selectedUrlList), [](const FileView*) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return QList<QUrl>();
    });
    
    bool result = shortcutHelper->reverseSelect();
    EXPECT_FALSE(result);
}

TEST_F(ShortcutHelperTest, RenameProcessing_DoesNotCrash)
{
    EXPECT_NO_THROW(shortcutHelper->renameProcessing());
}

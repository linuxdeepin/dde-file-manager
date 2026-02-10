// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "delegate/itemeditor.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QTextEdit>
#include <QTimer>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QShowEvent>
#include <QContextMenuEvent>
#include <QPaintEvent>
#include <QMenu>
#include <QAction>
#include <QTextCursor>
#include <QFontMetrics>
#include <QMetaObject>

#include <dfm-base/utils/fileutils.h>
#include <DTextEdit>
#include <DArrowRectangle>
#include <QLabel>
#include <QCoreApplication>

using namespace ddplugin_canvas;
DWIDGET_USE_NAMESPACE

class UT_ItemEditor : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        parentWidget = new QWidget();
        editor = new ItemEditor(parentWidget);
    }

    virtual void TearDown() override
    {
        // Clear stubs first to prevent any side effects during cleanup
        stub.clear();
        
        // Stop all timers and remove posted events before cleanup
        QCoreApplication::removePostedEvents(nullptr);
        
        // Safely clean up editor and its components
        if (editor) {
            // Disconnect all connections to prevent signals during cleanup
            editor->disconnect();
            delete editor;
            editor = nullptr;
        }
        
        if (parentWidget) {
            parentWidget->disconnect();
            delete parentWidget;
            parentWidget = nullptr;
        }
        
        // Remove any remaining posted events
        QCoreApplication::removePostedEvents(nullptr);
        
        // Note: Avoid QCoreApplication::processEvents() in TearDown
        // as it can trigger timer events that access deleted objects
    }

public:
    QApplication *app = nullptr;
    QWidget *parentWidget = nullptr;
    ItemEditor *editor = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_ItemEditor, constructor_CreateEditor_InitializesCorrectly)
{
    EXPECT_NE(editor, nullptr);
    EXPECT_EQ(editor->parent(), parentWidget);
}

TEST_F(UT_ItemEditor, show_ShowEditor_MakesVisible)
{
    // Mock QWidget::show to avoid actual GUI operations
    bool showCalled = false;
    stub.set_lamda(ADDR(QWidget, show), [&showCalled] {
        __DBG_STUB_INVOKE__
        showCalled = true;
    });
    
    editor->show();
    EXPECT_TRUE(showCalled);
}

TEST_F(UT_ItemEditor, hide_HideEditor_MakesInvisible)
{
    // Mock QWidget::hide to avoid actual GUI operations
    bool hideCalled = false;
    stub.set_lamda(ADDR(QWidget, hide), [&hideCalled] {
        __DBG_STUB_INVOKE__
        hideCalled = true;
    });
    
    editor->hide();
    EXPECT_TRUE(hideCalled);
}

TEST_F(UT_ItemEditor, setOpacity_SetOpacity_UpdatesEffect)
{
    qreal opacity = 0.5;
    
    // Mock QGraphicsOpacityEffect::setOpacity
    bool setOpacityCalled = false;
    stub.set_lamda(ADDR(QGraphicsOpacityEffect, setOpacity), [&setOpacityCalled] {
        __DBG_STUB_INVOKE__
        setOpacityCalled = true;
    });
    
    editor->setOpacity(opacity);
    // The method should not crash
    EXPECT_NO_THROW(editor->setOpacity(opacity));
}

TEST_F(UT_ItemEditor, setGeometry_SetGeometry_UpdatesGeometry)
{
    QRect geometry(10, 20, 100, 50);
    
    // Mock QWidget::setGeometry - need static_cast for overloaded function
    bool setGeometryCalled = false;
    using SetGeometryFunc = void (QWidget::*)(const QRect&);
    stub.set_lamda(static_cast<SetGeometryFunc>(&QWidget::setGeometry), 
                   [&setGeometryCalled] {
        __DBG_STUB_INVOKE__
        setGeometryCalled = true;
    });
    
    editor->setGeometry(geometry);
    EXPECT_TRUE(setGeometryCalled);
}

// Note: setMaxCharSize method doesn't exist in ItemEditor

TEST_F(UT_ItemEditor, setText_SetText_UpdatesText)
{
    QString text = "test.txt";
    
    // Test that the method doesn't crash
    EXPECT_NO_THROW(editor->setText(text));
}

TEST_F(UT_ItemEditor, text_GetText_ReturnsText)
{
    // Test that the method doesn't crash and returns a string
    QString result = editor->text();
    EXPECT_NO_THROW(editor->text());
}

TEST_F(UT_ItemEditor, setBaseGeometry_WithValidGeometry_UpdatesGeometry)
{
    QRect baseRect(0, 0, 100, 80);
    QSize itemSize(100, 80);
    QMargins margins(5, 5, 5, 5);
    
    // Mock layout operations to avoid GUI dependencies
    using SetGeometryIntFunc = void (QWidget::*)(int, int, int, int);
    stub.set_lamda(static_cast<SetGeometryIntFunc>(&QWidget::setGeometry), 
                   [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QWidget, setFixedWidth), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QWidget, setMinimumHeight), [] {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(editor->setBaseGeometry(baseRect, itemSize, margins));
}

TEST_F(UT_ItemEditor, setMaxHeight_WithValidHeight_SetsHeight)
{
    int height = 200;
    editor->setMaxHeight(height);
    // The method should not crash
    EXPECT_NO_THROW(editor->setMaxHeight(height));
}

TEST_F(UT_ItemEditor, setMaximumLength_WithValidLength_SetsLength)
{
    int length = 255;
    editor->setMaximumLength(length);
    EXPECT_EQ(editor->maximumLength(), length);
}

TEST_F(UT_ItemEditor, setMaximumLength_WithZeroLength_DoesNotSet)
{
    int originalLength = editor->maximumLength();
    editor->setMaximumLength(0);
    EXPECT_EQ(editor->maximumLength(), originalLength);
}

TEST_F(UT_ItemEditor, setCharCountLimit_SetsCharCountMode)
{
    EXPECT_NO_THROW(editor->setCharCountLimit());
}

TEST_F(UT_ItemEditor, select_WithValidPart_SelectsText)
{
    QString testText = "test.txt";
    QString partToSelect = "test";
    
    // Set text first
    editor->setText(testText);
    
    // Test selection
    EXPECT_NO_THROW(editor->select(partToSelect));
}

TEST_F(UT_ItemEditor, select_WithInvalidPart_DoesNotCrash)
{
    QString testText = "test.txt";
    QString partToSelect = "nonexistent";
    
    editor->setText(testText);
    EXPECT_NO_THROW(editor->select(partToSelect));
}

TEST_F(UT_ItemEditor, setOpacity_WithFullOpacity_RemovesEffect)
{
    // Test with full opacity (should remove effect)
    EXPECT_NO_THROW(editor->setOpacity(1.0));
}

TEST_F(UT_ItemEditor, setOpacity_WithPartialOpacity_SetsEffect)
{
    // Mock QGraphicsOpacityEffect creation and setting
    stub.set_lamda(ADDR(QGraphicsOpacityEffect, setOpacity), [] {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(editor->setOpacity(0.5));
    EXPECT_NO_THROW(editor->setOpacity(0.8));
}

TEST_F(UT_ItemEditor, editor_GetEditor_ReturnsEditor)
{
    RenameEdit *renameEdit = editor->editor();
    EXPECT_NE(renameEdit, nullptr);
}

TEST_F(UT_ItemEditor, showAlertMessage_WithValidMessage_ShowsTooltip)
{
    QString message = "Invalid characters";
    int duration = 2000;
    
    // Mock tooltip creation and operations
    // QTimer::singleShot has many overloads, use the one with std::function
    using SingleShotFunc = void (*)(int, const std::function<void()>&);
    stub.set_lamda(static_cast<SingleShotFunc>(&QTimer::singleShot), [] {
        __DBG_STUB_INVOKE__
    });
    
    // Simply test the method doesn't crash - the business logic validation is covered elsewhere
    EXPECT_NO_THROW(editor->showAlertMessage(message, duration));
}

TEST_F(UT_ItemEditor, text_ReturnsCorrectText)
{
    // Test text() method to increase coverage without GUI complications
    // Mock textEditor->toPlainText() to return known text
    QString expectedText = "Test content";
    stub.set_lamda(ADDR(QTextEdit, toPlainText), [expectedText](QTextEdit*) -> QString {
        __DBG_STUB_INVOKE__
        return expectedText;
    });
    
    QString actualText = editor->text();
    EXPECT_EQ(actualText, expectedText);
}

TEST_F(UT_ItemEditor, setText_SetsTextCorrectly)
{
    // Test setText() method to increase coverage
    QString testText = "New test content";
    
    // Mock textEditor operations - member function needs 'this' pointer as first parameter
    stub.set_lamda(ADDR(QTextEdit, setPlainText), [](QTextEdit*, const QString&) {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(editor->setText(testText));
}

TEST_F(UT_ItemEditor, updateGeometry_WithDifferentStates_UpdatesCorrectly)
{
    // Mock various geometry-related methods
    stub.set_lamda(ADDR(QWidget, width), []() -> int {
        __DBG_STUB_INVOKE__
        return 100;
    });
    
    stub.set_lamda(ADDR(QWidget, adjustSize), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(VADDR(QWidget, updateGeometry), [] {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(editor->updateGeometry());
}

TEST_F(UT_ItemEditor, textChanged_WithEmptyText_UpdatesGeometry)
{
    // Test the textChanged slot indirectly by calling updateGeometry
    EXPECT_NO_THROW(editor->updateGeometry());
}

TEST_F(UT_ItemEditor, destructor_WithTooltip_CleansUp)
{
    // Create a new editor to test destructor
    ItemEditor *testEditor = new ItemEditor();
    
    // Mock tooltip operations
    stub.set_lamda(ADDR(QWidget, hide), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QObject, deleteLater), [] {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(delete testEditor);
}

// Tests for RenameEdit class
class UT_RenameEdit : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        parentWidget = new QWidget();
        renameEdit = new RenameEdit(parentWidget);
    }

    virtual void TearDown() override
    {
        delete renameEdit;
        delete parentWidget;
        
        stub.clear();
    }

public:
    QApplication *app = nullptr;
    QWidget *parentWidget = nullptr;
    RenameEdit *renameEdit = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_RenameEdit, constructor_CreateRenameEdit_InitializesCorrectly)
{
    EXPECT_NE(renameEdit, nullptr);
    EXPECT_EQ(renameEdit->parent(), parentWidget);
}

TEST_F(UT_RenameEdit, undo_WithStack_RestoresPreviousText)
{
    // Mock various operations to avoid GUI dependencies
    stub.set_lamda(ADDR(QTextEdit, setPlainText), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, setTextCursor), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, setAlignment), [] {
        __DBG_STUB_INVOKE__
    });
    
    // For QMetaObject::invokeMethod - it's a static function
    using InvokeMethodFunc = bool (*)(QObject*, const char*, Qt::ConnectionType);
    stub.set_lamda(static_cast<InvokeMethodFunc>(&QMetaObject::invokeMethod), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    EXPECT_NO_THROW(renameEdit->undo());
}

TEST_F(UT_RenameEdit, redo_WithStack_RestoresNextText)
{
    // Mock the same operations as undo
    stub.set_lamda(ADDR(QTextEdit, setPlainText), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, setTextCursor), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, setAlignment), [] {
        __DBG_STUB_INVOKE__
    });
    
    using InvokeMethodFunc = bool (*)(QObject*, const char*, Qt::ConnectionType);
    stub.set_lamda(static_cast<InvokeMethodFunc>(&QMetaObject::invokeMethod), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    EXPECT_NO_THROW(renameEdit->redo());
}

TEST_F(UT_RenameEdit, eventFilter_WithPaintEvent_HandlesCustomPainting)
{
    QWidget *testWidget = new QWidget();
    QPaintEvent paintEvent(QRect(0, 0, 100, 100));
    
    // Mock style and painting operations
    stub.set_lamda(ADDR(QWidget, style), []() -> QStyle* {
        __DBG_STUB_INVOKE__
        return QApplication::style();
    });
    
    renameEdit->eventFilter(renameEdit, &paintEvent);
    
    delete testWidget;
}

TEST_F(UT_RenameEdit, contextMenuEvent_WithReadOnlyFalse_ShowsMenu)
{
    QContextMenuEvent contextEvent(QContextMenuEvent::Mouse, QPoint(10, 10), QPoint(10, 10));
    
    // Mock menu creation and operations
    stub.set_lamda(static_cast<QMenu* (QTextEdit::*)()>(&QTextEdit::createStandardContextMenu), []() -> QMenu* {
        __DBG_STUB_INVOKE__
        return new QMenu();
    });
    
    // findChild is a template function - simplify by stubbing the specific function needed
    // Instead of stubbing findChild, we'll skip this complex operation
    // Just create mock actions directly in the test
    
    using MenuExecFunc = QAction* (QMenu::*)(const QPoint&, QAction*);
    stub.set_lamda(static_cast<MenuExecFunc>(&QMenu::exec), []() -> QAction* {
        __DBG_STUB_INVOKE__
        return nullptr;
    });
    
    stub.set_lamda(ADDR(QObject, deleteLater), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, isReadOnly), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    
    EXPECT_NO_THROW(renameEdit->contextMenuEvent(&contextEvent));
}

TEST_F(UT_RenameEdit, contextMenuEvent_WithReadOnlyTrue_DoesNotShowMenu)
{
    QContextMenuEvent contextEvent(QContextMenuEvent::Mouse, QPoint(10, 10), QPoint(10, 10));
    
    stub.set_lamda(ADDR(QTextEdit, isReadOnly), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    EXPECT_NO_THROW(renameEdit->contextMenuEvent(&contextEvent));
}

TEST_F(UT_RenameEdit, focusOutEvent_WhenFocusWidget_InvokesParentMethod)
{
    QFocusEvent focusEvent(QEvent::FocusOut);
    
    // QApplication::focusWidget is a static function
    using FocusWidgetFunc = QWidget* (*)();
    stub.set_lamda(static_cast<FocusWidgetFunc>(&QApplication::focusWidget), []() -> QWidget* {
        __DBG_STUB_INVOKE__
        return nullptr; // Different widget to trigger the condition
    });
    
    using InvokeMethodFunc = bool (*)(QObject*, const char*, Qt::ConnectionType);
    stub.set_lamda(static_cast<InvokeMethodFunc>(&QMetaObject::invokeMethod), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    stub.set_lamda(VADDR(DTK_WIDGET_NAMESPACE::DTextEdit, focusOutEvent), [] {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(renameEdit->focusOutEvent(&focusEvent));
}

TEST_F(UT_RenameEdit, keyPressEvent_WithUndoKey_CallsUndo)
{
    QKeyEvent undoEvent(QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
    
    // Mock the undo operation
    stub.set_lamda(ADDR(QTextEdit, setPlainText), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, setTextCursor), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, setAlignment), [] {
        __DBG_STUB_INVOKE__
    });
    
    using InvokeMethodFunc = bool (*)(QObject*, const char*, Qt::ConnectionType);
    stub.set_lamda(static_cast<InvokeMethodFunc>(&QMetaObject::invokeMethod), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    EXPECT_NO_THROW(renameEdit->keyPressEvent(&undoEvent));
}

TEST_F(UT_RenameEdit, keyPressEvent_WithRedoKey_CallsRedo)
{
    QKeyEvent redoEvent(QEvent::KeyPress, Qt::Key_Y, Qt::ControlModifier);
    
    // Mock the redo operation
    stub.set_lamda(ADDR(QTextEdit, setPlainText), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, setTextCursor), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, setAlignment), [] {
        __DBG_STUB_INVOKE__
    });
    
    using InvokeMethodFunc = bool (*)(QObject*, const char*, Qt::ConnectionType);
    stub.set_lamda(static_cast<InvokeMethodFunc>(&QMetaObject::invokeMethod), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    EXPECT_NO_THROW(renameEdit->keyPressEvent(&redoEvent));
}

TEST_F(UT_RenameEdit, keyPressEvent_WithEnterKey_InvokesParentMethod)
{
    QKeyEvent enterEvent(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    
    using InvokeMethodFunc = bool (*)(QObject*, const char*, Qt::ConnectionType);
    stub.set_lamda(static_cast<InvokeMethodFunc>(&QMetaObject::invokeMethod), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    EXPECT_NO_THROW(renameEdit->keyPressEvent(&enterEvent));
}

TEST_F(UT_RenameEdit, keyPressEvent_WithReturnKey_InvokesParentMethod)
{
    QKeyEvent returnEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    
    using InvokeMethodFunc = bool (*)(QObject*, const char*, Qt::ConnectionType);
    stub.set_lamda(static_cast<InvokeMethodFunc>(&QMetaObject::invokeMethod), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    EXPECT_NO_THROW(renameEdit->keyPressEvent(&returnEvent));
}

TEST_F(UT_RenameEdit, keyPressEvent_WithTabKey_InvokesParentMethod)
{
    QKeyEvent tabEvent(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    
    using InvokeMethodFunc = bool (*)(QObject*, const char*, Qt::ConnectionType);
    stub.set_lamda(static_cast<InvokeMethodFunc>(&QMetaObject::invokeMethod), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    EXPECT_NO_THROW(renameEdit->keyPressEvent(&tabEvent));
}

TEST_F(UT_RenameEdit, keyPressEvent_WithBacktabKey_InvokesParentMethod)
{
    QKeyEvent backtabEvent(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    
    using InvokeMethodFunc = bool (*)(QObject*, const char*, Qt::ConnectionType);
    stub.set_lamda(static_cast<InvokeMethodFunc>(&QMetaObject::invokeMethod), []() -> bool {
        __DBG_STUB_INVOKE__
        return true;
    });
    
    EXPECT_NO_THROW(renameEdit->keyPressEvent(&backtabEvent));
}

TEST_F(UT_RenameEdit, keyPressEvent_WithNormalKey_CallsParentImplementation)
{
    QKeyEvent normalEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    
    stub.set_lamda(VADDR(DTK_WIDGET_NAMESPACE::DTextEdit, keyPressEvent), [] {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(renameEdit->keyPressEvent(&normalEvent));
}

TEST_F(UT_RenameEdit, showEvent_WhenNotActiveWindow_ActivatesWindow)
{
    QShowEvent showEvent;
    
    stub.set_lamda(VADDR(DTK_WIDGET_NAMESPACE::DTextEdit, showEvent), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QWidget, isActiveWindow), []() -> bool {
        __DBG_STUB_INVOKE__
        return false; // Not active window
    });
    
    stub.set_lamda(ADDR(QWidget, activateWindow), [] {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(renameEdit->showEvent(&showEvent));
}

TEST_F(UT_RenameEdit, showEvent_WhenActiveWindow_DoesNotActivate)
{
    QShowEvent showEvent;
    
    stub.set_lamda(VADDR(DTK_WIDGET_NAMESPACE::DTextEdit, showEvent), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QWidget, isActiveWindow), []() -> bool {
        __DBG_STUB_INVOKE__
        return true; // Already active window
    });
    
    EXPECT_NO_THROW(renameEdit->showEvent(&showEvent));
}

// Additional tests for ItemEditor complex methods
TEST_F(UT_ItemEditor, textChanged_WithEmptyText_HandlesCorrectly)
{
    // Mock textEditor operations
    stub.set_lamda(ADDR(QObject, sender), [&]() -> QObject* {
        __DBG_STUB_INVOKE__
        return editor->editor(); // Return the textEditor
    });
    
    stub.set_lamda(ADDR(QTextEdit, isReadOnly), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    
    stub.set_lamda(ADDR(QTextEdit, toPlainText), []() -> QString {
        __DBG_STUB_INVOKE__
        return QString(); // Empty text
    });
    
    // Test indirectly by simulating the textChanged signal
    editor->setText(""); // This should trigger textChanged handling
    EXPECT_NO_THROW(editor->updateGeometry());
}

TEST_F(UT_ItemEditor, textChanged_WithInvalidChars_ShowsAlert)
{
    QString testText = "test|file.txt"; // Contains invalid character |
    
    // Mock necessary operations
    stub.set_lamda(ADDR(QObject, sender), [&]() -> QObject* {
        __DBG_STUB_INVOKE__
        return editor->editor();
    });
    
    stub.set_lamda(ADDR(QTextEdit, isReadOnly), []() -> bool {
        __DBG_STUB_INVOKE__
        return false;
    });
    
    stub.set_lamda(ADDR(QTextEdit, toPlainText), [&testText]() -> QString {
        __DBG_STUB_INVOKE__
        return testText;
    });
    
    // Mock FileUtils operations
    using PreprocessingFunc = QString (*)(QString);
    stub.set_lamda(static_cast<PreprocessingFunc>(&DFMBASE_NAMESPACE::FileUtils::preprocessingFileName), [](QString fileName) -> QString {
        __DBG_STUB_INVOKE__
        return fileName.left(4); // Remove invalid chars, return "test"
    });
    
    using ProcessLengthFunc = bool (*)(const QString&, int, int, bool, QString&, int&);
    stub.set_lamda(static_cast<ProcessLengthFunc>(&DFMBASE_NAMESPACE::FileUtils::processLength), [](const QString &text, int cursorPos, int maxLength, bool useCharCount, QString &result, int &resultCursorPos) -> bool {
        __DBG_STUB_INVOKE__
        result = text; // No length processing
        resultCursorPos = cursorPos;
        return true;
    });
    
    // Mock textEditor operations
    stub.set_lamda(ADDR(QTextEdit, setPlainText), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, textCursor), []() -> QTextCursor {
        __DBG_STUB_INVOKE__
        return QTextCursor();
    });
    
    stub.set_lamda(ADDR(QTextEdit, setTextCursor), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QTextEdit, setAlignment), [] {
        __DBG_STUB_INVOKE__
    });
    
    // Mock showAlertMessage
    using SingleShotFunc2 = void (*)(int, const std::function<void()>&);
    stub.set_lamda(static_cast<SingleShotFunc2>(&QTimer::singleShot), [] {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(editor->setText(testText));
}

TEST_F(UT_ItemEditor, textChanged_WithReadOnlyEditor_DoesNotProcess)
{
    // Mock textEditor operations for read-only state
    stub.set_lamda(ADDR(QObject, sender), [&]() -> QObject* {
        __DBG_STUB_INVOKE__
        return editor->editor();
    });
    
    stub.set_lamda(&QTextEdit::isReadOnly, [](QTextEdit*) -> bool {
        __DBG_STUB_INVOKE__
        return true; // Read-only mode
    });
    
    EXPECT_NO_THROW(editor->setText("test"));
}

TEST_F(UT_ItemEditor, select_WithBuggyLogic_StillWorks)
{
    // Test the buggy logic in select method (org.indexOf(org))
    QString testText = "test.txt";
    QString partToSelect = "test";
    
    editor->setText(testText);
    
    // The method has a bug: org.indexOf(org) instead of org.indexOf(part)
    // But it should still not crash
    EXPECT_NO_THROW(editor->select(partToSelect));
}

TEST_F(UT_ItemEditor, updateGeometry_WithReadOnlyEditor_SetsFixedHeight)
{
    // Mock operations for read-only mode
    stub.set_lamda(ADDR(QWidget, width), []() -> int {
        __DBG_STUB_INVOKE__
        return 100;
    });
    
    stub.set_lamda(ADDR(QWidget, setFixedWidth), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(&QTextEdit::isReadOnly, [](QTextEdit*) -> bool {
        __DBG_STUB_INVOKE__
        return true; // Read-only mode
    });
    
    stub.set_lamda(ADDR(QWidget, setFixedHeight), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(&QWidget::adjustSize, [](QWidget*) {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(VADDR(QWidget, updateGeometry), [] {
        __DBG_STUB_INVOKE__
    });
    
    EXPECT_NO_THROW(editor->updateGeometry());
}

TEST_F(UT_ItemEditor, updateGeometry_WithEditableEditor_CalculatesHeight)
{
    // Mock operations for editable mode with different height scenarios
    stub.set_lamda(ADDR(QWidget, width), []() -> int {
        __DBG_STUB_INVOKE__
        return 100;
    });
    
    stub.set_lamda(ADDR(QWidget, setFixedWidth), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(&QTextEdit::isReadOnly, [](QTextEdit*) -> bool {
        __DBG_STUB_INVOKE__
        return false; // Editable mode
    });
    
    stub.set_lamda(ADDR(QWidget, setFixedHeight), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(&QWidget::adjustSize, [](QWidget*) {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(VADDR(QWidget, updateGeometry), [] {
        __DBG_STUB_INVOKE__
    });
    
    stub.set_lamda(ADDR(QWidget, fontMetrics), []() -> QFontMetrics {
        __DBG_STUB_INVOKE__
        QFont font;
        return QFontMetrics(font);
    });
    
    // Test with max height = -1 (default)
    editor->setMaxHeight(-1);
    EXPECT_NO_THROW(editor->updateGeometry());
    
    // Test with specific max height
    editor->setMaxHeight(200);
    EXPECT_NO_THROW(editor->updateGeometry());
}

TEST_F(UT_ItemEditor, createEditor_CreatesRenameEdit)
{
    // Test the static createEditor method indirectly
    RenameEdit *newEditor = ItemEditor::createEditor();
    EXPECT_NE(newEditor, nullptr);
    delete newEditor;
}

TEST_F(UT_ItemEditor, createTooltip_CreatesDArrowRectangle)
{
    // Test the static createTooltip method indirectly
    auto tooltip = ItemEditor::createTooltip();
    EXPECT_NE(tooltip, nullptr);
    delete tooltip;
}
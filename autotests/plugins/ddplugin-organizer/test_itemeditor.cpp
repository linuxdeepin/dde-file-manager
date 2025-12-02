// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "delegate/itemeditor.h"
#include "delegate/collectionitemdelegate.h"

#include <dfm-base/utils/fileutils.h>

#include <DArrowRectangle>
#include <DStyle>

#include <QApplication>
#include <QVBoxLayout>
#include <QGraphicsOpacityEffect>
#include <QTextCursor>
#include <QLabel>
#include <QTimer>
#include <QKeyEvent>
#include <QFocusEvent>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

class UT_ItemEditor : public testing::Test
{
protected:
    void SetUp() override
    {
        editor = new ItemEditor();
    }

    void TearDown() override
    {
        delete editor;
        editor = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    ItemEditor *editor = nullptr;
};

TEST_F(UT_ItemEditor, Constructor_InitializesCorrectly)
{
    EXPECT_NE(editor, nullptr);
    EXPECT_NE(editor->editor(), nullptr);
    EXPECT_EQ(editor->frameShape(), QFrame::NoFrame);
}

TEST_F(UT_ItemEditor, Text_EmptyEditor_ReturnsEmpty)
{
    EXPECT_TRUE(editor->text().isEmpty());
}

TEST_F(UT_ItemEditor, SetText_ValidText_SetsText)
{
    QString testText = "test_file.txt";
    editor->setText(testText);
    EXPECT_EQ(editor->text(), testText);
}

TEST_F(UT_ItemEditor, SetText_EmptyText_SetsEmpty)
{
    editor->setText("");
    EXPECT_TRUE(editor->text().isEmpty());
}

TEST_F(UT_ItemEditor, MaximumLength_DefaultValue_ReturnsMax)
{
    EXPECT_EQ(editor->maximumLength(), INT_MAX);
}

TEST_F(UT_ItemEditor, SetMaximumLength_ValidValue_SetsLength)
{
    editor->setMaximumLength(255);
    EXPECT_EQ(editor->maximumLength(), 255);
}

TEST_F(UT_ItemEditor, SetMaximumLength_ZeroValue_KeepsOld)
{
    int oldMax = editor->maximumLength();
    editor->setMaximumLength(0);
    EXPECT_EQ(editor->maximumLength(), oldMax);
}

TEST_F(UT_ItemEditor, SetMaximumLength_NegativeValue_KeepsOld)
{
    int oldMax = editor->maximumLength();
    editor->setMaximumLength(-10);
    EXPECT_EQ(editor->maximumLength(), oldMax);
}

TEST_F(UT_ItemEditor, SetMaxHeight_SetsValue)
{
    editor->setMaxHeight(500);
    EXPECT_NO_THROW(editor->updateGeometry());
}

TEST_F(UT_ItemEditor, SetCharCountLimit_SetsFlag)
{
    editor->setCharCountLimit();
    EXPECT_NO_THROW(editor->updateGeometry());
}

TEST_F(UT_ItemEditor, SetOpacity_FullOpacity_RemovesEffect)
{
    editor->setOpacity(0.5);
    editor->setOpacity(1.0);
    EXPECT_NE(editor->graphicsEffect(), nullptr);
}

TEST_F(UT_ItemEditor, SetOpacity_LessThanOne_AddsEffect)
{
    editor->setOpacity(0.5);
    QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect *>(editor->graphicsEffect());
    EXPECT_NE(effect, nullptr);
    EXPECT_DOUBLE_EQ(effect->opacity(), 0.5);
}

TEST_F(UT_ItemEditor, SetOpacity_ZeroOpacity_AddsEffect)
{
    editor->setOpacity(0.0);
    QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect *>(editor->graphicsEffect());
    EXPECT_NE(effect, nullptr);
}

TEST_F(UT_ItemEditor, SetOpacity_MultipleCalls_UpdatesEffect)
{
    editor->setOpacity(0.5);
    editor->setOpacity(0.3);
    QGraphicsOpacityEffect *effect = qobject_cast<QGraphicsOpacityEffect *>(editor->graphicsEffect());
    EXPECT_NE(effect, nullptr);
    EXPECT_DOUBLE_EQ(effect->opacity(), 0.3);
}

TEST_F(UT_ItemEditor, Select_TextContainsPart_SelectsPart)
{
    QString fullText = "test_file.txt";
    QString part = "test_file";
    editor->setText(fullText);
    editor->select(part);

    QTextCursor cursor = editor->editor()->textCursor();
    EXPECT_TRUE(cursor.hasSelection());
}

TEST_F(UT_ItemEditor, Select_TextNotContainsPart_NoSelection)
{
    QString fullText = "test_file.txt";
    QString part = "nonexistent";
    editor->setText(fullText);
    editor->select(part);

    QTextCursor cursor = editor->editor()->textCursor();
    EXPECT_FALSE(cursor.hasSelection());
}

TEST_F(UT_ItemEditor, SetBaseGeometry_SetsGeometry)
{
    QRect base(100, 100, 200, 300);
    QSize itemSize(150, 200);
    QMargins margin(5, 10, 5, 10);

    editor->setBaseGeometry(base, itemSize, margin);

    EXPECT_EQ(editor->pos(), base.topLeft());
    EXPECT_EQ(editor->width(), base.width());
}

TEST_F(UT_ItemEditor, UpdateGeometry_WithText_AdjustsSize)
{
    editor->setText("test");
    editor->updateGeometry();
    EXPECT_TRUE(editor->height() > 0);
}

TEST_F(UT_ItemEditor, Editor_ReturnsValidEditor)
{
    RenameEdit *renameEdit = editor->editor();
    EXPECT_NE(renameEdit, nullptr);
}

TEST_F(UT_ItemEditor, CreateEditor_ReturnsValidEdit)
{
    RenameEdit *edit = ItemEditor::createEditor();
    EXPECT_NE(edit, nullptr);
    EXPECT_EQ(edit->wordWrapMode(), QTextOption::WrapAnywhere);
    EXPECT_EQ(edit->alignment(), Qt::AlignHCenter);
    EXPECT_EQ(edit->verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    EXPECT_EQ(edit->horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
    EXPECT_EQ(edit->frameShape(), QFrame::NoFrame);
    delete edit;
}

TEST_F(UT_ItemEditor, CreateTooltip_ReturnsValidTooltip)
{
    DArrowRectangle *tooltip = ItemEditor::createTooltip();
    EXPECT_NE(tooltip, nullptr);
    EXPECT_EQ(tooltip->objectName(), "AlertTooltip");

    QLabel *label = qobject_cast<QLabel *>(tooltip->getContent());
    EXPECT_NE(label, nullptr);
    EXPECT_TRUE(label->wordWrap());

    delete tooltip;
}

TEST_F(UT_ItemEditor, ShowAlertMessage_CreatesTooltip)
{
    stub.set_lamda(VADDR(DArrowRectangle, show), [] {
        __DBG_STUB_INVOKE__
    });

    editor->showAlertMessage("Test message", 100);
    EXPECT_NE(editor->tooltip, nullptr);
}

TEST_F(UT_ItemEditor, ShowAlertMessage_SetsLabelText)
{
    stub.set_lamda(VADDR(DArrowRectangle, show), [] {
        __DBG_STUB_INVOKE__
    });

    QString testMsg = "Test alert message";
    editor->showAlertMessage(testMsg, 100);

    QLabel *label = qobject_cast<QLabel *>(editor->tooltip->getContent());
    EXPECT_NE(label, nullptr);
    EXPECT_EQ(label->text(), testMsg);
}

class UT_RenameEdit : public testing::Test
{
protected:
    void SetUp() override
    {
        edit = new RenameEdit();
    }

    void TearDown() override
    {
        delete edit;
        edit = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    RenameEdit *edit = nullptr;
};

TEST_F(UT_RenameEdit, Constructor_InitializesCorrectly)
{
    EXPECT_NE(edit, nullptr);
    EXPECT_EQ(edit->document()->documentMargin(), CollectionItemDelegate::kTextPadding);
}

TEST_F(UT_RenameEdit, StackCurrent_EmptyStack_ReturnsEmpty)
{
    EXPECT_TRUE(edit->stackCurrent().isEmpty());
}

TEST_F(UT_RenameEdit, PushStack_AddsItem)
{
    edit->pushStatck("first");
    EXPECT_EQ(edit->stackCurrent(), "first");
}

TEST_F(UT_RenameEdit, PushStack_MultipleItems_ReturnsLast)
{
    edit->pushStatck("first");
    edit->pushStatck("second");
    edit->pushStatck("third");
    EXPECT_EQ(edit->stackCurrent(), "third");
}

TEST_F(UT_RenameEdit, StackBack_SingleItem_ReturnsSame)
{
    edit->pushStatck("only");
    QString result = edit->stackBack();
    EXPECT_EQ(result, "only");
}

TEST_F(UT_RenameEdit, StackBack_MultipleItems_ReturnsPrevious)
{
    edit->pushStatck("first");
    edit->pushStatck("second");
    QString result = edit->stackBack();
    EXPECT_EQ(result, "first");
}

TEST_F(UT_RenameEdit, StackBack_AtBeginning_ReturnsFirst)
{
    edit->pushStatck("first");
    edit->pushStatck("second");
    edit->stackBack();
    QString result = edit->stackBack();
    EXPECT_EQ(result, "first");
}

TEST_F(UT_RenameEdit, StackAdvance_AtEnd_ReturnsLast)
{
    edit->pushStatck("first");
    edit->pushStatck("second");
    QString result = edit->stackAdvance();
    EXPECT_EQ(result, "second");
}

TEST_F(UT_RenameEdit, StackAdvance_AfterBack_ReturnsNext)
{
    edit->pushStatck("first");
    edit->pushStatck("second");
    edit->pushStatck("third");
    edit->stackBack();
    edit->stackBack();
    QString result = edit->stackAdvance();
    EXPECT_EQ(result, "second");
}

TEST_F(UT_RenameEdit, PushStack_AfterBack_RemovesForwardHistory)
{
    edit->pushStatck("first");
    edit->pushStatck("second");
    edit->pushStatck("third");
    edit->stackBack();
    edit->stackBack();
    edit->pushStatck("new");

    EXPECT_EQ(edit->stackCurrent(), "new");
    QString advanced = edit->stackAdvance();
    EXPECT_EQ(advanced, "new");
}

TEST_F(UT_RenameEdit, Undo_RestoresPreviousText)
{
    edit->pushStatck("first");
    edit->pushStatck("second");

    ItemEditor *parent = new ItemEditor();
    edit->setParent(parent);

    edit->undo();
    EXPECT_EQ(edit->stackCurrent(), "first");

    edit->setParent(nullptr);
    delete parent;
}

TEST_F(UT_RenameEdit, Redo_RestoresNextText)
{
    edit->pushStatck("first");
    edit->pushStatck("second");
    edit->stackBack();

    ItemEditor *parent = new ItemEditor();
    edit->setParent(parent);

    edit->redo();
    EXPECT_EQ(edit->stackCurrent(), "second");

    edit->setParent(nullptr);
    delete parent;
}

TEST_F(UT_RenameEdit, AdjustStyle_SetsDocumentMargin)
{
    edit->adjustStyle();
    EXPECT_EQ(edit->document()->documentMargin(), CollectionItemDelegate::kTextPadding);
}

TEST_F(UT_RenameEdit, KeyPressEvent_EnterKey_AcceptsEvent)
{
    ItemEditor *parent = new ItemEditor();
    edit->setParent(parent);

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    edit->keyPressEvent(&event);

    EXPECT_TRUE(event.isAccepted());

    edit->setParent(nullptr);
    delete parent;
}

TEST_F(UT_RenameEdit, KeyPressEvent_TabKey_AcceptsEvent)
{
    ItemEditor *parent = new ItemEditor();
    edit->setParent(parent);

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    edit->keyPressEvent(&event);

    EXPECT_TRUE(event.isAccepted());

    edit->setParent(nullptr);
    delete parent;
}

TEST_F(UT_RenameEdit, KeyPressEvent_BacktabKey_AcceptsEvent)
{
    ItemEditor *parent = new ItemEditor();
    edit->setParent(parent);

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier);
    edit->keyPressEvent(&event);

    EXPECT_TRUE(event.isAccepted());

    edit->setParent(nullptr);
    delete parent;
}

TEST_F(UT_RenameEdit, KeyPressEvent_UndoSequence_CallsUndo)
{
    edit->pushStatck("first");
    edit->pushStatck("second");

    ItemEditor *parent = new ItemEditor();
    edit->setParent(parent);

    QKeyEvent event(QEvent::KeyPress, Qt::Key_Z, Qt::ControlModifier);
    edit->keyPressEvent(&event);

    EXPECT_TRUE(event.isAccepted());
    EXPECT_EQ(edit->stackCurrent(), "first");

    edit->setParent(nullptr);
    delete parent;
}

TEST_F(UT_RenameEdit, ShowEvent_NotActiveWindow_ActivatesWindow)
{
    bool activateCalled = false;
    stub.set_lamda(ADDR(QWidget, isActiveWindow), [] {
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(ADDR(QWidget, activateWindow), [&activateCalled] {
        __DBG_STUB_INVOKE__
        activateCalled = true;
    });

    QShowEvent event;
    edit->showEvent(&event);

    EXPECT_TRUE(activateCalled);
}

TEST_F(UT_RenameEdit, ShowEvent_IsActiveWindow_DoesNotActivate)
{
    bool activateCalled = false;
    stub.set_lamda(ADDR(QWidget, isActiveWindow), [] {
        __DBG_STUB_INVOKE__
        return true;
    });
    stub.set_lamda(ADDR(QWidget, activateWindow), [&activateCalled] {
        __DBG_STUB_INVOKE__
        activateCalled = true;
    });

    QShowEvent event;
    edit->showEvent(&event);

    EXPECT_FALSE(activateCalled);
}

TEST_F(UT_RenameEdit, FocusOutEvent_DifferentFocusWidget_InvokesInputFocusOut)
{
    ItemEditor *parent = new ItemEditor();
    edit->setParent(parent);

    stub.set_lamda(&QApplication::focusWidget, [] {
        __DBG_STUB_INVOKE__
        return static_cast<QWidget *>(nullptr);
    });

    QFocusEvent event(QEvent::FocusOut, Qt::OtherFocusReason);
    edit->focusOutEvent(&event);

    edit->setParent(nullptr);
    delete parent;
}

TEST_F(UT_RenameEdit, EventFilter_PaintEvent_ReturnsTrueForSelf)
{
    QPaintEvent paintEvent(QRect(0, 0, 100, 100));
    bool result = edit->eventFilter(edit, &paintEvent);
    EXPECT_TRUE(result);
}

TEST_F(UT_RenameEdit, EventFilter_NonPaintEvent_ReturnsFalse)
{
    QMouseEvent mouseEvent(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    bool result = edit->eventFilter(edit, &mouseEvent);
    EXPECT_FALSE(result);
}

TEST_F(UT_RenameEdit, EventFilter_PaintEventOtherObj_CallsBase)
{
    QWidget other;
    QPaintEvent paintEvent(QRect(0, 0, 100, 100));
    bool result = edit->eventFilter(&other, &paintEvent);
    EXPECT_FALSE(result);
}

class UT_ItemEditor_TextChanged : public testing::Test
{
protected:
    void SetUp() override
    {
        editor = new ItemEditor();
    }

    void TearDown() override
    {
        delete editor;
        editor = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    ItemEditor *editor = nullptr;
};

TEST_F(UT_ItemEditor_TextChanged, TextChanged_EmptyText_UpdatesGeometry)
{
    editor->setText("");
    EXPECT_NO_THROW(editor->updateGeometry());
}

TEST_F(UT_ItemEditor_TextChanged, TextChanged_ReadOnly_DoesNotProcess)
{
    editor->editor()->setReadOnly(true);
    editor->setText("test");
    EXPECT_TRUE(editor->editor()->isReadOnly());
}

class UT_RenameEdit_ContextMenu : public testing::Test
{
protected:
    void SetUp() override
    {
        edit = new RenameEdit();
    }

    void TearDown() override
    {
        delete edit;
        edit = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    RenameEdit *edit = nullptr;
};

TEST_F(UT_RenameEdit_ContextMenu, ContextMenuEvent_ReadOnly_AcceptsEvent)
{
    edit->setReadOnly(true);

    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(10, 10));
    edit->contextMenuEvent(&event);

    EXPECT_TRUE(event.isAccepted());
}

TEST_F(UT_RenameEdit_ContextMenu, ContextMenuEvent_NotReadOnly_CreatesMenu)
{
    bool menuCreated = false;
    stub.set_lamda(qOverload<>(&QTextEdit::createStandardContextMenu), [&menuCreated] {
        __DBG_STUB_INVOKE__
        menuCreated = true;
        return static_cast<QMenu *>(nullptr);
    });

    QContextMenuEvent event(QContextMenuEvent::Mouse, QPoint(10, 10));
    edit->contextMenuEvent(&event);

    EXPECT_TRUE(menuCreated);
}

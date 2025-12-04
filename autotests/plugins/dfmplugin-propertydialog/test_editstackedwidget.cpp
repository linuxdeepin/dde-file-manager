// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QApplication>
#include <QUrl>
#include <QTextCursor>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTextBlockFormat>
#include "stubext.h"

#include "views/editstackedwidget.h"
#include "dfmplugin_propertydialog_global.h"

DPPROPERTYDIALOG_USE_NAMESPACE

class TestEditStackedWidget : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// Test NameTextEdit class
TEST_F(TestEditStackedWidget, NameTextEditConstructor)
{
    NameTextEdit *textEdit = new NameTextEdit("test");
    EXPECT_NE(textEdit, nullptr);
    delete textEdit;
}

TEST_F(TestEditStackedWidget, NameTextEditDestructor)
{
    NameTextEdit *textEdit = new NameTextEdit("test");
    EXPECT_NO_THROW(delete textEdit);
}

TEST_F(TestEditStackedWidget, NameTextEditIsCanceled)
{
    NameTextEdit textEdit("test");
    EXPECT_FALSE(textEdit.isCanceled());
}

TEST_F(TestEditStackedWidget, NameTextEditSetIsCanceled)
{
    NameTextEdit textEdit("test");
    textEdit.setIsCanceled(true);
    EXPECT_TRUE(textEdit.isCanceled());
    
    textEdit.setIsCanceled(false);
    EXPECT_FALSE(textEdit.isCanceled());
}

TEST_F(TestEditStackedWidget, NameTextEditSetPlainText)
{
    NameTextEdit textEdit("test");
    textEdit.setPlainText("new text");
    
    // Check if text is set correctly
    EXPECT_EQ(textEdit.toPlainText(), "new text");
}

TEST_F(TestEditStackedWidget, NameTextEditSlotTextChanged)
{
    NameTextEdit textEdit("test");
    textEdit.setPlainText("new text");
    
    // Call slot directly
    EXPECT_NO_THROW(textEdit.slotTextChanged());
}

TEST_F(TestEditStackedWidget, NameTextEditFocusOutEvent)
{
    NameTextEdit textEdit("test");
    QFocusEvent event(QEvent::FocusOut);
    EXPECT_NO_THROW(textEdit.focusOutEvent(&event));
}

TEST_F(TestEditStackedWidget, NameTextEditKeyPressEventEscape)
{
    NameTextEdit textEdit("test");
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    EXPECT_NO_THROW(textEdit.keyPressEvent(&event));
    EXPECT_TRUE(textEdit.isCanceled());
}

TEST_F(TestEditStackedWidget, NameTextEditKeyPressEventEnter)
{
    NameTextEdit textEdit("test");
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Enter, Qt::NoModifier);
    EXPECT_NO_THROW(textEdit.keyPressEvent(&event));
    EXPECT_FALSE(textEdit.isCanceled());
}

TEST_F(TestEditStackedWidget, NameTextEditKeyPressEventReturn)
{
    NameTextEdit textEdit("test");
    QKeyEvent event(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    EXPECT_NO_THROW(textEdit.keyPressEvent(&event));
    EXPECT_FALSE(textEdit.isCanceled());
}

// Test EditStackedWidget class
TEST_F(TestEditStackedWidget, EditStackedWidgetConstructor)
{
    EditStackedWidget *stackedWidget = new EditStackedWidget();
    EXPECT_NE(stackedWidget, nullptr);
    delete stackedWidget;
}

TEST_F(TestEditStackedWidget, EditStackedWidgetDestructor)
{
    EditStackedWidget *stackedWidget = new EditStackedWidget();
    EXPECT_NO_THROW(delete stackedWidget);
}

TEST_F(TestEditStackedWidget, EditStackedWidgetInitTextShowFrame)
{
    EditStackedWidget stackedWidget;
    EXPECT_NO_THROW(stackedWidget.initTextShowFrame("test file"));
}

TEST_F(TestEditStackedWidget, EditStackedWidgetRenameFile)
{
    EditStackedWidget stackedWidget;
    stackedWidget.selectFile(QUrl::fromLocalFile("/tmp/test.txt"));
    // EXPECT_NO_THROW(stackedWidget.renameFile());
}

TEST_F(TestEditStackedWidget, EditStackedWidgetShowTextShowFrame)
{
    EditStackedWidget stackedWidget;
    stackedWidget.selectFile(QUrl::fromLocalFile("/tmp/test.txt"));
    EXPECT_NO_THROW(stackedWidget.showTextShowFrame());
}

TEST_F(TestEditStackedWidget, EditStackedWidgetSelectFile)
{
    EditStackedWidget stackedWidget;
    EXPECT_NO_THROW(stackedWidget.selectFile(QUrl::fromLocalFile("/tmp/test.txt")));
}

TEST_F(TestEditStackedWidget, EditStackedWidgetMouseProcess)
{
    EditStackedWidget stackedWidget;
    stackedWidget.selectFile(QUrl::fromLocalFile("/tmp/test.txt"));
    // stackedWidget.renameFile(); // Switch to edit mode
    
    QMouseEvent event(QEvent::MouseButtonPress, QPointF(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    EXPECT_NO_THROW(stackedWidget.mouseProcess(&event));
}
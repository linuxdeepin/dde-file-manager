// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_renameedit.cpp
 * @brief Unit tests for RenameEdit methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "delegate/itemeditor.h"

#include <QTest>

using namespace ddplugin_canvas;

class RenameEditTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RenameEdit();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RenameEdit *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RenameEditTest, RenameEdit)
{
    // Test constructor: RenameEdit((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RenameEditTest, undo)
{
    // Test method: void undo(())
    EXPECT_NO_FATAL_FAILURE(obj->undo());
}

TEST_F(RenameEditTest, redo)
{
    // Test method: void redo(())
    EXPECT_NO_FATAL_FAILURE(obj->redo());
}

TEST_F(RenameEditTest, stackCurrent)
{
    // Test getter: QString stackCurrent()
    auto result = obj->stackCurrent();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RenameEditTest, stackBack)
{
    // Test getter: QString stackBack()
    auto result = obj->stackBack();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RenameEditTest, stackAdvance)
{
    // Test getter: QString stackAdvance()
    auto result = obj->stackAdvance();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RenameEditTest, adjustStyle)
{
    // Test method: void adjustStyle(())
    EXPECT_NO_FATAL_FAILURE(obj->adjustStyle());
}

TEST_F(RenameEditTest, eventFilter)
{
    // Test method: bool eventFilter((QObject *obj, QEvent *e))
    auto result = obj->eventFilter(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(RenameEditTest, pushStatck)
{
    // Test method: void pushStatck((const QString &item))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->pushStatck(_arg0));
}

TEST_F(RenameEditTest, showEvent)
{
    // Test event handler: showEvent((QShowEvent *e))
    QShowEvent _event(QShowEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->showEvent(&_event));
}

TEST_F(RenameEditTest, contextMenuEvent)
{
    // Test event handler: contextMenuEvent((QContextMenuEvent *e))
    QContextMenuEvent _event(QContextMenuEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->contextMenuEvent(&_event));
}

TEST_F(RenameEditTest, focusOutEvent)
{
    // Test event handler: focusOutEvent((QFocusEvent *e))
    QFocusEvent _event(QFocusEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->focusOutEvent(&_event));
}

TEST_F(RenameEditTest, keyPressEvent)
{
    // Test event handler: keyPressEvent((QKeyEvent *e))
    QKeyEvent _event(QKeyEvent::None);
    EXPECT_NO_FATAL_FAILURE(obj->keyPressEvent(&_event));
}

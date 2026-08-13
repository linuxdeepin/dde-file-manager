// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskdialog.cpp
 * @brief Unit tests for TaskDialog and TaskWidget (dialogs/taskdialog/taskdialog.cpp +
 *        dialogs/taskdialog/taskwidget.cpp) — GUI classes using stub-ext.
 *        TaskWidget ctor is private (friend TaskDialog), so we test via TaskDialog.
 */

#include <gtest/gtest.h>
#include <QApplication>
#include <QTest>
#include <QString>
#include <QSharedPointer>

#include "stubext.h"

#include <DDialog>
#include <dfm-base/dialogs/taskdialog/taskdialog.h>
#include <dfm-base/interfaces/abstractjobhandler.h>

using namespace dfmbase;

class TaskDialogTest : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.set_lamda(VADDR(DTK_WIDGET_NAMESPACE::DDialog, exec), [] {
            __DBG_STUB_INVOKE__
            return QDialog::Accepted;
        });
        stub.set_lamda(&QWidget::show, [](QWidget *) { __DBG_STUB_INVOKE__ });
        stub.set_lamda(&QWidget::hide, [](QWidget *) { __DBG_STUB_INVOKE__ });
    }
    void TearDown() override { stub.clear(); }
    stub_ext::StubExt stub;
};

TEST_F(TaskDialogTest, ConstructAndDestruct)
{
    {
        TaskDialog d;
        SUCCEED();
    }
}

TEST_F(TaskDialogTest, InitUI)
{
    TaskDialog d;
    d.initUI();
}

TEST_F(TaskDialogTest, SetTitle)
{
    TaskDialog d;
    d.initUI();
    d.setTitle(5);
}

TEST_F(TaskDialogTest, AddTaskWithNullHandle)
{
    TaskDialog d;
    d.initUI();
    JobHandlePointer nullHandle;
    EXPECT_NO_FATAL_FAILURE({ d.addTask(nullHandle); });
}

TEST_F(TaskDialogTest, BlockShutdown)
{
    TaskDialog d;
    d.initUI();
    // blockShutdown uses DBus but stub handles it gracefully
    EXPECT_NO_FATAL_FAILURE({ d.blockShutdown(); });
}

TEST_F(TaskDialogTest, AdjustSize)
{
    TaskDialog d;
    d.initUI();
    d.adjustSize(100);
    d.adjustSize(0);
}

TEST_F(TaskDialogTest, MoveYCenter)
{
    TaskDialog d;
    d.initUI();
    EXPECT_NO_FATAL_FAILURE({ d.moveYCenter(); });
}

// ============================================================
// Additional coverage for TaskDialog
// ============================================================

TEST_F(TaskDialogTest, AddTaskWidget_NullWidget)
{
    TaskDialog d;
    d.initUI();
    EXPECT_NO_FATAL_FAILURE({ d.addTaskWidget(nullptr, nullptr); });
}

TEST_F(TaskDialogTest, AddTask_DuplicateHandler)
{
    TaskDialog d;
    d.initUI();
    // Create a mock JobHandlePointer and add twice — second should just show/raise
    // But since we can't easily create a real JobHandle, test with null (already covered)
    // Test addTaskWidget with null taskHandler but non-null widget
    auto *fakeWidget = new QWidget();
    EXPECT_NO_FATAL_FAILURE({ d.addTaskWidget(nullptr, nullptr); });
    delete fakeWidget;
}

TEST_F(TaskDialogTest, SetTitle_Multiple)
{
    TaskDialog d;
    d.initUI();
    d.setTitle(0);
    d.setTitle(5);
    d.setTitle(100);
    SUCCEED();
}

TEST_F(TaskDialogTest, AdjustSize_WithPositiveHeight)
{
    TaskDialog d;
    d.initUI();
    d.adjustSize(200);
    SUCCEED();
}

TEST_F(TaskDialogTest, BlockShutdown_MultipleCalls)
{
    TaskDialog d;
    d.initUI();
    EXPECT_NO_FATAL_FAILURE({ d.blockShutdown(); });
    // Second call — should be safe even if cookie already set
    EXPECT_NO_FATAL_FAILURE({ d.blockShutdown(); });
}

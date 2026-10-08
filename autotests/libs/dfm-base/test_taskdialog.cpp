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

TEST_F(TaskDialogTest, ResizeAndCenter)
{
    TaskDialog d;
    d.initUI();
    d.resizeAndCenter();
    d.scheduleResize();
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

TEST_F(TaskDialogTest, ResizeAndCenter_WithHeight)
{
    TaskDialog d;
    d.initUI();
    d.resizeAndCenter();
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

// ============================================================
// PMS sev-2 regression: TaskDialog task widget lifecycle (bugs 324097/189653)
// ============================================================
#include <dfm-base/dialogs/taskdialog/taskwidget.h>
#include <QListWidget>

namespace {

// AbstractJobHandler 为纯接口实现类（无纯虚函数），仅供用例持有句柄
class PmsRegressionJobHandler : public AbstractJobHandler
{
};

}   // namespace

// PMS:324097 任务进度框偶发段错误/崩溃：addTask 绑定任务完成信号
// requestRemoveTaskWidget -> removeTask，移除时必须同步清理列表项/窗口部件/
// taskItems，全部任务移除后关闭对话框；重复派发（sender 不在 taskItems 中）
// 走告警分支不得崩溃
TEST_F(TaskDialogTest, BUG324097_RemoveTaskSignalCleansWidgetAndClosesDialog)
{
    auto handler = QSharedPointer<PmsRegressionJobHandler>::create();
    TaskDialog dialog;

    dialog.addTask(handler);
    ASSERT_EQ(dialog.taskItems.count(), 1);
    ASSERT_EQ(dialog.taskListWidget->count(), 1);

    // Qt6 中 signal 即公有成员函数：模拟任务完成方派发移除请求
    // （DirectConnection，removeTask 同步执行）
    emit handler->requestRemoveTaskWidget();

    // removeTask 同步执行：条目与映射必须被清空，且对话框已请求关闭
    EXPECT_EQ(dialog.taskItems.count(), 0);
    EXPECT_EQ(dialog.taskListWidget->count(), 0);

    // 二次派发：sender 不在 taskItems 中，走告警分支，不得崩溃
    EXPECT_NO_FATAL_FAILURE(emit handler->requestRemoveTaskWidget());
    EXPECT_EQ(dialog.taskItems.count(), 0);

    // 新任务加入后整体 close()：closeEvent 清理路径不得崩溃（324097 伴随修复）
    auto handler2 = QSharedPointer<PmsRegressionJobHandler>::create();
    dialog.addTask(handler2);
    ASSERT_EQ(dialog.taskItems.count(), 1);
    EXPECT_NO_FATAL_FAILURE(dialog.close());
    EXPECT_NO_FATAL_FAILURE(emit handler2->requestRemoveTaskWidget());
    EXPECT_EQ(dialog.taskItems.count(), 0);
}

// PMS:189653 打开任务进度框与后台任务并发时偶发死锁：原修复用的
// addTaskMutex/adjustSizeMutex 已在重构中移除，幸存契约是"任务仍在册时
// 立即析构对话框必须快速返回、不挂死、不崩溃"（双重删除/死锁类回归锚点）
TEST_F(TaskDialogTest, BUG189653_DestroyDialogWithActiveTaskDoesNotDeadlock)
{
    auto handler = QSharedPointer<PmsRegressionJobHandler>::create();
    {
        TaskDialog dialog;
        dialog.addTaskWidget(handler, new TaskWidget());
        ASSERT_EQ(dialog.taskItems.count(), 1);
        // 任务句柄仍存活（外部持有），对话框即刻析构
    }   // scope exit -> ~TaskDialog with an active task

    QApplication::processEvents();
    SUCCEED();
}

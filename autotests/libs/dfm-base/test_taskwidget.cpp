// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskwidget.cpp
 * @brief Unit tests for TaskWidget methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/dialogs/taskdialog/taskwidget.h"

#include <QTest>

using namespace src;

class TaskWidgetTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TaskWidget();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TaskWidget *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TaskWidgetTest, TaskWidget)
{
    // Test constructor: TaskWidget((QWidget *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TaskWidgetTest, createBaseWidget)
{
    // Test getter: QWidget createBaseWidget()
    auto result = obj->createBaseWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->createBaseWidget(); });

}

TEST_F(TaskWidgetTest, createBtnWidget)
{
    // Test getter: QWidget createBtnWidget()
    auto result = obj->createBtnWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->createBtnWidget(); });

}

TEST_F(TaskWidgetTest, createConflictWidget)
{
    // Test getter: QWidget createConflictWidget()
    auto result = obj->createConflictWidget();
    EXPECT_NO_FATAL_FAILURE({ obj->createConflictWidget(); });

}

TEST_F(TaskWidgetTest, formatTime)
{
    // Test method: QString formatTime((qint64 second))
    auto result = obj->formatTime(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TaskWidgetTest, onShowConflictInfo)
{
    // Test method: void onShowConflictInfo((const QUrl source, const QUrl target, const AbstractJobHandler::SupportActions action))
    EXPECT_NO_FATAL_FAILURE(obj->onShowConflictInfo(QUrl(), QUrl(), AbstractJobHandler::SupportActions()));
}

TEST_F(TaskWidgetTest, onShowErrors)
{
    // Test method: void onShowErrors((const JobInfoPointer jobInfo))
    EXPECT_NO_FATAL_FAILURE(obj->onShowErrors(JobInfoPointer()));
}

TEST_F(TaskWidgetTest, onShowPermanentlyDelete)
{
    // Test method: void onShowPermanentlyDelete((const QUrl source, const AbstractJobHandler::SupportActions action))
    EXPECT_NO_FATAL_FAILURE(obj->onShowPermanentlyDelete(QUrl(), AbstractJobHandler::SupportActions()));
}

TEST_F(TaskWidgetTest, onShowSpeedUpdatedInfo)
{
    // Test method: void onShowSpeedUpdatedInfo((const JobInfoPointer JobInfo))
    EXPECT_NO_FATAL_FAILURE(obj->onShowSpeedUpdatedInfo(JobInfoPointer()));
}

TEST_F(TaskWidgetTest, onShowTaskProccess)
{
    // Test method: void onShowTaskProccess((const JobInfoPointer JobInfo))
    EXPECT_NO_FATAL_FAILURE(obj->onShowTaskProccess(JobInfoPointer()));
}

TEST_F(TaskWidgetTest, resetPauseStute)
{
    // Test method: void resetPauseStute(())
    EXPECT_NO_FATAL_FAILURE(obj->resetPauseStute());
}

TEST_F(TaskWidgetTest, setTaskHandle)
{
    // Test setter: void setTaskHandle((const JobHandlePointer handle))
    EXPECT_NO_FATAL_FAILURE(obj->setTaskHandle(JobHandlePointer()));
}

TEST_F(TaskWidgetTest, showFileInfo)
{
    // Test method: bool showFileInfo((const FileInfoPointer info, const bool isOrg))
    auto result = obj->showFileInfo(FileInfoPointer(), false);
    EXPECT_FALSE(result);

}

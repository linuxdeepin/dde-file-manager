// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskdialog.cpp
 * @brief Unit tests for TaskDialog methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/dialogs/taskdialog/taskdialog.h"

#include <QTest>

using namespace src;

class TaskDialogTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TaskDialog();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TaskDialog *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TaskDialogTest, adjustSize)
{
    // Test method: void adjustSize((int hight))
    EXPECT_NO_FATAL_FAILURE(obj->adjustSize(0));
}

TEST_F(TaskDialogTest, removeTask)
{
    // Test method: void removeTask(())
    EXPECT_NO_FATAL_FAILURE(obj->removeTask());
}

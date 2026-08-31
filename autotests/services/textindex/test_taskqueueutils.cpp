// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskqueueutils.cpp
 * @brief Unit tests for TaskQueueUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/taskqueueutils.h"

#include <QTest>

using namespace src;

class TaskQueueUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TaskQueueUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TaskQueueUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TaskQueueUtilsTest, buildDirectoryMovePlans)
{
    // Test method: QList<TaskQueueUtils::DirectoryMovePlan> buildDirectoryMovePlans((const QHash<QString, QString> &movedFiles))
    QHash<QString, QString> _arg0{};
    auto result = obj->buildDirectoryMovePlans(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TaskQueueUtilsTest, rewriteQueuedTasksForDirectoryMove)
{
    // Test method: bool rewriteQueuedTasksForDirectoryMove((QQueue<TaskQueueItem> &taskQueue,
                                                        const QString &fromPath,
                                                        const QString &toPath))
    QQueue<TaskQueueItem> _arg0{};
    QString _arg1{};
    QString _arg2{};
    auto result = obj->rewriteQueuedTasksForDirectoryMove(_arg0, _arg1, _arg2);
    EXPECT_FALSE(result);

}

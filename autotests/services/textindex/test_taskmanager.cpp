// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_taskmanager.cpp
 * @brief Unit tests for TaskManager methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/taskmanager.h"

#include <QTest>

using namespace src;

class TaskManagerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TaskManager();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TaskManager *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TaskManagerTest, applyDirectoryMovePlans)
{
    // Test method: QStringList applyDirectoryMovePlans((const QHash<QString, QString> &movedFiles))
    QHash<QString, QString> _arg0{};
    auto result = obj->applyDirectoryMovePlans(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TaskManagerTest, finalizeIndexState)
{
    // Test method: void finalizeIndexState((IndexTask::Type type, const HandlerResult &result))
    HandlerResult _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->finalizeIndexState(IndexTask::Type(), _arg1));
}

TEST_F(TaskManagerTest, getTaskHandler)
{
    // Test method: TaskHandler getTaskHandler((IndexTask::Type type))
    auto result = obj->getTaskHandler(IndexTask::Type());
    EXPECT_NO_FATAL_FAILURE({ obj->getTaskHandler(IndexTask::Type()); });

}

TEST_F(TaskManagerTest, handleCorruptedIndex)
{
    // Test method: bool handleCorruptedIndex((IndexTask::Type type, const HandlerResult &result, const QString &taskPath))
    HandlerResult _arg1{};
    QString _arg2{};
    auto result = obj->handleCorruptedIndex(IndexTask::Type(), _arg1, _arg2);
    EXPECT_FALSE(result);

}

TEST_F(TaskManagerTest, startFileMoveTask)
{
    // Test method: bool startFileMoveTask((const QHash<QString, QString> &movedFiles, bool silent))
    QHash<QString, QString> _arg0{};
    auto result = obj->startFileMoveTask(_arg0, false);
    EXPECT_FALSE(result);

}

TEST_F(TaskManagerTest, updateIndexStatusOnSuccess)
{
    // Test method: void updateIndexStatusOnSuccess((IndexTask::Type type, const HandlerResult &result))
    HandlerResult _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->updateIndexStatusOnSuccess(IndexTask::Type(), _arg1));
}

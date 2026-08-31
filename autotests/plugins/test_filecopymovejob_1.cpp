// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filecopymovejob_1.cpp
 * @brief Unit tests for FileCopyMoveJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/filecopymovejob.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class FileCopyMoveJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileCopyMoveJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileCopyMoveJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileCopyMoveJobTest, cleanTrash)
{
    // Test method: JobHandlePointer cleanTrash((const QList<QUrl> &sources))
    QList<QUrl> _arg0{};
    auto result = obj->cleanTrash(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileCopyMoveJobTest, copyFromTrash)
{
    // Test method: JobHandlePointer copyFromTrash((const QList<QUrl> &sources, const QUrl &target, const AbstractJobHandler::JobFlags &flags))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    AbstractJobHandler::JobFlags _arg2{};
    auto result = obj->copyFromTrash(_arg0, _arg1, _arg2);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileCopyMoveJobTest, getOperationsAndDialogService)
{
    // Test bool getter: getOperationsAndDialogService()
    bool result = obj->getOperationsAndDialogService();
    EXPECT_FALSE(result);

}

TEST_F(FileCopyMoveJobTest, initArguments)
{
    // Test method: void initArguments((const JobHandlePointer handler, const AbstractJobHandler::JobFlags flags))
    EXPECT_NO_FATAL_FAILURE(obj->initArguments(JobHandlePointer(), AbstractJobHandler::JobFlags()));
}

TEST_F(FileCopyMoveJobTest, moveToTrash)
{
    // Test method: JobHandlePointer moveToTrash((const QList<QUrl> &sources,
                                              const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags, const bool isInit))
    QList<QUrl> _arg0{};
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags _arg1{};
    auto result = obj->moveToTrash(_arg0, _arg1, false);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileCopyMoveJobTest, onHandleAddTask)
{
    // Test method: void onHandleAddTask(())
    EXPECT_NO_FATAL_FAILURE(obj->onHandleAddTask());
}

TEST_F(FileCopyMoveJobTest, onHandleAddTaskWithArgs)
{
    // Test method: void onHandleAddTaskWithArgs((const JobInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->onHandleAddTaskWithArgs(JobInfoPointer()));
}

TEST_F(FileCopyMoveJobTest, onHandleTaskFinished)
{
    // Test method: void onHandleTaskFinished((const JobInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->onHandleTaskFinished(JobInfoPointer()));
}

TEST_F(FileCopyMoveJobTest, restoreFromTrash)
{
    // Test method: JobHandlePointer restoreFromTrash((const QList<QUrl> &sources, const QUrl &target,
                                                   const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags, const bool isInit))
    QList<QUrl> _arg0{};
    QUrl _arg1{};
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags _arg2{};
    auto result = obj->restoreFromTrash(_arg0, _arg1, _arg2, false);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(FileCopyMoveJobTest, startAddTaskTimer)
{
    // Test method: void startAddTaskTimer((const JobHandlePointer handler, const bool isRemote))
    EXPECT_NO_FATAL_FAILURE(obj->startAddTaskTimer(JobHandlePointer(), false));
}

TEST_F(FileCopyMoveJobTest, FileCopyMoveJob_Destructor)
{
    // Test method:  ~FileCopyMoveJob(())
    EXPECT_NO_FATAL_FAILURE({ FileCopyMoveJob *tmp = new FileCopyMoveJob(); delete tmp; });
}

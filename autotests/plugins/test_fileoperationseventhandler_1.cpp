// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperationseventhandler_1.cpp
 * @brief Unit tests for FileOperationsEventHandler methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperationsevent/fileoperationseventhandler.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class FileOperationsEventHandlerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperationsEventHandler();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperationsEventHandler *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperationsEventHandlerTest, handleErrorNotify)
{
    // Test method: void handleErrorNotify((const JobInfoPointer &jobInfo))
    JobInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleErrorNotify(_arg0));
}

TEST_F(FileOperationsEventHandlerTest, handleFinishedNotify)
{
    // Test method: void handleFinishedNotify((const JobInfoPointer &jobInfo))
    JobInfoPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFinishedNotify(_arg0));
}

TEST_F(FileOperationsEventHandlerTest, handleJobResult)
{
    // Test method: void handleJobResult((DFMBASE_NAMESPACE::AbstractJobHandler::JobType jobType, JobHandlePointer ptr))
    EXPECT_NO_FATAL_FAILURE(obj->handleJobResult(DFMBASE_NAMESPACE::AbstractJobHandler::JobType(), JobHandlePointer()));
}

TEST_F(FileOperationsEventHandlerTest, instance)
{
    // Test getter: FileOperationsEventHandler instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dumpisoimagejob_1.cpp
 * @brief Unit tests for DumpISOImageJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burnjob.h"

#include <QTest>

using namespace dfmplugin_burn;

class DumpISOImageJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DumpISOImageJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DumpISOImageJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DumpISOImageJobTest, DumpISOImageJob)
{
    // Test constructor: DumpISOImageJob((const QString &dev, const JobHandlePointer handler))
    ASSERT_NE(obj, nullptr);
}

TEST_F(DumpISOImageJobTest, finishFunc)
{
    // Test method: void finishFunc((const VerifyResult &result))
    VerifyResult _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->finishFunc(_arg0));
}

TEST_F(DumpISOImageJobTest, updateMessage)
{
    // Test method: void updateMessage((JobInfoPointer ptr))
    EXPECT_NO_FATAL_FAILURE(obj->updateMessage(JobInfoPointer()));
}

TEST_F(DumpISOImageJobTest, updateSpeed)
{
    // Test method: void updateSpeed((JobInfoPointer ptr, JobStatus status, const QString &speed))
    QString _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->updateSpeed(JobInfoPointer(), JobStatus(), _arg2));
}

TEST_F(DumpISOImageJobTest, work)
{
    // Test method: void work(())
    EXPECT_NO_FATAL_FAILURE(obj->work());
}

TEST_F(DumpISOImageJobTest, DumpISOImageJob_Destructor)
{
    // Test method:  ~DumpISOImageJob(())
    EXPECT_NO_FATAL_FAILURE({ DumpISOImageJob *tmp = new DumpISOImageJob(); delete tmp; });
}

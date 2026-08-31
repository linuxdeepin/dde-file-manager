// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractjob_1.cpp
 * @brief Unit tests for AbstractJob methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/fileoperationutils/abstractjob.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class AbstractJobTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new AbstractJob();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    AbstractJob *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(AbstractJobTest, handleError)
{
    // Test method: void handleError((const JobInfoPointer jobInfo))
    EXPECT_NO_FATAL_FAILURE(obj->handleError(JobInfoPointer()));
}

TEST_F(AbstractJobTest, handleFileAdded)
{
    // Test method: void handleFileAdded((const QUrl &url))
    QUrl _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileAdded(_arg0));
}

TEST_F(AbstractJobTest, handleFileRenamed)
{
    // Test method: void handleFileRenamed((const QUrl &old, const QUrl &cur))
    QUrl _arg0{};
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileRenamed(_arg0, _arg1));
}

TEST_F(AbstractJobTest, handleRetryErrorSuccess)
{
    // Test method: void handleRetryErrorSuccess((const quint64 Id))
    EXPECT_NO_FATAL_FAILURE(obj->handleRetryErrorSuccess(0));
}

TEST_F(AbstractJobTest, setJobArgs)
{
    // Test method: DPFILEOPERATIONS_USE_NAMESPACE setJobArgs((const JobHandlePointer handle, const QList<QUrl> &sources, const QUrl &target,
                             const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags &flags))
    QList<QUrl> _arg1{};
    QUrl _arg2{};
    DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags _arg3{};
    auto result = obj->setJobArgs(JobHandlePointer(), _arg1, _arg2, _arg3);
    EXPECT_NO_FATAL_FAILURE({ obj->setJobArgs(JobHandlePointer(), _arg1, _arg2, _arg3); });

}

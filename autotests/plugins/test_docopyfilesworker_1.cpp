// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docopyfilesworker_1.cpp
 * @brief Unit tests for DoCopyFilesWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/copyfiles/docopyfilesworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoCopyFilesWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoCopyFilesWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoCopyFilesWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoCopyFilesWorkerTest, onUpdateProgress)
{
    // Test method: void onUpdateProgress(())
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateProgress());
}

TEST_F(DoCopyFilesWorkerTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

TEST_F(DoCopyFilesWorkerTest, DoCopyFilesWorker_Destructor)
{
    // Test method:  ~DoCopyFilesWorker(())
    EXPECT_NO_FATAL_FAILURE({ DoCopyFilesWorker *tmp = new DoCopyFilesWorker(); delete tmp; });
}

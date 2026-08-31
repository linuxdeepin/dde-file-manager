// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docopyfilesworker.cpp
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

TEST_F(DoCopyFilesWorkerTest, doWork)
{
    // Test bool getter: doWork()
    bool result = obj->doWork();
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFilesWorkerTest, endWork)
{
    // Test method: void endWork(())
    EXPECT_NO_FATAL_FAILURE(obj->endWork());
}

TEST_F(DoCopyFilesWorkerTest, initArgs)
{
    // Test bool getter: initArgs()
    bool result = obj->initArgs();
    EXPECT_FALSE(result);

}

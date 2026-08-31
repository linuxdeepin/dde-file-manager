// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docopyfromtrashfilesworker_1.cpp
 * @brief Unit tests for DoCopyFromTrashFilesWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/trashfiles/docopyfromtrashfilesworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoCopyFromTrashFilesWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoCopyFromTrashFilesWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoCopyFromTrashFilesWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoCopyFromTrashFilesWorkerTest, doWork)
{
    // Test bool getter: doWork()
    bool result = obj->doWork();
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFromTrashFilesWorkerTest, initArgs)
{
    // Test bool getter: initArgs()
    bool result = obj->initArgs();
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFromTrashFilesWorkerTest, statisticsFilesSize)
{
    // Test bool getter: statisticsFilesSize()
    bool result = obj->statisticsFilesSize();
    EXPECT_FALSE(result);

}

TEST_F(DoCopyFromTrashFilesWorkerTest, DoCopyFromTrashFilesWorker_Destructor)
{
    // Test method:  ~DoCopyFromTrashFilesWorker(())
    EXPECT_NO_FATAL_FAILURE({ DoCopyFromTrashFilesWorker *tmp = new DoCopyFromTrashFilesWorker(); delete tmp; });
}

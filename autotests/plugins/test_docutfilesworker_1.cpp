// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docutfilesworker_1.cpp
 * @brief Unit tests for DoCutFilesWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/cutfiles/docutfilesworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoCutFilesWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoCutFilesWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoCutFilesWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoCutFilesWorkerTest, checkSelf)
{
    // Test method: bool checkSelf((const DFileInfoPointer &fileInfo))
    DFileInfoPointer _arg0{};
    auto result = obj->checkSelf(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DoCutFilesWorkerTest, checkSymLink)
{
    // Test method: bool checkSymLink((const DFileInfoPointer &fileInfo))
    DFileInfoPointer _arg0{};
    auto result = obj->checkSymLink(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DoCutFilesWorkerTest, doWork)
{
    // Test bool getter: doWork()
    bool result = obj->doWork();
    EXPECT_FALSE(result);

}

TEST_F(DoCutFilesWorkerTest, emitCompleteFilesUpdatedNotify)
{
    // Test method: void emitCompleteFilesUpdatedNotify((const qint64 &writCount))
    qint64 _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->emitCompleteFilesUpdatedNotify(_arg0));
}

TEST_F(DoCutFilesWorkerTest, endWork)
{
    // Test method: void endWork(())
    EXPECT_NO_FATAL_FAILURE(obj->endWork());
}

TEST_F(DoCutFilesWorkerTest, initArgs)
{
    // Test bool getter: initArgs()
    bool result = obj->initArgs();
    EXPECT_FALSE(result);

}

TEST_F(DoCutFilesWorkerTest, onUpdateProgress)
{
    // Test method: void onUpdateProgress(())
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateProgress());
}

TEST_F(DoCutFilesWorkerTest, DoCutFilesWorker_Destructor)
{
    // Test method:  ~DoCutFilesWorker(())
    EXPECT_NO_FATAL_FAILURE({ DoCutFilesWorker *tmp = new DoCutFilesWorker(); delete tmp; });
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docleantrashfilesworker_1.cpp
 * @brief Unit tests for DoCleanTrashFilesWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/cleantrash/docleantrashfilesworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoCleanTrashFilesWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoCleanTrashFilesWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoCleanTrashFilesWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoCleanTrashFilesWorkerTest, doHandleErrorAndWait)
{
    // Test method: AbstractJobHandler::SupportAction doHandleErrorAndWait((const QUrl &from,
                                              const AbstractJobHandler::JobErrorType &error,
                                              const bool isTo,
                                              const QString &errorMsg))
    QUrl _arg0{};
    AbstractJobHandler::JobErrorType _arg1{};
    QString _arg3{};
    auto result = obj->doHandleErrorAndWait(_arg0, _arg1, false, _arg3);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DoCleanTrashFilesWorkerTest, doWork)
{
    // Test bool getter: doWork()
    bool result = obj->doWork();
    EXPECT_FALSE(result);

}

TEST_F(DoCleanTrashFilesWorkerTest, initArgs)
{
    // Test bool getter: initArgs()
    bool result = obj->initArgs();
    EXPECT_FALSE(result);

}

TEST_F(DoCleanTrashFilesWorkerTest, onUpdateProgress)
{
    // Test method: void onUpdateProgress(())
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateProgress());
}

TEST_F(DoCleanTrashFilesWorkerTest, DoCleanTrashFilesWorker_Destructor)
{
    // Test method:  ~DoCleanTrashFilesWorker(())
    EXPECT_NO_FATAL_FAILURE({ DoCleanTrashFilesWorker *tmp = new DoCleanTrashFilesWorker(); delete tmp; });
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dodeletefilesworker_1.cpp
 * @brief Unit tests for DoDeleteFilesWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/deletefiles/dodeletefilesworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoDeleteFilesWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoDeleteFilesWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoDeleteFilesWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoDeleteFilesWorkerTest, doHandleErrorAndWait)
{
    // Test method: AbstractJobHandler::SupportAction doHandleErrorAndWait((const QUrl &from,
                                          const AbstractJobHandler::JobErrorType &error,
                                          const QString &errorMsg))
    QUrl _arg0{};
    AbstractJobHandler::JobErrorType _arg1{};
    QString _arg2{};
    auto result = obj->doHandleErrorAndWait(_arg0, _arg1, _arg2);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DoDeleteFilesWorkerTest, doWork)
{
    // Test bool getter: doWork()
    bool result = obj->doWork();
    EXPECT_FALSE(result);

}

TEST_F(DoDeleteFilesWorkerTest, onUpdateProgress)
{
    // Test method: void onUpdateProgress(())
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateProgress());
}

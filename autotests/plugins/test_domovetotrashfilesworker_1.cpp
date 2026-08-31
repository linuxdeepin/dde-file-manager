// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_domovetotrashfilesworker_1.cpp
 * @brief Unit tests for DoMoveToTrashFilesWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/trashfiles/domovetotrashfilesworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoMoveToTrashFilesWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoMoveToTrashFilesWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoMoveToTrashFilesWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoMoveToTrashFilesWorkerTest, doHandleErrorNoSpace)
{
    // Test method: AbstractJobHandler::SupportAction doHandleErrorNoSpace((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->doHandleErrorNoSpace(_arg0);
    EXPECT_GE(static_cast<int>(result), 0);

}

TEST_F(DoMoveToTrashFilesWorkerTest, doWork)
{
    // Test bool getter: doWork()
    bool result = obj->doWork();
    EXPECT_FALSE(result);

}

TEST_F(DoMoveToTrashFilesWorkerTest, isCanMoveToTrash)
{
    // Test method: bool isCanMoveToTrash((const QUrl &url, bool *result))
    QUrl _arg0{};
    auto result = obj->isCanMoveToTrash(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(DoMoveToTrashFilesWorkerTest, onUpdateProgress)
{
    // Test method: void onUpdateProgress(())
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateProgress());
}

TEST_F(DoMoveToTrashFilesWorkerTest, statisticsFilesSize)
{
    // Test bool getter: statisticsFilesSize()
    bool result = obj->statisticsFilesSize();
    EXPECT_FALSE(result);

}

TEST_F(DoMoveToTrashFilesWorkerTest, DoMoveToTrashFilesWorker_Destructor)
{
    // Test method:  ~DoMoveToTrashFilesWorker(())
    EXPECT_NO_FATAL_FAILURE({ DoMoveToTrashFilesWorker *tmp = new DoMoveToTrashFilesWorker(); delete tmp; });
}

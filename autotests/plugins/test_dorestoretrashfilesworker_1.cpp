// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dorestoretrashfilesworker_1.cpp
 * @brief Unit tests for DoRestoreTrashFilesWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/trashfiles/dorestoretrashfilesworker.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DoRestoreTrashFilesWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DoRestoreTrashFilesWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DoRestoreTrashFilesWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DoRestoreTrashFilesWorkerTest, checkRestoreInfo)
{
    // Test method: DFileInfoPointer checkRestoreInfo((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->checkRestoreInfo(_arg0);
    EXPECT_NE(result.get(), nullptr);

}

TEST_F(DoRestoreTrashFilesWorkerTest, doWork)
{
    // Test bool getter: doWork()
    bool result = obj->doWork();
    EXPECT_FALSE(result);

}

TEST_F(DoRestoreTrashFilesWorkerTest, initArgs)
{
    // Test bool getter: initArgs()
    bool result = obj->initArgs();
    EXPECT_FALSE(result);

}

TEST_F(DoRestoreTrashFilesWorkerTest, mergeDir)
{
    // Test method: bool mergeDir((const QUrl &urlSource, const QUrl &urlTarget, DFile::CopyFlag flag))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->mergeDir(_arg0, _arg1, DFile::CopyFlag());
    EXPECT_FALSE(result);

}

TEST_F(DoRestoreTrashFilesWorkerTest, onUpdateProgress)
{
    // Test method: void onUpdateProgress(())
    EXPECT_NO_FATAL_FAILURE(obj->onUpdateProgress());
}

TEST_F(DoRestoreTrashFilesWorkerTest, DoRestoreTrashFilesWorker_Destructor)
{
    // Test method:  ~DoRestoreTrashFilesWorker(())
    EXPECT_NO_FATAL_FAILURE({ DoRestoreTrashFilesWorker *tmp = new DoRestoreTrashFilesWorker(); delete tmp; });
}

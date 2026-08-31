// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docleantrashfilesworker.cpp
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

TEST_F(DoCleanTrashFilesWorkerTest, clearTrashFile)
{
    // Test method: bool clearTrashFile((const FileInfoPointer &trashInfo))
    FileInfoPointer _arg0{};
    auto result = obj->clearTrashFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(DoCleanTrashFilesWorkerTest, deleteFile)
{
    // Test method: bool deleteFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->deleteFile(_arg0);
    EXPECT_FALSE(result);

}

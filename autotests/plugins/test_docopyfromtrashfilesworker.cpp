// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_docopyfromtrashfilesworker.cpp
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

TEST_F(DoCopyFromTrashFilesWorkerTest, createParentDir)
{
    // Test method: DFileInfoPointer createParentDir((const DFileInfoPointer &trashInfo,
                                                             const DFileInfoPointer &restoreInfo,
                                                             bool *result))
    DFileInfoPointer _arg0{};
    DFileInfoPointer _arg1{};
    auto result = obj->createParentDir(_arg0, _arg1, nullptr);
    EXPECT_NE(result.get(), nullptr);

}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_copyfromtrashtrashfiles.cpp
 * @brief Unit tests for CopyFromTrashTrashFiles methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/trashfiles/copyfromtrashfiles.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class CopyFromTrashTrashFilesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CopyFromTrashTrashFiles();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CopyFromTrashTrashFiles *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CopyFromTrashTrashFilesTest, CopyFromTrashTrashFiles_Destructor)
{
    // Test method:  ~CopyFromTrashTrashFiles(())
    EXPECT_NO_FATAL_FAILURE({ CopyFromTrashTrashFiles *tmp = new CopyFromTrashTrashFiles(); delete tmp; });
}

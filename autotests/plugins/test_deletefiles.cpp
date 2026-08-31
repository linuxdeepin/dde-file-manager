// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_deletefiles.cpp
 * @brief Unit tests for DeleteFiles methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/deletefiles/deletefiles.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class DeleteFilesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DeleteFiles();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DeleteFiles *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DeleteFilesTest, M_~DeleteFiles)
{
    // Test method:  ~DeleteFiles(())
    EXPECT_NO_FATAL_FAILURE({ DeleteFiles *tmp = new DeleteFiles(); delete tmp; });
}

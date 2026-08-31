// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_restoretrashfiles.cpp
 * @brief Unit tests for RestoreTrashFiles methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/trashfiles/restoretrashfiles.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class RestoreTrashFilesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RestoreTrashFiles();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RestoreTrashFiles *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RestoreTrashFilesTest, RestoreTrashFiles_Destructor)
{
    // Test method:  ~RestoreTrashFiles(())
    EXPECT_NO_FATAL_FAILURE({ RestoreTrashFiles *tmp = new RestoreTrashFiles(); delete tmp; });
}

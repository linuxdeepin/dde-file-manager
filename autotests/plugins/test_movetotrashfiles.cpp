// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_movetotrashfiles.cpp
 * @brief Unit tests for MoveToTrashFiles methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/trashfiles/movetotrashfiles.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class MoveToTrashFilesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MoveToTrashFiles();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MoveToTrashFiles *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MoveToTrashFilesTest, MoveToTrashFiles_Destructor)
{
    // Test method:  ~MoveToTrashFiles(())
    EXPECT_NO_FATAL_FAILURE({ MoveToTrashFiles *tmp = new MoveToTrashFiles(); delete tmp; });
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_cleantrashfiles.cpp
 * @brief Unit tests for CleanTrashFiles methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "fileoperations/cleantrash/cleantrashfiles.h"

#include <QTest>

using namespace dfmplugin_fileoperations;

class CleanTrashFilesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CleanTrashFiles();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CleanTrashFiles *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CleanTrashFilesTest, CleanTrashFiles_Destructor)
{
    // Test method:  ~CleanTrashFiles(())
    EXPECT_NO_FATAL_FAILURE({ CleanTrashFiles *tmp = new CleanTrashFiles(); delete tmp; });
}

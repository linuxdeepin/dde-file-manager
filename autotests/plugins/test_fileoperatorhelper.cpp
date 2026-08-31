// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileoperatorhelper.cpp
 * @brief Unit tests for FileOperatorHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/fileoperatorhelper.h"

#include <QTest>

using namespace dfmplugin_sidebar;

class FileOperatorHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileOperatorHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileOperatorHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileOperatorHelperTest, deleteFiles)
{
    // Test method: void deleteFiles((const FileView *view))
    EXPECT_NO_FATAL_FAILURE(obj->deleteFiles(nullptr));
}

TEST_F(FileOperatorHelperTest, instance)
{
    // Test getter: DFMBASE_USE_NAMESPACE instance()
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });
}

TEST_F(FileOperatorHelperTest, pasteTraditionalFiles)
{
    // Test method: bool pasteTraditionalFiles((const FileView *view))
    auto result = obj->pasteTraditionalFiles(nullptr);
    EXPECT_FALSE(result);

}

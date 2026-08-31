// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileitemdata.cpp
 * @brief Unit tests for FileItemData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "models/fileitemdata.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileItemDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileItemData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileItemData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileItemDataTest, clearThumbnail)
{
    // Test method: void clearThumbnail(())
    EXPECT_NO_FATAL_FAILURE(obj->clearThumbnail());
}

TEST_F(FileItemDataTest, fileIcon)
{
    // Test getter: QIcon fileIcon()
    auto result = obj->fileIcon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(FileItemDataTest, fileInfo)
{
    // Test getter: FileInfoPointer fileInfo()
    auto result = obj->fileInfo();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(FileItemDataTest, isDir)
{
    // Test bool getter: isDir()
    bool result = obj->isDir();
    EXPECT_FALSE(result);

}

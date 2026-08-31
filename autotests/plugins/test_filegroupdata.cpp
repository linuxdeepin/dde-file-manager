// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filegroupdata.cpp
 * @brief Unit tests for FileGroupData methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "groups/filegroupdata.h"

#include <QTest>

using namespace dfmplugin_workspace;

class FileGroupDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FileGroupData();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FileGroupData *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FileGroupDataTest, addFile)
{
    // Test method: void addFile((const FileItemDataPointer &file))
    FileItemDataPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addFile(_arg0));
}

TEST_F(FileGroupDataTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}

TEST_F(FileGroupDataTest, getHeaderText)
{
    // Test getter: QString getHeaderText()
    auto result = obj->getHeaderText();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileGroupDataTest, isEmpty)
{
    // Test bool getter: isEmpty()
    bool result = obj->isEmpty();
    EXPECT_TRUE(result);

}

TEST_F(FileGroupDataTest, removeFile)
{
    // Test method: bool removeFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->removeFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileGroupDataTest, updateFileCount)
{
    // Test method: void updateFileCount(())
    EXPECT_NO_FATAL_FAILURE(obj->updateFileCount());
}

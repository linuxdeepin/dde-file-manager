// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_filegroupdata_1.cpp
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

TEST_F(FileGroupDataTest, FileGroupData)
{
    // Test constructor: FileGroupData((const FileGroupData &other))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FileGroupDataTest, M_~FileGroupData)
{
    // Test method:  ~FileGroupData(())
    EXPECT_NO_FATAL_FAILURE({ FileGroupData *tmp = new FileGroupData(); delete tmp; });
}

TEST_F(FileGroupDataTest, operator=)
{
    // Test getter: FileGroupData operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(FileGroupDataTest, clear)
{
    // Test method: void clear(())
    EXPECT_NO_FATAL_FAILURE(obj->clear());
}

TEST_F(FileGroupDataTest, addFile)
{
    // Test method: void addFile((const FileItemDataPointer &file))
    FileItemDataPointer _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addFile(_arg0));
}

TEST_F(FileGroupDataTest, insertFile)
{
    // Test method: void insertFile((int index, const FileItemDataPointer &file))
    FileItemDataPointer _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->insertFile(0, _arg1));
}

TEST_F(FileGroupDataTest, replaceFile)
{
    // Test method: void replaceFile((int index, const FileItemDataPointer &file))
    FileItemDataPointer _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->replaceFile(0, _arg1));
}

TEST_F(FileGroupDataTest, removeFile)
{
    // Test method: bool removeFile((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->removeFile(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FileGroupDataTest, isEmpty)
{
    // Test bool getter: isEmpty()
    bool result = obj->isEmpty();
    EXPECT_TRUE(result);

}

TEST_F(FileGroupDataTest, sortFiles)
{
    // Test method: void sortFiles((const std::function<bool(const FileItemDataPointer &, const FileItemDataPointer &)> &lessThan))
    std::function<bool( FileItemDataPointer ,  FileItemDataPointer )> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sortFiles(_arg0));
}

TEST_F(FileGroupDataTest, getHeaderText)
{
    // Test getter: QString getHeaderText()
    auto result = obj->getHeaderText();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FileGroupDataTest, updateFileCount)
{
    // Test method: void updateFileCount(())
    EXPECT_NO_FATAL_FAILURE(obj->updateFileCount());
}

TEST_F(FileGroupDataTest, findFileIndex)
{
    // Test method: std::optional<int> findFileIndex((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->findFileIndex(_arg0);
    EXPECT_FALSE(result.has_value());

}

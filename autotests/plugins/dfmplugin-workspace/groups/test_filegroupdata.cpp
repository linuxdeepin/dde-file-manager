// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "groups/filegroupdata.h"
#include "models/fileitemdata.h"
#include <dfm-base/utils/universalutils.h>
#include "stubext.h"

#include <QUrl>
#include <QString>
#include <QList>
#include <QVariant>

using namespace dfmplugin_workspace;

class FileGroupDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        testUrl1 = QUrl::fromLocalFile("/tmp/test/file1.txt");
        testUrl2 = QUrl::fromLocalFile("/tmp/test/file2.txt");
        testUrl3 = QUrl::fromLocalFile("/tmp/test/file3.txt");
        
        // Create mock file items
        fileItem1 = QSharedPointer<FileItemData>::create(testUrl1, nullptr);
        fileItem2 = QSharedPointer<FileItemData>::create(testUrl2, nullptr);
        fileItem3 = QSharedPointer<FileItemData>::create(testUrl3, nullptr);
        
        // Mock UniversalUtils::urlEquals
        stub.set_lamda(&dfmbase::UniversalUtils::urlEquals,
                      [](const QUrl &url1, const QUrl &url2) -> bool {
                          return url1.toString() == url2.toString();
                      });
    }

    void TearDown() override
    {
        stub.clear();
    }

    QUrl testUrl1;
    QUrl testUrl2;
    QUrl testUrl3;
    FileItemDataPointer fileItem1;
    FileItemDataPointer fileItem2;
    FileItemDataPointer fileItem3;
    stub_ext::StubExt stub;
};

TEST_F(FileGroupDataTest, Constructor_Default_CreatesEmptyGroup)
{
    // Test default constructor
    FileGroupData group;
    
    EXPECT_TRUE(group.groupKey.isEmpty());
    EXPECT_TRUE(group.displayName.isEmpty());
    EXPECT_EQ(group.fileCount, 0);
    EXPECT_TRUE(group.isExpanded);
    EXPECT_EQ(group.displayOrder, 0);
    EXPECT_EQ(group.displayIndex, 0);
    EXPECT_TRUE(group.files.isEmpty());
}

TEST_F(FileGroupDataTest, Constructor_Copy_CopiesAllData)
{
    // Test copy constructor
    FileGroupData original;
    original.groupKey = "test_key";
    original.displayName = "Test Group";
    original.fileCount = 2;
    original.isExpanded = false;
    original.displayOrder = 1;
    original.displayIndex = 2;
    original.files.append(fileItem1);
    original.files.append(fileItem2);
    
    FileGroupData copy(original);
    
    EXPECT_EQ(copy.groupKey, original.groupKey);
    EXPECT_EQ(copy.displayName, original.displayName);
    EXPECT_EQ(copy.fileCount, original.fileCount);
    EXPECT_EQ(copy.isExpanded, original.isExpanded);
    EXPECT_EQ(copy.displayOrder, original.displayOrder);
    EXPECT_EQ(copy.displayIndex, original.displayIndex);
    EXPECT_EQ(copy.files.size(), original.files.size());
}

TEST_F(FileGroupDataTest, AssignmentOperator_CopiesAllData)
{
    // Test assignment operator
    FileGroupData original;
    original.groupKey = "test_key";
    original.displayName = "Test Group";
    original.fileCount = 2;
    original.isExpanded = false;
    original.displayOrder = 1;
    original.displayIndex = 2;
    original.files.append(fileItem1);
    original.files.append(fileItem2);
    
    FileGroupData copy;
    copy = original;
    
    EXPECT_EQ(copy.groupKey, original.groupKey);
    EXPECT_EQ(copy.displayName, original.displayName);
    EXPECT_EQ(copy.fileCount, original.fileCount);
    EXPECT_EQ(copy.isExpanded, original.isExpanded);
    EXPECT_EQ(copy.displayOrder, original.displayOrder);
    EXPECT_EQ(copy.displayIndex, original.displayIndex);
    EXPECT_EQ(copy.files.size(), original.files.size());
}

TEST_F(FileGroupDataTest, AssignmentOperator_SelfAssignment_HandledCorrectly)
{
    // Test self-assignment
    FileGroupData group;
    group.groupKey = "test_key";
    
    group = group;
    
    EXPECT_EQ(group.groupKey, "test_key");
}

TEST_F(FileGroupDataTest, GetHeaderText_ReturnsDisplayName)
{
    // Test getHeaderText method
    FileGroupData group;
    group.displayName = "Test Group";
    
    auto result = group.getHeaderText();
    
    EXPECT_EQ(result, "Test Group");
}

TEST_F(FileGroupDataTest, AddFile_ValidFile_AddsFileAndUpdatesCount)
{
    // Test addFile method with valid file
    FileGroupData group;
    
    group.addFile(fileItem1);
    
    EXPECT_EQ(group.files.size(), 1);
    EXPECT_EQ(group.fileCount, 1);
    EXPECT_EQ(group.files.first(), fileItem1);
}

TEST_F(FileGroupDataTest, AddFile_NullFile_DoesNotAddFile)
{
    // Test addFile method with null file
    FileGroupData group;
    
    group.addFile(nullptr);
    
    EXPECT_TRUE(group.files.isEmpty());
    EXPECT_EQ(group.fileCount, 0);
}

TEST_F(FileGroupDataTest, InsertFile_ValidIndex_InsertsFileAndUpdatesCount)
{
    // Test insertFile method with valid index
    FileGroupData group;
    group.files.append(fileItem1);
    group.files.append(fileItem3);
    
    group.insertFile(1, fileItem2);
    
    EXPECT_EQ(group.files.size(), 3);
    EXPECT_EQ(group.fileCount, 3);
    EXPECT_EQ(group.files.at(1), fileItem2);
}

TEST_F(FileGroupDataTest, InsertFile_InvalidIndex_DoesNotInsertFile)
{
    // Test insertFile method with invalid index
    FileGroupData group;
    group.files.append(fileItem1);
    
    group.insertFile(-1, fileItem2);
    group.insertFile(5, fileItem3);
    
    EXPECT_EQ(group.files.size(), 1);
    EXPECT_EQ(group.fileCount, 1);
}

TEST_F(FileGroupDataTest, InsertFile_NullFile_DoesNotInsertFile)
{
    // Test insertFile method with null file
    FileGroupData group;
    
    group.insertFile(0, nullptr);
    
    EXPECT_TRUE(group.files.isEmpty());
    EXPECT_EQ(group.fileCount, 0);
}

TEST_F(FileGroupDataTest, ReplaceFile_ValidIndex_ReplacesFile)
{
    // Test replaceFile method with valid index
    FileGroupData group;
    group.files.append(fileItem1);
    group.files.append(fileItem2);
    
    group.replaceFile(0, fileItem3);
    
    EXPECT_EQ(group.files.size(), 2);
    EXPECT_EQ(group.files.at(0), fileItem3);
    EXPECT_EQ(group.files.at(1), fileItem2);
}

TEST_F(FileGroupDataTest, ReplaceFile_InvalidIndex_DoesNotReplaceFile)
{
    // Test replaceFile method with invalid index
    FileGroupData group;
    group.files.append(fileItem1);
    
    group.replaceFile(-1, fileItem2);
    group.replaceFile(5, fileItem3);
    
    EXPECT_EQ(group.files.size(), 1);
    EXPECT_EQ(group.files.at(0), fileItem1);
}

TEST_F(FileGroupDataTest, ReplaceFile_NullFile_DoesNotReplaceFile)
{
    // Test replaceFile method with null file
    FileGroupData group;
    group.files.append(fileItem1);
    
    group.replaceFile(0, nullptr);
    
    EXPECT_EQ(group.files.size(), 1);
    EXPECT_EQ(group.files.at(0), fileItem1);
}

TEST_F(FileGroupDataTest, RemoveFile_ExistingFile_RemovesFileAndUpdatesCount)
{
    // Test removeFile method with existing file
    FileGroupData group;
    group.files.append(fileItem1);
    group.files.append(fileItem2);
    group.fileCount = 2;
    
    bool result = group.removeFile(testUrl1);
    
    EXPECT_TRUE(result);
    EXPECT_EQ(group.files.size(), 1);
    EXPECT_EQ(group.fileCount, 1);
    EXPECT_EQ(group.files.first(), fileItem2);
}

TEST_F(FileGroupDataTest, RemoveFile_NonExistingFile_ReturnsFalse)
{
    // Test removeFile method with non-existing file
    FileGroupData group;
    group.files.append(fileItem1);
    group.fileCount = 1;
    
    bool result = group.removeFile(testUrl3);
    
    EXPECT_FALSE(result);
    EXPECT_EQ(group.files.size(), 1);
    EXPECT_EQ(group.fileCount, 1);
}

TEST_F(FileGroupDataTest, Clear_ClearsFilesAndResetsCount)
{
    // Test clear method
    FileGroupData group;
    group.files.append(fileItem1);
    group.files.append(fileItem2);
    group.fileCount = 2;
    
    group.clear();
    
    EXPECT_TRUE(group.files.isEmpty());
    EXPECT_EQ(group.fileCount, 0);
}

TEST_F(FileGroupDataTest, IsEmpty_EmptyGroup_ReturnsTrue)
{
    // Test isEmpty method with empty group
    FileGroupData group;
    
    bool result = group.isEmpty();
    
    EXPECT_TRUE(result);
}

TEST_F(FileGroupDataTest, IsEmpty_NonEmptyGroup_ReturnsFalse)
{
    // Test isEmpty method with non-empty group
    FileGroupData group;
    group.files.append(fileItem1);
    
    bool result = group.isEmpty();
    
    EXPECT_FALSE(result);
}

TEST_F(FileGroupDataTest, SortFiles_ValidComparator_SortsFiles)
{
    // Test sortFiles method with valid comparator
    FileGroupData group;
    group.files.append(fileItem3);
    group.files.append(fileItem1);
    group.files.append(fileItem2);
    
    // Sort by URL string
    auto lessThan = [](const FileItemDataPointer &a, const FileItemDataPointer &b) {
        if (!a || !b) return false;
        return a->data(DFMBASE_NAMESPACE::Global::kItemUrlRole).toString() < 
               b->data(DFMBASE_NAMESPACE::Global::kItemUrlRole).toString();
    };
    
    group.sortFiles(lessThan);
    
    EXPECT_EQ(group.files.at(0), fileItem1);
    EXPECT_EQ(group.files.at(1), fileItem2);
    EXPECT_EQ(group.files.at(2), fileItem3);
}

TEST_F(FileGroupDataTest, SortFiles_NullComparator_DoesNotSortFiles)
{
    // Test sortFiles method with null comparator
    FileGroupData group;
    group.files.append(fileItem3);
    group.files.append(fileItem1);
    group.files.append(fileItem2);
    
    group.sortFiles({});
    
    EXPECT_EQ(group.files.at(0), fileItem3);
    EXPECT_EQ(group.files.at(1), fileItem1);
    EXPECT_EQ(group.files.at(2), fileItem2);
}

TEST_F(FileGroupDataTest, UpdateFileCount_UpdatesCountBasedOnFiles)
{
    // Test updateFileCount method
    FileGroupData group;
    group.files.append(fileItem1);
    group.files.append(fileItem2);
    group.fileCount = 5; // Set to wrong value
    
    group.updateFileCount();
    
    EXPECT_EQ(group.fileCount, 2);
}

TEST_F(FileGroupDataTest, FindFileIndex_ExistingFile_ReturnsIndex)
{
    // Test findFileIndex method with existing file
    FileGroupData group;
    group.files.append(fileItem1);
    group.files.append(fileItem2);
    group.files.append(fileItem3);
    
    auto result = group.findFileIndex(testUrl2);
    
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 1);
}

TEST_F(FileGroupDataTest, FindFileIndex_NonExistingFile_ReturnsNullopt)
{
    // Test findFileIndex method with non-existing file
    FileGroupData group;
    group.files.append(fileItem1);
    group.files.append(fileItem2);
    
    auto result = group.findFileIndex(testUrl3);
    
    EXPECT_FALSE(result.has_value());
}

TEST_F(FileGroupDataTest, FindFileIndex_InvalidUrl_ReturnsNullopt)
{
    // Test findFileIndex method with invalid URL
    FileGroupData group;
    group.files.append(fileItem1);
    
    auto result = group.findFileIndex(QUrl());
    
    EXPECT_FALSE(result.has_value());
}

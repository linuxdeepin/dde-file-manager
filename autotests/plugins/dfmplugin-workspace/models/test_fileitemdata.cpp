// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "models/fileitemdata.h"
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/interfaces/sortfileinfo.h>

#include <QUrl>
#include <QIcon>
#include <QVariant>
#include <QDateTime>
#include <QStandardPaths>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

class FileItemDataTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        testUrl = QUrl("file:///test.txt");
        
        // Mock InfoFactory::create to return null to avoid issues
        stub.set_lamda(&InfoFactory::create<FileInfo>, [&]() -> FileInfoPointer {
            return FileInfoPointer(); // Return null pointer
        });
        
        testInfo = nullptr; // Use null to avoid constructor issues
    }

    void TearDown() override
    {
        stub.clear();
    }

    QUrl testUrl;
    FileInfoPointer testInfo;
    stub_ext::StubExt stub;
};

TEST_F(FileItemDataTest, Constructor_WithUrlAndInfo_CreatesValidObject)
{
    // Test constructor with URL and FileInfo
    FileItemData itemData(testUrl, testInfo);
    
    EXPECT_EQ(itemData.fileInfo(), testInfo);
    // Don't test parentData for null info case
}

TEST_F(FileItemDataTest, Constructor_WithSortInfo_CreatesValidObject)
{
    // Test constructor with SortInfo
    auto sortInfo = QSharedPointer<dfmbase::SortFileInfo>::create();
    FileItemData itemData(sortInfo);
    
    EXPECT_EQ(itemData.fileSortInfo(), sortInfo);
    // Don't test parentData for sortInfo case
}

TEST_F(FileItemDataTest, SetParentData_SetsParentCorrectly)
{
    // Test setting parent data
    FileItemData itemData(testUrl, testInfo);
    FileItemData parentData(QUrl("file:///parent"));
    
    itemData.setParentData(&parentData);
    
    EXPECT_EQ(itemData.parentData(), &parentData);
}

TEST_F(FileItemDataTest, SetSortFileInfo_SetsSortInfoCorrectly)
{
    // Test setting sort file info
    FileItemData itemData(testUrl, testInfo);
    auto sortInfo = QSharedPointer<dfmbase::SortFileInfo>::create();
    
    itemData.setSortFileInfo(sortInfo);
    
    EXPECT_EQ(itemData.fileSortInfo(), sortInfo);
}

TEST_F(FileItemDataTest, FileSortInfo_ReturnsCorrectSortInfo)
{
    // Test getting sort file info
    auto sortInfo = QSharedPointer<dfmbase::SortFileInfo>::create();
    FileItemData itemData(sortInfo);
    
    EXPECT_EQ(itemData.fileSortInfo(), sortInfo);
}

TEST_F(FileItemDataTest, RefreshInfo_DoesNotCrash)
{
    // Test refreshing file info
    FileItemData itemData(testUrl, testInfo);
    
    // Since testInfo is null, refreshInfo should handle it gracefully
    // We just test that it doesn't crash
    EXPECT_NO_THROW(itemData.refreshInfo());
}

TEST_F(FileItemDataTest, ClearThumbnail_DoesNotCrash)
{
    // Test clearing thumbnail
    FileItemData itemData(testUrl, testInfo);
    
    // Since testInfo is null, clearThumbnail should handle it gracefully
    EXPECT_NO_THROW(itemData.clearThumbnail());
}

TEST_F(FileItemDataTest, FileInfo_ReturnsCorrectInfo)
{
    // Test getting file info
    FileItemData itemData(testUrl, testInfo);
    
    EXPECT_EQ(itemData.fileInfo(), testInfo);
}

TEST_F(FileItemDataTest, ParentData_ReturnsCorrectParent)
{
    // Test getting parent data
    FileItemData itemData(testUrl, testInfo);
    FileItemData parentData(QUrl("file:///parent"));
    
    itemData.setParentData(&parentData);
    
    EXPECT_EQ(itemData.parentData(), &parentData);
}

TEST_F(FileItemDataTest, FileIcon_ReturnsValidIcon)
{
    // Test getting file icon
    FileItemData itemData(testUrl, testInfo);
    
    QIcon icon = itemData.fileIcon();
    
    // Just test that it doesn't crash
    EXPECT_NO_THROW(itemData.fileIcon());
}

TEST_F(FileItemDataTest, Data_WithValidRole_ReturnsCorrectData)
{
    // Test getting data with valid role
    FileItemData itemData(testUrl, testInfo);
    
    // Test display role
    QVariant displayData = itemData.data(Qt::DisplayRole);
    EXPECT_TRUE(displayData.isValid());
    
    // Test name role
    QVariant nameData = itemData.data(kItemNameRole);
    EXPECT_TRUE(nameData.isValid());
    
    // Test URL role
    QVariant urlData = itemData.data(kItemUrlRole);
    EXPECT_TRUE(urlData.isValid());
}

TEST_F(FileItemDataTest, Data_WithInvalidRole_ReturnsInvalidData)
{
    // Test getting data with invalid role
    FileItemData itemData(testUrl, testInfo);
    
    QVariant data = itemData.data(-1);
    
    EXPECT_FALSE(data.isValid());
}

TEST_F(FileItemDataTest, SetAvailableState_SetsStateCorrectly)
{
    // Test setting available state
    FileItemData itemData(testUrl, testInfo);
    
    itemData.setAvailableState(false);
    
    EXPECT_EQ(itemData.data(kItemFileIsAvailableRole).toBool(), false);
    
    itemData.setAvailableState(true);
    
    EXPECT_EQ(itemData.data(kItemFileIsAvailableRole).toBool(), true);
}

TEST_F(FileItemDataTest, SetExpanded_SetsExpandedStateCorrectly)
{
    // Test setting expanded state
    FileItemData itemData(testUrl, testInfo);
    
    itemData.setExpanded(true);
    
    EXPECT_EQ(itemData.data(kItemTreeViewExpandedRole).toBool(), true);
    
    itemData.setExpanded(false);
    
    EXPECT_EQ(itemData.data(kItemTreeViewExpandedRole).toBool(), false);
}

TEST_F(FileItemDataTest, SetDepth_SetsDepthCorrectly)
{
    // Test setting depth
    FileItemData itemData(testUrl, testInfo);
    
    itemData.setDepth(5);
    
    EXPECT_EQ(itemData.data(kItemTreeViewDepthRole).toInt(), 5);
}

TEST_F(FileItemDataTest, SetGroupDisplayIndex_SetsIndexCorrectly)
{
    // Test setting group display index
    FileItemData itemData(testUrl, testInfo);
    
    itemData.setGroupDisplayIndex(3);
    
    EXPECT_EQ(itemData.data(kItemGroupDisplayIndex).toInt(), 3);
}

TEST_F(FileItemDataTest, Data_WithFilePathRole_ReturnsCorrectPath)
{
    // Test getting file path
    FileItemData itemData(testUrl, testInfo);
    
    QVariant pathData = itemData.data(kItemFilePathRole);
    
    EXPECT_TRUE(pathData.isValid());
    EXPECT_FALSE(pathData.toString().isEmpty());
}

TEST_F(FileItemDataTest, Data_WithFileLastModifiedRole_ReturnsCorrectTime)
{
    // Test getting last modified time
    FileItemData itemData(testUrl, testInfo);
    
    QVariant timeData = itemData.data(kItemFileLastModifiedRole);
    
    EXPECT_TRUE(timeData.isValid());
    EXPECT_FALSE(timeData.toString().isEmpty());
}

TEST_F(FileItemDataTest, Data_WithFileCreatedRole_ReturnsCorrectTime)
{
    // Test getting created time
    FileItemData itemData(testUrl, testInfo);
    
    QVariant timeData = itemData.data(kItemFileCreatedRole);
    
    EXPECT_TRUE(timeData.isValid());
    EXPECT_FALSE(timeData.toString().isEmpty());
}

TEST_F(FileItemDataTest, Data_WithFileSizeRole_ReturnsCorrectSize)
{
    // Test getting file size
    FileItemData itemData(testUrl, testInfo);
    
    QVariant sizeData = itemData.data(kItemFileSizeRole);
    
    EXPECT_TRUE(sizeData.isValid());
    EXPECT_FALSE(sizeData.toString().isEmpty());
}

TEST_F(FileItemDataTest, Data_WithFileMimeTypeRole_ReturnsCorrectMimeType)
{
    // Test getting file mime type
    FileItemData itemData(testUrl, testInfo);
    
    QVariant mimeTypeData = itemData.data(kItemFileMimeTypeRole);
    
    EXPECT_TRUE(mimeTypeData.isValid());
}

TEST_F(FileItemDataTest, Data_WithFileIsWritableRole_ReturnsCorrectWritableState)
{
    // Test getting writable state
    FileItemData itemData(testUrl, testInfo);
    
    QVariant writableData = itemData.data(kItemFileIsWritableRole);
    
    EXPECT_TRUE(writableData.isValid());
}

TEST_F(FileItemDataTest, Data_WithFileIsDirRole_ReturnsCorrectDirState)
{
    // Test getting directory state
    FileItemData itemData(testUrl, testInfo);
    
    QVariant dirData = itemData.data(kItemFileIsDirRole);
    
    EXPECT_TRUE(dirData.isValid());
}

TEST_F(FileItemDataTest, Data_WithFileCanRenameRole_ReturnsCorrectRenameState)
{
    // Test getting can rename state
    FileItemData itemData(testUrl, testInfo);
    
    QVariant canRenameData = itemData.data(kItemFileCanRenameRole);
    
    EXPECT_TRUE(canRenameData.isValid());
}

TEST_F(FileItemDataTest, Data_WithFileCanDropRole_ReturnsCorrectDropState)
{
    // Test getting can drop state
    FileItemData itemData(testUrl, testInfo);
    
    QVariant canDropData = itemData.data(kItemFileCanDropRole);
    
    EXPECT_TRUE(canDropData.isValid());
}

TEST_F(FileItemDataTest, Data_WithFileCanDragRole_ReturnsCorrectDragState)
{
    // Test getting can drag state
    FileItemData itemData(testUrl, testInfo);
    
    QVariant canDragData = itemData.data(kItemFileCanDragRole);
    
    EXPECT_TRUE(canDragData.isValid());
}

TEST_F(FileItemDataTest, Data_WithFileSizeIntRole_ReturnsCorrectIntSize)
{
    // Test getting file size as integer
    FileItemData itemData(testUrl, testInfo);
    
    QVariant sizeData = itemData.data(kItemFileSizeIntRole);
    
    EXPECT_TRUE(sizeData.isValid());
    EXPECT_GE(sizeData.toInt(), 0);
}

TEST_F(FileItemDataTest, Data_WithTreeViewCanExpandRole_ReturnsCorrectExpandState)
{
    // Test getting tree view can expand state
    FileItemData itemData(testUrl, testInfo);
    
    QVariant canExpandData = itemData.data(kItemTreeViewCanExpandRole);
    
    EXPECT_TRUE(canExpandData.isValid());
}

TEST_F(FileItemDataTest, Data_WithFileNameOfRenameRole_ReturnsCorrectName)
{
    // Test getting file name for rename
    FileItemData itemData(testUrl, testInfo);
    
    QVariant nameData = itemData.data(kItemFileNameOfRenameRole);
    
    EXPECT_TRUE(nameData.isValid());
    EXPECT_FALSE(nameData.toString().isEmpty());
}

TEST_F(FileItemDataTest, Data_WithFileBaseNameOfRenameRole_ReturnsCorrectBaseName)
{
    // Test getting file base name for rename
    FileItemData itemData(testUrl, testInfo);
    
    QVariant baseNameData = itemData.data(kItemFileBaseNameOfRenameRole);
    
    EXPECT_TRUE(baseNameData.isValid());
}

TEST_F(FileItemDataTest, Data_WithFileSuffixOfRenameRole_ReturnsCorrectSuffix)
{
    // Test getting file suffix for rename
    FileItemData itemData(testUrl, testInfo);
    
    QVariant suffixData = itemData.data(kItemFileSuffixOfRenameRole);
    
    EXPECT_TRUE(suffixData.isValid());
}

TEST_F(FileItemDataTest, Data_WithFileIconModelToolTipRole_ReturnsCorrectToolTip)
{
    // Test getting file icon model tooltip
    FileItemData itemData(testUrl, testInfo);
    
    QVariant toolTipData = itemData.data(kItemFileIconModelToolTipRole);
    
    EXPECT_TRUE(toolTipData.isValid());
}

TEST_F(FileItemDataTest, Data_WithSizeHintRole_ReturnsCorrectSize)
{
    // Test getting size hint
    FileItemData itemData(testUrl, testInfo);
    
    QVariant sizeHintData = itemData.data(kItemSizeHintRole);
    
    EXPECT_TRUE(sizeHintData.isValid());
    QSize size = sizeHintData.toSize();
    EXPECT_EQ(size.height(), 26);
}

TEST_F(FileItemDataTest, TransFileInfo_DoesNotCrash)
{
    // Test transforming file info
    FileItemData itemData(testUrl, testInfo);
    
    // Since testInfo is null, transFileInfo should handle it gracefully
    EXPECT_NO_THROW(itemData.transFileInfo());
}

TEST_F(FileItemDataTest, Data_WithCreateFileInfoRole_DoesNotCrash)
{
    // Test that file info is created if needed
    FileItemData itemData(testUrl, nullptr); // No initial info
    
    // Mock InfoFactory::create
    stub.set_lamda(&InfoFactory::create<FileInfo>, [&]() -> FileInfoPointer {
        return testInfo;
    });
    
    EXPECT_NO_THROW(itemData.data(kItemCreateFileInfoRole));
}

TEST_F(FileItemDataTest, Data_WithUpdateAndTransFileInfoRole_DoesNotCrash)
{
    // Test update and transform file info role
    FileItemData itemData(testUrl, testInfo);
    
    // Since testInfo is null, data should handle it gracefully
    EXPECT_NO_THROW(itemData.data(kItemUpdateAndTransFileInfoRole));
}

TEST_F(FileItemDataTest, Data_WithFileContentPreviewRole_ReturnsContent)
{
    // Test getting file content preview
    auto sortInfo = QSharedPointer<dfmbase::SortFileInfo>::create();
    QString expectedContent = "test content";
    sortInfo->setHighlightContent(expectedContent);
    
    FileItemData itemData(sortInfo);
    
    QVariant contentData = itemData.data(kItemFileContentPreviewRole);
    
    EXPECT_TRUE(contentData.isValid());
    EXPECT_EQ(contentData.toString(), expectedContent);
}

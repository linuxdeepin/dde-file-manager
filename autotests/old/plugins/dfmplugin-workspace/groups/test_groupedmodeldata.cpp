// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "groups/groupedmodeldata.h"
#include "groups/filegroupdata.h"
#include "groups/modelitemwrapper.h"

#include <dfm-base/utils/universalutils.h>
#include <gtest/gtest.h>

#include <QMutexLocker>
#include <QUrl>
#include <QVariant>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

#include "stubext.h"

DPWORKSPACE_BEGIN_NAMESPACE

class TestGroupedModelData : public testing::Test
{
public:
    void SetUp() override
    {
        // Mock UniversalUtils::urlEquals
        stub.set_lamda(&dfmbase::UniversalUtils::urlEquals, [](const QUrl &url1, const QUrl &url2) -> bool {
            __DBG_STUB_INVOKE__
            return url1.toString() == url2.toString();
        });

        // Create test file item data
        QUrl file1Url = QUrl::fromLocalFile("/test/file1.txt");
        QUrl file2Url = QUrl::fromLocalFile("/test/file2.txt");
        testFileData = FileItemDataPointer::create(file1Url);
        testFileData2 = FileItemDataPointer::create(file2Url);

        // Create test group data
        testGroup.groupKey = "test_group";
        testGroup.displayName = "Test Group";
        testGroup.files.append(testFileData);
        testGroup.files.append(testFileData2);
        testGroup.isExpanded = true;
        testGroup.displayIndex = 0;
        testGroup.displayOrder = 1;
        testGroup.fileCount = 2;

        testGroup2.groupKey = "test_group2";
        testGroup2.displayName = "Test Group 2";
        testGroup2.files.append(testFileData2);
        testGroup2.isExpanded = false;
        testGroup2.displayIndex = 1;
        testGroup2.displayOrder = 2;
        testGroup2.fileCount = 1;
    }

    void TearDown() override
    {
        stub.clear();
    }

    FileItemDataPointer testFileData;
    FileItemDataPointer testFileData2;
    FileGroupData testGroup;
    FileGroupData testGroup2;
    stub_ext::StubExt stub;
};

TEST_F(TestGroupedModelData, Constructor)
{
    GroupedModelData data;
    EXPECT_TRUE(data.groups.isEmpty());
    EXPECT_TRUE(data.flattenedItems.isEmpty());
    EXPECT_TRUE(data.groupExpansionStates.isEmpty());
}

TEST_F(TestGroupedModelData, CopyConstructor)
{
    GroupedModelData original;
    original.addGroup(testGroup);
    original.setGroupExpanded(testGroup.groupKey, true);
    original.rebuildFlattenedItems();

    GroupedModelData copy(original);
    EXPECT_EQ(copy.groups.size(), original.groups.size());
    EXPECT_EQ(copy.flattenedItems.size(), original.flattenedItems.size());
    EXPECT_EQ(copy.groupExpansionStates.size(), original.groupExpansionStates.size());
}

TEST_F(TestGroupedModelData, AssignmentOperator)
{
    GroupedModelData original;
    original.addGroup(testGroup);
    original.setGroupExpanded(testGroup.groupKey, true);
    original.rebuildFlattenedItems();

    GroupedModelData copy;
    copy = original;
    EXPECT_EQ(copy.groups.size(), original.groups.size());
    EXPECT_EQ(copy.flattenedItems.size(), original.flattenedItems.size());
    EXPECT_EQ(copy.groupExpansionStates.size(), original.groupExpansionStates.size());
}

TEST_F(TestGroupedModelData, GetAllFiles)
{
    GroupedModelData data;
    data.addGroup(testGroup);
    data.addGroup(testGroup2);

    QList<FileItemDataPointer> allFiles = data.getAllFiles();
    EXPECT_EQ(allFiles.size(), 3);  // 2 files in testGroup + 1 file in testGroup2
}

TEST_F(TestGroupedModelData, SetGroupExpanded)
{
    GroupedModelData data;
    data.addGroup(testGroup);
    data.rebuildFlattenedItems();

    data.setGroupExpanded(testGroup.groupKey, false);
    EXPECT_FALSE(data.isGroupExpanded(testGroup.groupKey));

    data.setGroupExpanded(testGroup.groupKey, true);
    EXPECT_TRUE(data.isGroupExpanded(testGroup.groupKey));
}

TEST_F(TestGroupedModelData, SetGroupExpanded_InvalidKey)
{
    GroupedModelData data;
    data.setGroupExpanded("", true);  // Should not crash
    data.setGroupExpanded("nonexistent", true);  // Should not crash
}

TEST_F(TestGroupedModelData, IsGroupExpanded)
{
    GroupedModelData data;
    
    // Test default behavior (should return true for non-existent keys)
    EXPECT_TRUE(data.isGroupExpanded("nonexistent"));
    
    data.addGroup(testGroup);
    data.setGroupExpanded(testGroup.groupKey, false);
    EXPECT_FALSE(data.isGroupExpanded(testGroup.groupKey));
}

TEST_F(TestGroupedModelData, UpdateGroupHeader)
{
    GroupedModelData data;
    data.addGroup(testGroup);
    data.rebuildFlattenedItems();

    // Update group header
    data.updateGroupHeader(testGroup.groupKey);
    
    // Verify group header is updated (this is mainly to ensure no crash)
    auto pos = data.findGroupHeaderStartPos(testGroup.groupKey);
    ASSERT_TRUE(pos.has_value());
    EXPECT_GE(pos.value(), 0);
}

TEST_F(TestGroupedModelData, UpdateGroupHeader_InvalidKey)
{
    GroupedModelData data;
    data.updateGroupHeader("");  // Should not crash
    data.updateGroupHeader("nonexistent");  // Should not crash
}

TEST_F(TestGroupedModelData, RebuildFlattenedItems)
{
    GroupedModelData data;
    data.addGroup(testGroup);
    data.addGroup(testGroup2);
    
    data.rebuildFlattenedItems();
    
    // Should have group headers + files from expanded groups
    int expectedCount = 2;  // 2 group headers
    expectedCount += testGroup.files.size();  // testGroup is expanded
    // testGroup2 is not expanded, so its files shouldn't be included
    
    EXPECT_EQ(data.getItemCount(), expectedCount);
}

TEST_F(TestGroupedModelData, Clear)
{
    GroupedModelData data;
    data.addGroup(testGroup);
    data.addGroup(testGroup2);
    data.rebuildFlattenedItems();
    
    data.clear();
    
    EXPECT_TRUE(data.groups.isEmpty());
    EXPECT_TRUE(data.flattenedItems.isEmpty());
    EXPECT_TRUE(data.groupExpansionStates.isEmpty());
}

TEST_F(TestGroupedModelData, IsEmpty)
{
    GroupedModelData data;
    EXPECT_TRUE(data.isEmpty());
    
    data.addGroup(testGroup);
    EXPECT_FALSE(data.isEmpty());
}

TEST_F(TestGroupedModelData, AddGroup)
{
    GroupedModelData data;
    
    // Test adding valid group
    EXPECT_TRUE(data.addGroup(testGroup));
    EXPECT_EQ(data.groups.size(), 1);
    
    // Test adding duplicate group
    EXPECT_FALSE(data.addGroup(testGroup));
    EXPECT_EQ(data.groups.size(), 1);
    
    // Test adding group with empty key
    FileGroupData emptyGroup;
    emptyGroup.groupKey = "";
    EXPECT_FALSE(data.addGroup(emptyGroup));
    EXPECT_EQ(data.groups.size(), 1);
}

TEST_F(TestGroupedModelData, RemoveGroup)
{
    GroupedModelData data;
    data.addGroup(testGroup);
    data.addGroup(testGroup2);
    
    // Test removing existing group
    EXPECT_TRUE(data.removeGroup(testGroup.groupKey));
    EXPECT_EQ(data.groups.size(), 1);
    
    // Test removing non-existent group
    EXPECT_FALSE(data.removeGroup("nonexistent"));
    EXPECT_EQ(data.groups.size(), 1);
    
    // Test removing group with empty key
    EXPECT_FALSE(data.removeGroup(""));
    EXPECT_EQ(data.groups.size(), 1);
}

TEST_F(TestGroupedModelData, InsertItem)
{
    GroupedModelData data;
    data.rebuildFlattenedItems();
    
    ModelItemWrapper item(testFileData, "test_group");
    
    // Test inserting at valid position
    data.insertItem(0, item);
    EXPECT_EQ(data.getItemCount(), 1);
    
    // Test inserting at end
    data.insertItem(data.getItemCount(), item);
    EXPECT_EQ(data.getItemCount(), 2);
    
    // Test inserting at invalid position
    data.insertItem(-1, item);  // Should not crash
    data.insertItem(data.getItemCount() + 1, item);  // Should not crash
    EXPECT_EQ(data.getItemCount(), 2);  // Count should not change
}

TEST_F(TestGroupedModelData, RemoveItems)
{
    GroupedModelData data;
    data.rebuildFlattenedItems();
    
    // Insert some items first
    ModelItemWrapper item(testFileData, "test_group");
    data.insertItem(0, item);
    data.insertItem(1, item);
    data.insertItem(2, item);
    
    // Test removing valid range
    int removedCount = data.removeItems(0, 2);
    EXPECT_EQ(removedCount, 2);
    EXPECT_EQ(data.getItemCount(), 1);
    
    // Test removing with invalid parameters
    EXPECT_EQ(data.removeItems(-1, 1), 0);
    EXPECT_EQ(data.removeItems(data.getItemCount(), 1), 0);
    EXPECT_EQ(data.removeItems(0, 0), 0);
}

TEST_F(TestGroupedModelData, ReplaceItem)
{
    GroupedModelData data;
    data.rebuildFlattenedItems();
    
    // Insert an item first
    ModelItemWrapper item(testFileData, "test_group");
    data.insertItem(0, item);
    
    // Test replacing at valid position
    ModelItemWrapper newItem(testFileData2, "test_group2");
    data.replaceItem(0, newItem);
    
    ModelItemWrapper retrievedItem = data.getItemAt(0);
    EXPECT_EQ(retrievedItem.groupKey, "test_group2");
    
    // Test replacing at invalid position
    data.replaceItem(-1, newItem);  // Should not crash
    data.replaceItem(data.getItemCount(), newItem);  // Should not crash
}

TEST_F(TestGroupedModelData, GetGroup)
{
    GroupedModelData data;
    data.addGroup(testGroup);
    
    // Test non-const version
    FileGroupData *group = data.getGroup(testGroup.groupKey);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->groupKey, testGroup.groupKey);
    
    // Test non-existent group
    EXPECT_EQ(data.getGroup("nonexistent"), nullptr);
}

TEST_F(TestGroupedModelData, GetGroup_Const)
{
    GroupedModelData data;
    data.addGroup(testGroup);
    
    const GroupedModelData &constData = data;
    
    // Test const version
    const FileGroupData *group = constData.getGroup(testGroup.groupKey);
    ASSERT_NE(group, nullptr);
    EXPECT_EQ(group->groupKey, testGroup.groupKey);
    
    // Test non-existent group
    EXPECT_EQ(constData.getGroup("nonexistent"), nullptr);
}

TEST_F(TestGroupedModelData, GetItemCount)
{
    GroupedModelData data;
    EXPECT_EQ(data.getItemCount(), 0);
    
    data.addGroup(testGroup);
    data.rebuildFlattenedItems();
    
    int expectedCount = 1;  // Group header
    expectedCount += testGroup.files.size();  // Files
    EXPECT_EQ(data.getItemCount(), expectedCount);
}

TEST_F(TestGroupedModelData, GetFileItemCount)
{
    GroupedModelData data;
    EXPECT_EQ(data.getFileItemCount(), 0);
    
    data.addGroup(testGroup);
    data.addGroup(testGroup2);
    
    EXPECT_EQ(data.getFileItemCount(), 3);  // 2 files in testGroup + 1 file in testGroup2
}

TEST_F(TestGroupedModelData, GetGroupItemCount)
{
    GroupedModelData data;
    EXPECT_EQ(data.getGroupItemCount(), 0);
    
    data.addGroup(testGroup);
    data.addGroup(testGroup2);
    
    EXPECT_EQ(data.getGroupItemCount(), 2);
}

TEST_F(TestGroupedModelData, GetItemAt)
{
    GroupedModelData data;
    data.rebuildFlattenedItems();
    
    // Test invalid index
    ModelItemWrapper item = data.getItemAt(-1);
    EXPECT_FALSE(item.isValid());
    
    item = data.getItemAt(0);
    EXPECT_FALSE(item.isValid());
    
    // Test valid index
    data.addGroup(testGroup);
    data.rebuildFlattenedItems();
    
    item = data.getItemAt(0);
    EXPECT_TRUE(item.isValid());
    EXPECT_TRUE(item.isGroupHeader());
}

TEST_F(TestGroupedModelData, FindGroupHeaderStartPos)
{
    GroupedModelData data;
    data.addGroup(testGroup);
    data.rebuildFlattenedItems();
    
    // Test finding existing group
    auto pos = data.findGroupHeaderStartPos(testGroup.groupKey);
    ASSERT_TRUE(pos.has_value());
    EXPECT_GE(pos.value(), 0);
    
    // Test finding non-existent group
    pos = data.findGroupHeaderStartPos("nonexistent");
    EXPECT_FALSE(pos.has_value());
}

TEST_F(TestGroupedModelData, FindFileStartPos)
{
    GroupedModelData data;
    data.addGroup(testGroup);
    data.rebuildFlattenedItems();
    
    // Test finding existing file
    QUrl fileUrl = QUrl::fromLocalFile("/test/file1.txt");
    auto pos = data.findFileStartPos(fileUrl);
    EXPECT_TRUE(pos.has_value());  // Should find file if group is expanded
    
    // Test finding non-existent file
    QUrl nonExistentUrl = QUrl::fromLocalFile("/test/nonexistent.txt");
    pos = data.findFileStartPos(nonExistentUrl);
    EXPECT_FALSE(pos.has_value());
    
    // Test with invalid URL
    pos = data.findFileStartPos(QUrl());
    EXPECT_FALSE(pos.has_value());
}

DPWORKSPACE_END_NAMESPACE

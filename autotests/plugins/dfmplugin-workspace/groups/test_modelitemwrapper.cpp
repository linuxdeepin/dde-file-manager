// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "groups/modelitemwrapper.h"
#include "groups/filegroupdata.h"
#include "models/fileitemdata.h"
#include "stubext.h"

#include <QUrl>
#include <QString>
#include <QVariant>
#include <QHash>

using namespace dfmplugin_workspace;

class ModelItemWrapperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        testUrl = QUrl::fromLocalFile("/tmp/test/file.txt");
        fileItem = QSharedPointer<FileItemData>::create(testUrl, nullptr);
        
        // Create test group data
        testGroup.groupKey = "test_group";
        testGroup.displayName = "Test Group";
        testGroup.fileCount = 3;
        testGroup.isExpanded = true;
        testGroup.displayOrder = 1;
        testGroup.displayIndex = 0;
    }

    void TearDown() override
    {
        stub.clear();
    }

    QUrl testUrl;
    FileItemDataPointer fileItem;
    FileGroupData testGroup;
    stub_ext::StubExt stub;
};

TEST_F(ModelItemWrapperTest, Constructor_Default_CreatesFileItemWrapper)
{
    // Test default constructor
    ModelItemWrapper wrapper;
    
    EXPECT_EQ(wrapper.itemType, ModelItemWrapper::FileItem);
    EXPECT_TRUE(wrapper.groupKey.isEmpty());
    EXPECT_EQ(wrapper.fileData, nullptr);
    EXPECT_TRUE(wrapper.groupValues.isEmpty());
}

TEST_F(ModelItemWrapperTest, Constructor_FileItem_CreatesFileItemWrapper)
{
    // Test file item constructor
    QString groupKey = "test_group";
    ModelItemWrapper wrapper(fileItem, groupKey);
    
    EXPECT_EQ(wrapper.itemType, ModelItemWrapper::FileItem);
    EXPECT_EQ(wrapper.groupKey, groupKey);
    EXPECT_EQ(wrapper.fileData, fileItem);
    EXPECT_TRUE(wrapper.groupValues.isEmpty());
}

TEST_F(ModelItemWrapperTest, Constructor_GroupHeader_CreatesGroupHeaderWrapper)
{
    // Test group header constructor
    ModelItemWrapper wrapper(&testGroup);
    
    EXPECT_EQ(wrapper.itemType, ModelItemWrapper::GroupHeaderItem);
    EXPECT_EQ(wrapper.groupKey, testGroup.groupKey);
    EXPECT_NE(wrapper.fileData, nullptr);
    EXPECT_FALSE(wrapper.groupValues.isEmpty());
    
    // Check group values
    EXPECT_EQ(wrapper.groupValues.value(DFMBASE_NAMESPACE::Global::kItemIsGroupHeaderType).toBool(), true);
    EXPECT_EQ(wrapper.groupValues.value(DFMBASE_NAMESPACE::Global::kItemDisplayRole).toString(), testGroup.getHeaderText());
    EXPECT_EQ(wrapper.groupValues.value(DFMBASE_NAMESPACE::Global::kItemNameRole).toString(), testGroup.getHeaderText());
    EXPECT_EQ(wrapper.groupValues.value(DFMBASE_NAMESPACE::Global::kItemGroupFileCount).toInt(), testGroup.fileCount);
    EXPECT_EQ(wrapper.groupValues.value(DFMBASE_NAMESPACE::Global::kItemGroupHeaderKey).toString(), testGroup.groupKey);
    EXPECT_EQ(wrapper.groupValues.value(DFMBASE_NAMESPACE::Global::kItemGroupDisplayIndex).toInt(), testGroup.displayIndex);
    EXPECT_EQ(wrapper.groupValues.value(DFMBASE_NAMESPACE::Global::kItemGroupExpandedRole).toBool(), testGroup.isExpanded);
}

TEST_F(ModelItemWrapperTest, Constructor_GroupHeaderNull_CreatesGroupHeaderWrapperWithNullData)
{
    // Test group header constructor with null group data
    ModelItemWrapper wrapper(nullptr);
    
    EXPECT_EQ(wrapper.itemType, ModelItemWrapper::GroupHeaderItem);
    EXPECT_TRUE(wrapper.groupKey.isEmpty());
    EXPECT_EQ(wrapper.fileData, nullptr);
    EXPECT_TRUE(wrapper.groupValues.isEmpty());
}

TEST_F(ModelItemWrapperTest, Constructor_Copy_CopiesAllData)
{
    // Test copy constructor
    ModelItemWrapper original(fileItem, "test_group");
    original.groupValues[123] = QVariant("test_value");
    
    ModelItemWrapper copy(original);
    
    EXPECT_EQ(copy.itemType, original.itemType);
    EXPECT_EQ(copy.groupKey, original.groupKey);
    EXPECT_EQ(copy.fileData, original.fileData);
    EXPECT_EQ(copy.groupValues, original.groupValues);
}

TEST_F(ModelItemWrapperTest, AssignmentOperator_CopiesAllData)
{
    // Test assignment operator
    ModelItemWrapper original(fileItem, "test_group");
    original.groupValues[123] = QVariant("test_value");
    
    ModelItemWrapper copy;
    copy = original;
    
    EXPECT_EQ(copy.itemType, original.itemType);
    EXPECT_EQ(copy.groupKey, original.groupKey);
    EXPECT_EQ(copy.fileData, original.fileData);
    EXPECT_EQ(copy.groupValues, original.groupValues);
}

TEST_F(ModelItemWrapperTest, AssignmentOperator_SelfAssignment_HandledCorrectly)
{
    // Test self-assignment
    ModelItemWrapper wrapper(fileItem, "test_group");
    
    wrapper = wrapper;
    
    EXPECT_EQ(wrapper.itemType, ModelItemWrapper::FileItem);
    EXPECT_EQ(wrapper.groupKey, "test_group");
    EXPECT_EQ(wrapper.fileData, fileItem);
}

TEST_F(ModelItemWrapperTest, IsGroupHeader_FileItem_ReturnsFalse)
{
    // Test isGroupHeader for file item
    ModelItemWrapper wrapper(fileItem, "test_group");
    
    bool result = wrapper.isGroupHeader();
    
    EXPECT_FALSE(result);
}

TEST_F(ModelItemWrapperTest, IsGroupHeader_GroupHeader_ReturnsTrue)
{
    // Test isGroupHeader for group header
    ModelItemWrapper wrapper(&testGroup);
    
    bool result = wrapper.isGroupHeader();
    
    EXPECT_TRUE(result);
}

TEST_F(ModelItemWrapperTest, IsFileItem_FileItem_ReturnsTrue)
{
    // Test isFileItem for file item
    ModelItemWrapper wrapper(fileItem, "test_group");
    
    bool result = wrapper.isFileItem();
    
    EXPECT_TRUE(result);
}

TEST_F(ModelItemWrapperTest, IsFileItem_GroupHeader_ReturnsFalse)
{
    // Test isFileItem for group header
    ModelItemWrapper wrapper(&testGroup);
    
    bool result = wrapper.isFileItem();
    
    EXPECT_FALSE(result);
}

TEST_F(ModelItemWrapperTest, GetData_FileItemWithValidRole_ReturnsData)
{
    // Test getData for file item with valid role
    ModelItemWrapper wrapper(fileItem, "test_group");
    
    // Mock file data
    stub.set_lamda(&FileItemData::data, [this](FileItemData *, int role) -> QVariant {
        if (role == DFMBASE_NAMESPACE::Global::kItemUrlRole) {
            return testUrl;
        }
        return QVariant();
    });
    
    auto result = wrapper.getData(DFMBASE_NAMESPACE::Global::kItemUrlRole);
    
    EXPECT_EQ(result.toUrl(), testUrl);
}

TEST_F(ModelItemWrapperTest, GetData_FileItemWithGroupHeaderRole_ReturnsFalse)
{
    // Test getData for file item with group header role
    ModelItemWrapper wrapper(fileItem, "test_group");
    
    auto result = wrapper.getData(DFMBASE_NAMESPACE::Global::kItemIsGroupHeaderType);
    
    EXPECT_FALSE(result.toBool());
}

TEST_F(ModelItemWrapperTest, GetData_FileItemWithNullData_ReturnsQVariant)
{
    // Test getData for file item with null data
    ModelItemWrapper wrapper(nullptr, "test_group");
    
    auto result = wrapper.getData(DFMBASE_NAMESPACE::Global::kItemUrlRole);
    
    EXPECT_TRUE(result.isNull());
}

TEST_F(ModelItemWrapperTest, GetData_GroupHeaderWithValidRole_ReturnsGroupValue)
{
    // Test getData for group header with valid role
    ModelItemWrapper wrapper(&testGroup);
    
    auto result = wrapper.getData(DFMBASE_NAMESPACE::Global::kItemDisplayRole);
    
    EXPECT_EQ(result.toString(), testGroup.getHeaderText());
}

TEST_F(ModelItemWrapperTest, GetData_GroupHeaderWithInvalidRole_ReturnsQVariant)
{
    // Test getData for group header with invalid role
    ModelItemWrapper wrapper(&testGroup);
    
    auto result = wrapper.getData(999999);
    
    EXPECT_TRUE(result.isNull());
}

TEST_F(ModelItemWrapperTest, GetData_DefaultItem_ReturnsQVariant)
{
    // Test getData for default constructed item
    ModelItemWrapper wrapper;
    
    auto result = wrapper.getData(DFMBASE_NAMESPACE::Global::kItemUrlRole);
    
    EXPECT_TRUE(result.isNull());
}

TEST_F(ModelItemWrapperTest, IsValid_FileItemWithValidData_ReturnsTrue)
{
    // Test isValid for file item with valid data
    ModelItemWrapper wrapper(fileItem, "test_group");
    
    bool result = wrapper.isValid();
    
    EXPECT_TRUE(result);
}

TEST_F(ModelItemWrapperTest, IsValid_FileItemWithNullData_ReturnsFalse)
{
    // Test isValid for file item with null data
    ModelItemWrapper wrapper(nullptr, "test_group");
    
    bool result = wrapper.isValid();
    
    EXPECT_FALSE(result);
}

TEST_F(ModelItemWrapperTest, IsValid_GroupHeaderWithValues_ReturnsTrue)
{
    // Test isValid for group header with values
    ModelItemWrapper wrapper(&testGroup);
    
    bool result = wrapper.isValid();
    
    EXPECT_TRUE(result);
}

TEST_F(ModelItemWrapperTest, IsValid_GroupHeaderWithNullData_ReturnsFalse)
{
    // Test isValid for group header with null data
    ModelItemWrapper wrapper(nullptr);
    
    bool result = wrapper.isValid();
    
    EXPECT_FALSE(result);
}

TEST_F(ModelItemWrapperTest, IsValid_DefaultItem_ReturnsFalse)
{
    // Test isValid for default constructed item
    ModelItemWrapper wrapper;
    
    bool result = wrapper.isValid();
    
    EXPECT_FALSE(result);
}
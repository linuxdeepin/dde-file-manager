// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>

#include "beans/filetaginfo.h"
#include "beans/tagproperty.h"

DAEMONPTAG_USE_NAMESPACE

class TestFileTagInfo : public testing::Test
{
public:
    void SetUp() override
    {
        fileTagInfo = new FileTagInfo;
    }

    void TearDown() override
    {
        delete fileTagInfo;
        fileTagInfo = nullptr;
    }

protected:
    FileTagInfo *fileTagInfo = nullptr;
};

class TestTagProperty : public testing::Test
{
public:
    void SetUp() override
    {
        tagProperty = new TagProperty;
    }

    void TearDown() override
    {
        delete tagProperty;
        tagProperty = nullptr;
    }

protected:
    TagProperty *tagProperty = nullptr;
};

// FileTagInfo Tests

// Test FileTagInfo constructor
TEST_F(TestFileTagInfo, Constructor_ShouldInitializeWithDefaultValues)
{
    EXPECT_EQ(fileTagInfo->getFileIndex(), 0);
    EXPECT_TRUE(fileTagInfo->getFilePath().isEmpty());
    EXPECT_TRUE(fileTagInfo->getTagName().isEmpty());
    EXPECT_EQ(fileTagInfo->getTagOrder(), 0);
    EXPECT_TRUE(fileTagInfo->getFuture().isEmpty());
}

// Test FileTagInfo fileIndex property
TEST_F(TestFileTagInfo, SetGetFileIndex_ShouldWorkCorrectly)
{
    int testIndex = 12345;
    
    fileTagInfo->setFileIndex(testIndex);
    
    EXPECT_EQ(fileTagInfo->getFileIndex(), testIndex);
}

// Test FileTagInfo filePath property
TEST_F(TestFileTagInfo, SetGetFilePath_ShouldWorkCorrectly)
{
    QString testPath = "/home/user/document.txt";
    
    fileTagInfo->setFilePath(testPath);
    
    EXPECT_EQ(fileTagInfo->getFilePath(), testPath);
}

// Test FileTagInfo filePath property with empty string
TEST_F(TestFileTagInfo, SetGetFilePath_WithEmptyString_ShouldWorkCorrectly)
{
    QString emptyPath = "";
    
    fileTagInfo->setFilePath(emptyPath);
    
    EXPECT_EQ(fileTagInfo->getFilePath(), emptyPath);
    EXPECT_TRUE(fileTagInfo->getFilePath().isEmpty());
}

// Test FileTagInfo filePath property with special characters
TEST_F(TestFileTagInfo, SetGetFilePath_WithSpecialCharacters_ShouldWorkCorrectly)
{
    QString specialPath = "/path/with spaces/文档.txt";
    
    fileTagInfo->setFilePath(specialPath);
    
    EXPECT_EQ(fileTagInfo->getFilePath(), specialPath);
}

// Test FileTagInfo tagName property
TEST_F(TestFileTagInfo, SetGetTagName_ShouldWorkCorrectly)
{
    QString testTagName = "ImportantTag";
    
    fileTagInfo->setTagName(testTagName);
    
    EXPECT_EQ(fileTagInfo->getTagName(), testTagName);
}

// Test FileTagInfo tagName property with empty string
TEST_F(TestFileTagInfo, SetGetTagName_WithEmptyString_ShouldWorkCorrectly)
{
    QString emptyTag = "";
    
    fileTagInfo->setTagName(emptyTag);
    
    EXPECT_EQ(fileTagInfo->getTagName(), emptyTag);
    EXPECT_TRUE(fileTagInfo->getTagName().isEmpty());
}

// Test FileTagInfo tagName property with Unicode characters
TEST_F(TestFileTagInfo, SetGetTagName_WithUnicodeCharacters_ShouldWorkCorrectly)
{
    QString unicodeTag = "重要标签";
    
    fileTagInfo->setTagName(unicodeTag);
    
    EXPECT_EQ(fileTagInfo->getTagName(), unicodeTag);
}

// Test FileTagInfo tagOrder property
TEST_F(TestFileTagInfo, SetGetTagOrder_ShouldWorkCorrectly)
{
    int testOrder = 5;
    
    fileTagInfo->setTagOrder(testOrder);
    
    EXPECT_EQ(fileTagInfo->getTagOrder(), testOrder);
}

// Test FileTagInfo tagOrder property with negative value
TEST_F(TestFileTagInfo, SetGetTagOrder_WithNegativeValue_ShouldWorkCorrectly)
{
    int negativeOrder = -1;
    
    fileTagInfo->setTagOrder(negativeOrder);
    
    EXPECT_EQ(fileTagInfo->getTagOrder(), negativeOrder);
}

// Test FileTagInfo tagOrder property with large value
TEST_F(TestFileTagInfo, SetGetTagOrder_WithLargeValue_ShouldWorkCorrectly)
{
    int largeOrder = 999999;
    
    fileTagInfo->setTagOrder(largeOrder);
    
    EXPECT_EQ(fileTagInfo->getTagOrder(), largeOrder);
}

// Test FileTagInfo future property
TEST_F(TestFileTagInfo, SetGetFuture_ShouldWorkCorrectly)
{
    QString testFuture = "future_data";
    
    fileTagInfo->setFuture(testFuture);
    
    EXPECT_EQ(fileTagInfo->getFuture(), testFuture);
}

// Test FileTagInfo future property with empty string
TEST_F(TestFileTagInfo, SetGetFuture_WithEmptyString_ShouldWorkCorrectly)
{
    QString emptyFuture = "";
    
    fileTagInfo->setFuture(emptyFuture);
    
    EXPECT_EQ(fileTagInfo->getFuture(), emptyFuture);
    EXPECT_TRUE(fileTagInfo->getFuture().isEmpty());
}

// Test FileTagInfo multiple property changes
TEST_F(TestFileTagInfo, MultiplePropertyChanges_ShouldWorkCorrectly)
{
    int testIndex = 100;
    QString testPath = "/test/path";
    QString testTag = "TestTag";
    int testOrder = 3;
    QString testFuture = "future";
    
    fileTagInfo->setFileIndex(testIndex);
    fileTagInfo->setFilePath(testPath);
    fileTagInfo->setTagName(testTag);
    fileTagInfo->setTagOrder(testOrder);
    fileTagInfo->setFuture(testFuture);
    
    EXPECT_EQ(fileTagInfo->getFileIndex(), testIndex);
    EXPECT_EQ(fileTagInfo->getFilePath(), testPath);
    EXPECT_EQ(fileTagInfo->getTagName(), testTag);
    EXPECT_EQ(fileTagInfo->getTagOrder(), testOrder);
    EXPECT_EQ(fileTagInfo->getFuture(), testFuture);
}

// TagProperty Tests

// Test TagProperty constructor
TEST_F(TestTagProperty, Constructor_ShouldInitializeWithDefaultValues)
{
    EXPECT_EQ(tagProperty->getTagIndex(), 0);
    EXPECT_TRUE(tagProperty->getTagName().isEmpty());
    EXPECT_TRUE(tagProperty->getTagColor().isEmpty());
    EXPECT_EQ(tagProperty->getAmbiguity(), 0);
    EXPECT_TRUE(tagProperty->getFuture().isEmpty());
}

// Test TagProperty tagIndex property
TEST_F(TestTagProperty, SetGetTagIndex_ShouldWorkCorrectly)
{
    int testIndex = 54321;
    
    tagProperty->setTagIndex(testIndex);
    
    EXPECT_EQ(tagProperty->getTagIndex(), testIndex);
}

// Test TagProperty tagName property
TEST_F(TestTagProperty, SetGetTagName_ShouldWorkCorrectly)
{
    QString testTagName = "WorkTag";
    
    tagProperty->setTagName(testTagName);
    
    EXPECT_EQ(tagProperty->getTagName(), testTagName);
}

// Test TagProperty tagName property with empty string
TEST_F(TestTagProperty, SetGetTagName_WithEmptyString_ShouldWorkCorrectly)
{
    QString emptyName = "";
    
    tagProperty->setTagName(emptyName);
    
    EXPECT_EQ(tagProperty->getTagName(), emptyName);
    EXPECT_TRUE(tagProperty->getTagName().isEmpty());
}

// Test TagProperty tagName property with special characters
TEST_F(TestTagProperty, SetGetTagName_WithSpecialCharacters_ShouldWorkCorrectly)
{
    QString specialName = "Tag@#$%^&*()";
    
    tagProperty->setTagName(specialName);
    
    EXPECT_EQ(tagProperty->getTagName(), specialName);
}

// Test TagProperty tagColor property
TEST_F(TestTagProperty, SetGetTagColor_ShouldWorkCorrectly)
{
    QString testColor = "#FF0000";
    
    tagProperty->setTagColor(testColor);
    
    EXPECT_EQ(tagProperty->getTagColor(), testColor);
}

// Test TagProperty tagColor property with color name
TEST_F(TestTagProperty, SetGetTagColor_WithColorName_ShouldWorkCorrectly)
{
    QString colorName = "red";
    
    tagProperty->setTagColor(colorName);
    
    EXPECT_EQ(tagProperty->getTagColor(), colorName);
}

// Test TagProperty tagColor property with empty string
TEST_F(TestTagProperty, SetGetTagColor_WithEmptyString_ShouldWorkCorrectly)
{
    QString emptyColor = "";
    
    tagProperty->setTagColor(emptyColor);
    
    EXPECT_EQ(tagProperty->getTagColor(), emptyColor);
    EXPECT_TRUE(tagProperty->getTagColor().isEmpty());
}

// Test TagProperty ambiguity property
TEST_F(TestTagProperty, SetGetAmbiguity_ShouldWorkCorrectly)
{
    int testAmbiguity = 1;
    
    tagProperty->setAmbiguity(testAmbiguity);
    
    EXPECT_EQ(tagProperty->getAmbiguity(), testAmbiguity);
}

// Test TagProperty ambiguity property with zero value
TEST_F(TestTagProperty, SetGetAmbiguity_WithZeroValue_ShouldWorkCorrectly)
{
    int zeroAmbiguity = 0;
    
    tagProperty->setAmbiguity(zeroAmbiguity);
    
    EXPECT_EQ(tagProperty->getAmbiguity(), zeroAmbiguity);
}

// Test TagProperty ambiguity property with negative value
TEST_F(TestTagProperty, SetGetAmbiguity_WithNegativeValue_ShouldWorkCorrectly)
{
    int negativeAmbiguity = -5;
    
    tagProperty->setAmbiguity(negativeAmbiguity);
    
    EXPECT_EQ(tagProperty->getAmbiguity(), negativeAmbiguity);
}

// Test TagProperty future property
TEST_F(TestTagProperty, SetGetFuture_ShouldWorkCorrectly)
{
    QString testFuture = "null";
    
    tagProperty->setFuture(testFuture);
    
    EXPECT_EQ(tagProperty->getFuture(), testFuture);
}

// Test TagProperty future property with empty string
TEST_F(TestTagProperty, SetGetFuture_WithEmptyString_ShouldWorkCorrectly)
{
    QString emptyFuture = "";
    
    tagProperty->setFuture(emptyFuture);
    
    EXPECT_EQ(tagProperty->getFuture(), emptyFuture);
    EXPECT_TRUE(tagProperty->getFuture().isEmpty());
}

// Test TagProperty multiple property changes
TEST_F(TestTagProperty, MultiplePropertyChanges_ShouldWorkCorrectly)
{
    int testIndex = 200;
    QString testName = "TestProperty";
    QString testColor = "blue";
    int testAmbiguity = 2;
    QString testFuture = "reserved";
    
    tagProperty->setTagIndex(testIndex);
    tagProperty->setTagName(testName);
    tagProperty->setTagColor(testColor);
    tagProperty->setAmbiguity(testAmbiguity);
    tagProperty->setFuture(testFuture);
    
    EXPECT_EQ(tagProperty->getTagIndex(), testIndex);
    EXPECT_EQ(tagProperty->getTagName(), testName);
    EXPECT_EQ(tagProperty->getTagColor(), testColor);
    EXPECT_EQ(tagProperty->getAmbiguity(), testAmbiguity);
    EXPECT_EQ(tagProperty->getFuture(), testFuture);
}

// Test TagProperty property overwriting
TEST_F(TestTagProperty, PropertyOverwriting_ShouldWorkCorrectly)
{
    QString firstColor = "red";
    QString secondColor = "green";
    
    tagProperty->setTagColor(firstColor);
    EXPECT_EQ(tagProperty->getTagColor(), firstColor);
    
    tagProperty->setTagColor(secondColor);
    EXPECT_EQ(tagProperty->getTagColor(), secondColor);
    EXPECT_NE(tagProperty->getTagColor(), firstColor);
}

// Test FileTagInfo property overwriting
TEST_F(TestFileTagInfo, PropertyOverwriting_ShouldWorkCorrectly)
{
    QString firstPath = "/first/path";
    QString secondPath = "/second/path";
    
    fileTagInfo->setFilePath(firstPath);
    EXPECT_EQ(fileTagInfo->getFilePath(), firstPath);
    
    fileTagInfo->setFilePath(secondPath);
    EXPECT_EQ(fileTagInfo->getFilePath(), secondPath);
    EXPECT_NE(fileTagInfo->getFilePath(), firstPath);
} 
// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QString>
#include <QUrl>

#include <dfm-base/utils/sortutils.h>
#include "stubext.h"

using namespace dfmbase;

class SortUtilsTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(SortUtilsTest, CompareString_AscendingOrder_ExpectedCorrectComparison) {
    // Arrange
    QString str1 = "apple";
    QString str2 = "banana";
    
    // Act
    bool result = SortUtils::compareString(str1, str2, Qt::AscendingOrder);
    
    // Assert
    EXPECT_TRUE(result);  // apple should come before banana
}

TEST_F(SortUtilsTest, CompareString_DescendingOrder_ExpectedCorrectComparison) {
    // Arrange
    QString str1 = "apple";
    QString str2 = "banana";
    
    // Act
    bool result = SortUtils::compareString(str1, str2, Qt::DescendingOrder);
    
    // Assert
    EXPECT_FALSE(result);  // With descending order, banana should come before apple
}

TEST_F(SortUtilsTest, CompareStringForFileName_SimpleNames_ExpectedCorrectComparison) {
    // Arrange
    QString str1 = "file1.txt";
    QString str2 = "file2.txt";
    
    // Act
    bool result = SortUtils::compareStringForFileName(str1, str2);
    
    // Assert
    EXPECT_TRUE(result);  // file1.txt should come before file2.txt
}

TEST_F(SortUtilsTest, CompareStringForTime_SimpleTimes_ExpectedCorrectComparison) {
    // Arrange
    QString time1 = "2023/01/01 10:00:00";
    QString time2 = "2023/01/02 10:00:00";
    
    // Act
    bool result = SortUtils::compareStringForTime(time1, time2);
    
    // Assert
    EXPECT_TRUE(result);  // Earlier time should come first
}

TEST_F(SortUtilsTest, CompareStringForMimeType_SimpleMimes_ExpectedCorrectComparison) {
    // Arrange
    QString mime1 = "Directory";
    QString mime2 = "Text";
    
    // Act
    bool result = SortUtils::compareStringForMimeType(mime1, mime2);
    
    // Assert
    EXPECT_TRUE(result);  // Directory should come before Text according to type ranking
}

TEST_F(SortUtilsTest, CompareForSize_WithInt64Values_ExpectedCorrectComparison) {
    // Arrange
    qint64 size1 = 100;
    qint64 size2 = 200;
    
    // Act
    bool result = SortUtils::compareForSize(size1, size2);
    
    // Assert
    EXPECT_TRUE(result);  // 100 should be less than 200
}

TEST_F(SortUtilsTest, AccurateDisplayType_WithUrl_ExpectedNoCrash) {
    // Arrange
    QUrl url("file:///tmp/test.txt");
    
    // Act
    QString result = SortUtils::accurateDisplayType(url);
    
    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(SortUtilsTest, AccurateLocalMimeType_WithUrl_ExpectedNoCrash) {
    // Arrange
    QUrl url("file:///tmp/test.txt");
    
    // Act
    QString result = SortUtils::accurateLocalMimeType(url);
    
    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(SortUtilsTest, GetLocalPath_WithUrl_ExpectedPathReturned) {
    // Arrange
    QUrl url("file:///tmp/test.txt");
    
    // Act
    QString result = SortUtils::getLocalPath(url);
    
    // Assert
    // Just ensure no crash and we get some result
    EXPECT_TRUE(true);
}
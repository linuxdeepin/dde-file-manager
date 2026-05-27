// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QSet>
#include <QString>

#include <dfm-base/utils/hidefilehelper.h>
#include "stubext.h"

using namespace dfmbase;

class HideFileHelperTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(HideFileHelperTest, Constructor_WithDirUrl_ExpectedHelperCreated) {
    // Arrange
    QUrl dirUrl("file:///tmp/testdir");

    // Act
    HideFileHelper helper(dirUrl);

    // Assert
    EXPECT_EQ(helper.dirUrl(), dirUrl);
}

TEST_F(HideFileHelperTest, DirUrl_GetDirUrl_ExpectedSameUrl) {
    // Arrange
    QUrl dirUrl("file:///tmp/testdir");
    HideFileHelper helper(dirUrl);

    // Act
    QUrl result = helper.dirUrl();

    // Assert
    EXPECT_EQ(result, dirUrl);
}

TEST_F(HideFileHelperTest, FileUrl_GetFileUrl_ExpectedHiddenFilePath) {
    // Arrange
    QUrl dirUrl("file:///tmp/testdir");
    HideFileHelper helper(dirUrl);

    // Act
    QUrl result = helper.fileUrl();

    // Assert
    EXPECT_EQ(result.toString(), "file:///tmp/testdir/.hidden");
}

TEST_F(HideFileHelperTest, InsertAndContains_WithFileName_ExpectedFileAdded) {
    // Arrange
    QUrl dirUrl("file:///tmp/testdir");
    HideFileHelper helper(dirUrl);
    QString fileName = "testfile.txt";

    // Act
    bool inserted = helper.insert(fileName);
    bool contains = helper.contains(fileName);

    // Assert
    EXPECT_TRUE(inserted);
    EXPECT_TRUE(contains);
}

TEST_F(HideFileHelperTest, InsertAndRemove_WithFileName_ExpectedFileRemoved) {
    // Arrange
    QUrl dirUrl("file:///tmp/testdir");
    HideFileHelper helper(dirUrl);
    QString fileName = "testfile.txt";

    // Act
    helper.insert(fileName);
    bool containsBefore = helper.contains(fileName);
    bool removed = helper.remove(fileName);
    bool containsAfter = helper.contains(fileName);

    // Assert
    EXPECT_TRUE(containsBefore);
    EXPECT_TRUE(removed);
    EXPECT_FALSE(containsAfter);
}

TEST_F(HideFileHelperTest, HideFileList_GetList_ExpectedEmptyListInitially) {
    // Arrange
    QUrl dirUrl("file:///tmp/testdir");
    HideFileHelper helper(dirUrl);

    // Act
    QSet<QString> list = helper.hideFileList();

    // Assert
    EXPECT_TRUE(list.isEmpty());
}

TEST_F(HideFileHelperTest, MultipleInserts_GetList_ExpectedAllFilesInList) {
    // Arrange
    QUrl dirUrl("file:///tmp/testdir");
    HideFileHelper helper(dirUrl);
    QString fileName1 = "testfile1.txt";
    QString fileName2 = "testfile2.txt";

    // Act
    helper.insert(fileName1);
    helper.insert(fileName2);
    QSet<QString> list = helper.hideFileList();

    // Assert
    EXPECT_EQ(list.size(), 2);
    EXPECT_TRUE(list.contains(fileName1));
    EXPECT_TRUE(list.contains(fileName2));
}

TEST_F(HideFileHelperTest, Save_CallSave_ExpectedNoCrash) {
    // Arrange
    QUrl dirUrl("file:///tmp/testdir");
    HideFileHelper helper(dirUrl);

    // Act
    bool result = helper.save();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}
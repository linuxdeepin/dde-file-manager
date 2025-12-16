// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QList>

#include <dfm-base/widgets/dfmstatusbar/basicstatusbar.h>
#include <dfm-base/interfaces/fileinfo.h>
#include "stubext.h"

using namespace dfmbase;

class BasicStatusBarTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(BasicStatusBarTest, Constructor_WithParent_ExpectedStatusBarCreated) {
    // Arrange
    QWidget parent;

    // Act
    BasicStatusBar statusBar(&parent);

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarTest, Constructor_WithoutParent_ExpectedStatusBarCreated) {
    // Act
    BasicStatusBar statusBar;

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarTest, SizeHint_Call_ExpectedValidSize) {
    // Arrange
    QWidget parent;
    BasicStatusBar statusBar(&parent);

    // Act
    QSize size = statusBar.sizeHint();

    // Assert
    EXPECT_TRUE(!size.isEmpty());
}

TEST_F(BasicStatusBarTest, ClearLayoutAndAnchors_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    BasicStatusBar statusBar(&parent);

    // Act
    statusBar.clearLayoutAndAnchors();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarTest, ItemSelected_CallWithCounts_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    BasicStatusBar statusBar(&parent);
    int selectFiles = 2;
    int selectFolders = 1;
    qint64 filesize = 1024;
    QList<QUrl> selectFolderList;
    selectFolderList << QUrl("file:///tmp/test");

    // Act
    statusBar.itemSelected(selectFiles, selectFolders, filesize, selectFolderList);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarTest, ItemSelected_CallWithInfoList_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    BasicStatusBar statusBar(&parent);
    QList<FileInfo *> infoList;

    // Act
    statusBar.itemSelected(infoList);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarTest, ItemCounted_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    BasicStatusBar statusBar(&parent);
    int count = 5;

    // Act
    statusBar.itemCounted(count);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarTest, UpdateStatusMessage_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    BasicStatusBar statusBar(&parent);

    // Act
    statusBar.updateStatusMessage();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarTest, InsertWidget_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    BasicStatusBar statusBar(&parent);
    QWidget *widget = new QWidget();  // Create widget on heap to be managed by parent

    // Act
    statusBar.insertWidget(0, widget);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarTest, AddWidget_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    BasicStatusBar statusBar(&parent);
    QWidget *widget = new QWidget();  // Create widget on heap to be managed by parent

    // Act
    statusBar.addWidget(widget);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarTest, SetTipText_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    BasicStatusBar statusBar(&parent);
    QString tipText = "Test tip";

    // Act
    statusBar.setTipText(tipText);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}
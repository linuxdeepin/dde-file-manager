// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QUrl>
#include <QList>

#include <dfm-base/widgets/dfmstatusbar/basicstatusbar.h>
#include <dfm-base/widgets/dfmstatusbar/private/basicstatusbar_p.h>
#include "stubext.h"

using namespace dfmbase;

class BasicStatusBarPrivateTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(BasicStatusBarPrivateTest, Constructor_WithParent_ExpectedPrivateCreated) {
    // Arrange
    BasicStatusBar parent;

    // Act
    BasicStatusBarPrivate privateObj(&parent);

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarPrivateTest, InitFormatStrings_Call_ExpectedNoCrash) {
    // Arrange
    BasicStatusBar parent;
    BasicStatusBarPrivate privateObj(&parent);

    // Act
    privateObj.initFormatStrings();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarPrivateTest, InitTipLabel_Call_ExpectedNoCrash) {
    // Arrange
    BasicStatusBar parent;
    BasicStatusBarPrivate privateObj(&parent);

    // Act
    privateObj.initTipLabel();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarPrivateTest, InitLayout_Call_ExpectedNoCrash) {
    // Arrange
    BasicStatusBar parent;
    BasicStatusBarPrivate privateObj(&parent);

    // Act
    privateObj.initLayout();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarPrivateTest, CalcFolderContains_Call_ExpectedNoCrash) {
    // Arrange
    BasicStatusBar parent;
    BasicStatusBarPrivate privateObj(&parent);
    QList<QUrl> folderList;
    folderList << QUrl("file:///tmp");

    // Act
    privateObj.calcFolderContains(folderList);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarPrivateTest, DiscardCurrentJob_Call_ExpectedNoCrash) {
    // Arrange
    BasicStatusBar parent;
    BasicStatusBarPrivate privateObj(&parent);

    // Act
    privateObj.discardCurrentJob();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(BasicStatusBarPrivateTest, InitJobConnection_Call_ExpectedNoCrash) {
    // Arrange
    BasicStatusBar parent;
    BasicStatusBarPrivate privateObj(&parent);

    // Act
    privateObj.initJobConnection();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}
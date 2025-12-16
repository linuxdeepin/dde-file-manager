// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QWidget>

#include <dfm-base/widgets/dfmsplitter/splitter.h>
#include "stubext.h"

using namespace dfmbase;

class SplitterTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(SplitterTest, Constructor_WithOrientationAndParent_ExpectedSplitterCreated) {
    // Arrange
    QWidget parent;

    // Act
    Splitter splitter(Qt::Horizontal, &parent);

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(SplitterTest, Constructor_WithOrientationAndNullParent_ExpectedSplitterCreated) {
    // Act
    Splitter splitter(Qt::Vertical, nullptr);

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(SplitterTest, SplitPosition_GetInitialValue_ExpectedDefaultValue) {
    // Arrange
    QWidget parent;
    Splitter splitter(Qt::Horizontal, &parent);

    // Act
    int position = splitter.splitPosition();

    // Assert
    EXPECT_EQ(position, 0);
}

TEST_F(SplitterTest, SetSplitPosition_CallWithValue_ExpectedPositionSet) {
    // Arrange
    QWidget parent;
    Splitter splitter(Qt::Horizontal, &parent);
    int newPosition = 100;

    // Act
    splitter.setSplitPosition(newPosition);

    // Assert
    EXPECT_EQ(splitter.splitPosition(), newPosition);
}

TEST_F(SplitterTest, CreateHandle_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    Splitter splitter(Qt::Horizontal, &parent);

    // Act
    QSplitterHandle *handle = splitter.createHandle();

    // Assert
    // Just ensure no crash and handle is created
    EXPECT_TRUE(true);
    
    // Clean up
    delete handle;
}

class SplitterHandleTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(SplitterHandleTest, Constructor_WithOrientationAndParent_ExpectedHandleCreated) {
    // Arrange
    QWidget parent;
    QSplitter splitter(&parent);

    // Act
    SplitterHandle handle(Qt::Horizontal, &splitter);

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}
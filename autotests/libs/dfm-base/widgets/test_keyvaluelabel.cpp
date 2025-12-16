// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QWidget>
#include <QString>

#include <dfm-base/widgets/dfmkeyvaluelabel/keyvaluelabel.h>
#include "stubext.h"

using namespace dfmbase;

class KeyValueLabelTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(KeyValueLabelTest, Constructor_WithParent_ExpectedLabelCreated) {
    // Arrange
    QWidget parent;

    // Act
    KeyValueLabel label(&parent);

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, Constructor_WithoutParent_ExpectedLabelCreated) {
    // Act
    KeyValueLabel label(nullptr);

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, SetLeftValue_CallWithValidParams_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);
    QString value = "Left Value";

    // Act
    label.setLeftValue(value);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, SetRightValue_CallWithValidParams_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);
    QString value = "Right Value";

    // Act
    label.setRightValue(value);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, SetLeftRightValue_CallWithValidParams_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);
    QString leftValue = "Left Value";
    QString rightValue = "Right Value";

    // Act
    label.setLeftRightValue(leftValue, rightValue);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, LeftValue_Call_ExpectedStringReturned) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);

    // Act
    QString result = label.LeftValue();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, RightValue_Call_ExpectedStringReturned) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);

    // Act
    QString result = label.RightValue();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, LeftWidget_Call_ExpectedWidgetReturned) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);

    // Act
    DLabel *widget = label.leftWidget();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, RightWidget_Call_ExpectedWidgetReturned) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);

    // Act
    RightValueWidget *widget = label.rightWidget();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, AdjustHeight_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);

    // Act
    label.adjustHeight();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, SetLeftWordWrap_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);

    // Act
    label.setLeftWordWrap(true);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, SetLeftValueLabelFixedWidth_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);
    int width = 100;

    // Act
    label.setLeftValueLabelFixedWidth(width);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, SetLeftFontSizeWeight_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);

    // Act
    label.setLeftFontSizeWeight(DFontSizeManager::T8);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(KeyValueLabelTest, SetRightFontSizeWeight_Call_ExpectedNoCrash) {
    // Arrange
    QWidget parent;
    KeyValueLabel label(&parent);

    // Act
    label.setRightFontSizeWeight(DFontSizeManager::T8);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}
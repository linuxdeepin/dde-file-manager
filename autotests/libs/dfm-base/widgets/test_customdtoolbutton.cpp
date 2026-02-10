// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QWidget>

#include <dfm-base/widgets/dfmcustombuttons/customdtoolbutton.h>
#include "stubext.h"

using namespace dfmbase;

class CustomDToolButtonTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(CustomDToolButtonTest, Constructor_WithParent_ExpectedButtonCreated) {
    // Arrange
    QWidget parent;

    // Act
    CustomDToolButton button(&parent);

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(CustomDToolButtonTest, Constructor_WithoutParent_ExpectedButtonCreated) {
    // Act
    CustomDToolButton button;

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(CustomDToolButtonTest, PaintEvent_Call_ExpectedNoCrash) {
    // Arrange
    CustomDToolButton button;
    QPaintEvent event(QRect(0, 0, 100, 100));

    // Act
    button.paintEvent(&event);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(CustomDToolButtonTest, InitStyleOption_Call_ExpectedNoCrash) {
    // Arrange
    CustomDToolButton button;
    QStyleOptionToolButton option;

    // Act
    button.initStyleOption(&option);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}
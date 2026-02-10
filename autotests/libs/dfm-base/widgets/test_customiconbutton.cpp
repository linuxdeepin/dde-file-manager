// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QWidget>

#include <dfm-base/widgets/dfmcustombuttons/customiconbutton.h>
#include "stubext.h"

using namespace dfmbase;

class CustomDIconButtonTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(CustomDIconButtonTest, Constructor_WithParent_ExpectedButtonCreated) {
    // Arrange
    QWidget parent;

    // Act
    CustomDIconButton button(&parent);

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(CustomDIconButtonTest, Constructor_WithIconTypeAndParent_ExpectedButtonCreated) {
    // Arrange
    QWidget parent;

    // Act
    CustomDIconButton button(DTK_NAMESPACE::Widget::DStyle::StandardPixmap::SP_CloseButton, &parent);

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(CustomDIconButtonTest, Constructor_WithoutParent_ExpectedButtonCreated) {
    // Act
    CustomDIconButton button;

    // Assert
    // Just ensure no crash during construction
    EXPECT_TRUE(true);
}

TEST_F(CustomDIconButtonTest, PaintEvent_Call_ExpectedNoCrash) {
    // Arrange
    CustomDIconButton button;
    QPaintEvent event(QRect(0, 0, 100, 100));

    // Act
    button.paintEvent(&event);

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}
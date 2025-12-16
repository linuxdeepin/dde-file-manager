// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QApplication>
#include <QScreen>

#include <dfm-base/utils/windowutils.h>
#include "stubext.h"

using namespace dfmbase;

class WindowUtilsTest : public testing::Test {
protected:
    void SetUp() override {
        stub.clear();
    }

    void TearDown() override {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

TEST_F(WindowUtilsTest, IsX11_Call_ExpectedNoCrash) {
    // Act
    bool result = WindowUtils::isX11();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(WindowUtilsTest, IsWayLand_Call_ExpectedNoCrash) {
    // Act
    bool result = WindowUtils::isWayLand();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(WindowUtilsTest, KeyShiftIsPressed_Call_ExpectedNoCrash) {
    // Act
    bool result = WindowUtils::keyShiftIsPressed();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(WindowUtilsTest, KeyCtrlIsPressed_Call_ExpectedNoCrash) {
    // Act
    bool result = WindowUtils::keyCtrlIsPressed();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(WindowUtilsTest, KeyAltIsPressed_Call_ExpectedNoCrash) {
    // Act
    bool result = WindowUtils::keyAltIsPressed();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(WindowUtilsTest, CursorScreen_Call_ExpectedNoCrash) {
    // Act
    QScreen *screen = WindowUtils::cursorScreen();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}

TEST_F(WindowUtilsTest, CloseAllFileManagerWindows_Call_ExpectedNoCrash) {
    // Act
    WindowUtils::closeAllFileManagerWindows();

    // Assert
    // Just ensure no crash
    EXPECT_TRUE(true);
}
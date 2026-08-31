// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_canvasdbusinterface_1.cpp
 * @brief Unit tests for CanvasDBusInterface Low-priority methods
 */

#include <gtest/gtest.h>

class CanvasDBusInterfaceTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(CanvasDBusInterfaceTest, EnableUIDebug)
{
    // EnableUIDebug
    SUCCEED();
}


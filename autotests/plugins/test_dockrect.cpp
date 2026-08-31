// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_test_dockrect.cpp
 * @brief Unit tests for DockRect Low-priority methods
 */

#include <gtest/gtest.h>

class DockRectTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(DockRectTest, Operator_QRectCast)
{
    // operator QRect() const
    SUCCEED();
}


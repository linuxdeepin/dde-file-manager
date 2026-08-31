// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_displayrect.cpp
 * @brief Unit tests for DisplayRect methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/dbus-private/dbusdisplay1.h"

#include <QTest>

using namespace ddplugin_core;

class DisplayRectTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DisplayRect();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DisplayRect *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DisplayRectTest, operator QRect() const)
{
    // Test method:  operator QRect() const(())
    EXPECT_NO_FATAL_FAILURE(obj->operator QRect() const());
}

TEST_F(DisplayRectTest, x)
{
    // Test getter: qint16 x()
    auto result = obj->x();
    EXPECT_EQ(result, 0);

}

TEST_F(DisplayRectTest, y)
{
    // Test getter: qint16 y()
    auto result = obj->y();
    EXPECT_EQ(result, 0);

}

TEST_F(DisplayRectTest, width)
{
    // Test getter: quint16 width()
    auto result = obj->width();
    EXPECT_EQ(result, 0);

}

TEST_F(DisplayRectTest, height)
{
    // Test getter: quint16 height()
    auto result = obj->height();
    EXPECT_EQ(result, 0);

}

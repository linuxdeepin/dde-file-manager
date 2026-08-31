// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dockrect.cpp
 * @brief Unit tests for DockRect methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/dbus-private/dbusdock1.h"

#include <QTest>

using namespace ddplugin_core;

class DockRectTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DockRect();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DockRect *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DockRectTest, operator QRect() const)
{
    // Test method:  operator QRect() const(())
    EXPECT_NO_FATAL_FAILURE(obj->operator QRect() const());
}

TEST_F(DockRectTest, x)
{
    // Test getter: qint32 x()
    auto result = obj->x();
    EXPECT_EQ(result, 0);

}

TEST_F(DockRectTest, y)
{
    // Test getter: qint32 y()
    auto result = obj->y();
    EXPECT_EQ(result, 0);

}

TEST_F(DockRectTest, width)
{
    // Test getter: qint32 width()
    auto result = obj->width();
    EXPECT_EQ(result, 0);

}

TEST_F(DockRectTest, height)
{
    // Test getter: qint32 height()
    auto result = obj->height();
    EXPECT_EQ(result, 0);

}

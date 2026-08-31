// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_screenqt.cpp
 * @brief Unit tests for ScreenQt methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/screenqt.h"

#include <QTest>

using namespace ddplugin_core;

class ScreenQtTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ScreenQt();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ScreenQt *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ScreenQtTest, geometry)
{
    // Test getter: QRect geometry()
    auto result = obj->geometry();
    EXPECT_FALSE(result.isValid());

}

TEST_F(ScreenQtTest, handleGeometry)
{
    // Test getter: QRect handleGeometry()
    auto result = obj->handleGeometry();
    EXPECT_FALSE(result.isValid());

}

TEST_F(ScreenQtTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

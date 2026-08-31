// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_screendbus.cpp
 * @brief Unit tests for ScreenDBus methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "screen/screendbus.h"

#include <QTest>

using namespace ddplugin_core;

class ScreenDBusTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ScreenDBus();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ScreenDBus *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ScreenDBusTest, enabled)
{
    // Test bool getter: enabled()
    bool result = obj->enabled();
    EXPECT_FALSE(result);

}

TEST_F(ScreenDBusTest, geometry)
{
    // Test getter: QRect geometry()
    auto result = obj->geometry();
    EXPECT_FALSE(result.isValid());

}

TEST_F(ScreenDBusTest, name)
{
    // Test getter: QString name()
    auto result = obj->name();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ScreenDBusTest, path)
{
    // Test getter: QString path()
    auto result = obj->path();
    EXPECT_TRUE(result.isEmpty());

}

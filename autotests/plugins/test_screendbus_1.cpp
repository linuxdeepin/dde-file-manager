// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_screendbus_1.cpp
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

TEST_F(ScreenDBusTest, handleGeometry)
{
    // Test getter: QRect handleGeometry()
    auto result = obj->handleGeometry();
    EXPECT_FALSE(result.isValid());

}

TEST_F(ScreenDBusTest, ScreenDBus_Destructor)
{
    // Test method:  ~ScreenDBus(())
    EXPECT_NO_FATAL_FAILURE({ ScreenDBus *tmp = new ScreenDBus(); delete tmp; });
}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_menu.cpp
 * @brief Unit tests for Menu methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menu.h"

#include <QTest>

using namespace dfmplugin_menu;

class MenuTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new Menu();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    Menu *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MenuTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(MenuTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(MenuTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

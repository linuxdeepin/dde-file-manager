// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_menuutils.cpp
 * @brief Unit tests for MenuUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "menuscene/menuutils.h"

#include <QTest>

using namespace dfmplugin_menu;

class MenuUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MenuUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MenuUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MenuUtilsTest, perfectMenuParams)
{
    // Test getter: QVariantHash perfectMenuParams()
    auto result = obj->perfectMenuParams();
    EXPECT_TRUE(result.isEmpty());

}

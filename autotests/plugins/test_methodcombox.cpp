// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_methodcombox.cpp
 * @brief Unit tests for MethodComBox methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/methodgroup/methodcombox.h"

#include <QTest>

using namespace ddplugin_organizer;

class MethodComBoxTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MethodComBox();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MethodComBox *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MethodComBoxTest, currentMethod)
{
    // Test getter: int currentMethod()
    auto result = obj->currentMethod();
    EXPECT_EQ(result, 0);

}

TEST_F(MethodComBoxTest, initCheckBox)
{
    // Test method: void initCheckBox(())
    EXPECT_NO_FATAL_FAILURE(obj->initCheckBox());
}

TEST_F(MethodComBoxTest, setCurrentMethod)
{
    // Test setter: void setCurrentMethod((int idx))
    EXPECT_NO_FATAL_FAILURE(obj->setCurrentMethod(0));
}

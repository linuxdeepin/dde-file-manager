// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_methodgrouphelper.cpp
 * @brief Unit tests for MethodGroupHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "options/methodgroup/methodgrouphelper.h"

#include <QTest>

using namespace ddplugin_organizer;

class MethodGroupHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MethodGroupHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MethodGroupHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MethodGroupHelperTest, MethodGroupHelper)
{
    // Test constructor: MethodGroupHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(MethodGroupHelperTest, build)
{
    // Test bool getter: build()
    bool result = obj->build();
    EXPECT_FALSE(result);

}

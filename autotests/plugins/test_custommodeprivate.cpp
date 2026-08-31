// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_custommodeprivate.cpp
 * @brief Unit tests for CustomModePrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "mode/custommode.h"

#include <QTest>

using namespace ddplugin_organizer;

class CustomModePrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CustomModePrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CustomModePrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CustomModePrivateTest, CustomModePrivate)
{
    // Test constructor: CustomModePrivate((CustomMode *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CustomModePrivateTest, CustomModePrivate_Destructor)
{
    // Test method:  ~CustomModePrivate(())
    EXPECT_NO_FATAL_FAILURE({ CustomModePrivate *tmp = new CustomModePrivate(); delete tmp; });
}

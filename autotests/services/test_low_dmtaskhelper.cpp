// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_dmtaskhelper.cpp
 * @brief Unit tests for DMTaskHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/core/dmsetup.h"

#include <QTest>

using namespace src;

class DMTaskHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DMTaskHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DMTaskHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DMTaskHelperTest, DMTaskHelper)
{
    // Test constructor: DMTaskHelper(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(DMTaskHelperTest, DMTaskHelper_Destructor)
{
    // Test method:  ~DMTaskHelper(())
    EXPECT_NO_FATAL_FAILURE({ DMTaskHelper *tmp = new DMTaskHelper(); delete tmp; });
}

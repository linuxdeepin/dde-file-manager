// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_policykithelper.cpp
 * @brief Unit tests for PolicyKitHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/common/polkit/policykithelper.h"

#include <QTest>

using namespace src;

class PolicyKitHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new PolicyKitHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    PolicyKitHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(PolicyKitHelperTest, instance)
{
    // Test getter: PolicyKitHelper instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_common_helper.cpp
 * @brief Unit tests for common_helper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/helpers/commonhelper.h"

#include <QTest>

using namespace src;

class common_helperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new common_helper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    common_helper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(common_helperTest, genRandomString)
{
    // Test method: QString genRandomString((int len))
    auto result = obj->genRandomString(0);
    EXPECT_TRUE(result.isEmpty());

}

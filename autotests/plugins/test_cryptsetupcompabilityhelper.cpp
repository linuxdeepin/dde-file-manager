// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_cryptsetupcompabilityhelper.cpp
 * @brief Unit tests for CryptSetupCompabilityHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/helpers/cryptsetupcompabilityhelper.h"

#include <QTest>

using namespace src;

class CryptSetupCompabilityHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CryptSetupCompabilityHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CryptSetupCompabilityHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CryptSetupCompabilityHelperTest, instance)
{
    // Test getter: CryptSetupCompabilityHelper instance()
    auto result = obj->instance();
    EXPECT_TRUE(result.isEmpty());

}

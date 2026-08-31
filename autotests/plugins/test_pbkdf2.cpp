// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_pbkdf2.cpp
 * @brief Unit tests for pbkdf2 methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/operator/pbkdf2.h"

#include <QTest>

using namespace dfmplugin_vault;

class pbkdf2Test : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new pbkdf2();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    pbkdf2 *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(pbkdf2Test, charToHexadecimalChar)
{
    auto result0 = obj->charToHexadecimalChar(0);
    EXPECT_EQ(result0, '0');

    auto result9 = obj->charToHexadecimalChar(9);
    EXPECT_EQ(result9, '9');
}

TEST_F(pbkdf2Test, createRandomSalt)
{
    auto result = obj->createRandomSalt(16);
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(pbkdf2Test, octalToHexadecimal)
{
    const char *str = "test";
    auto result = obj->octalToHexadecimal(str, 4);
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(pbkdf2Test, pbkdf2EncrypyPassword)
{
    QString password = "testpassword";
    QString randSalt = "randomsalt";
    auto result = obj->pbkdf2EncrypyPassword(password, randSalt, 1000, 32);
    EXPECT_FALSE(result.isEmpty());
}

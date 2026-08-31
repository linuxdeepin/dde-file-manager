// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_cryptsetupcompabilityhelper.cpp
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

TEST_F(CryptSetupCompabilityHelperTest, CryptSetupCompabilityHelper)
{
    // Test constructor: CryptSetupCompabilityHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CryptSetupCompabilityHelperTest, CryptSetupCompabilityHelper_Destructor)
{
    // Test method:  ~CryptSetupCompabilityHelper(())
    EXPECT_NO_FATAL_FAILURE({ CryptSetupCompabilityHelper *tmp = new CryptSetupCompabilityHelper(); delete tmp; });
}

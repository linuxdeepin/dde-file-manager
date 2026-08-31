// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_burncheckstrategy.cpp
 * @brief Unit tests for BurnCheckStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burncheckstrategy.h"

#include <QTest>

using namespace dfmplugin_burn;

class BurnCheckStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BurnCheckStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BurnCheckStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BurnCheckStrategyTest, check)
{
    // Test bool getter: check()
    bool result = obj->check();
    EXPECT_FALSE(result);

}

TEST_F(BurnCheckStrategyTest, lastError)
{
    // Test getter: QString lastError()
    auto result = obj->lastError();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BurnCheckStrategyTest, validFile)
{
    // Test method: bool validFile((const QFileInfo &info))
    QFileInfo _arg0{};
    auto result = obj->validFile(_arg0);
    EXPECT_FALSE(result);

}

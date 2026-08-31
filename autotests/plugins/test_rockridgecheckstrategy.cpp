// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_rockridgecheckstrategy.cpp
 * @brief Unit tests for RockRidgeCheckStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burncheckstrategy.h"

#include <QTest>

using namespace dfmplugin_burn;

class RockRidgeCheckStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RockRidgeCheckStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RockRidgeCheckStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RockRidgeCheckStrategyTest, RockRidgeCheckStrategy)
{
    // Test constructor: RockRidgeCheckStrategy((const QString &path, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RockRidgeCheckStrategyTest, validFileNameBytes)
{
    // Test method: bool validFileNameBytes((const QString &fileName))
    QString _arg0{};
    auto result = obj->validFileNameBytes(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(RockRidgeCheckStrategyTest, validFilePathBytes)
{
    // Test method: bool validFilePathBytes((const QString &filePath))
    QString _arg0{};
    auto result = obj->validFilePathBytes(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(RockRidgeCheckStrategyTest, validFilePathDeepLength)
{
    // Test method: bool validFilePathDeepLength((const QString &filePath))
    QString _arg0{};
    auto result = obj->validFilePathDeepLength(_arg0);
    EXPECT_FALSE(result);

}

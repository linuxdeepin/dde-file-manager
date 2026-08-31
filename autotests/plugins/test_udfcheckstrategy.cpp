// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_udfcheckstrategy.cpp
 * @brief Unit tests for UDFCheckStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burncheckstrategy.h"

#include <QTest>

using namespace dfmplugin_burn;

class UDFCheckStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new UDFCheckStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    UDFCheckStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UDFCheckStrategyTest, UDFCheckStrategy)
{
    // Test constructor: UDFCheckStrategy((const QString &path, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(UDFCheckStrategyTest, validFileNameBytes)
{
    // Test method: bool validFileNameBytes((const QString &fileName))
    QString _arg0{};
    auto result = obj->validFileNameBytes(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(UDFCheckStrategyTest, validFilePathBytes)
{
    // Test method: bool validFilePathBytes((const QString &filePath))
    QString _arg0{};
    auto result = obj->validFilePathBytes(_arg0);
    EXPECT_FALSE(result);

}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_jolietcheckstrategy.cpp
 * @brief Unit tests for JolietCheckStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burncheckstrategy.h"

#include <QTest>

using namespace dfmplugin_burn;

class JolietCheckStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new JolietCheckStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    JolietCheckStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(JolietCheckStrategyTest, JolietCheckStrategy)
{
    // Test constructor: JolietCheckStrategy((const QString &path, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(JolietCheckStrategyTest, validFileNameCharacters)
{
    // Test method: bool validFileNameCharacters((const QString &fileName))
    QString _arg0{};
    auto result = obj->validFileNameCharacters(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(JolietCheckStrategyTest, validFilePathCharacters)
{
    // Test method: bool validFilePathCharacters((const QString &filePath))
    QString _arg0{};
    auto result = obj->validFilePathCharacters(_arg0);
    EXPECT_FALSE(result);

}

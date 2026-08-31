// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_iso9660checkstrategy.cpp
 * @brief Unit tests for ISO9660CheckStrategy methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/burncheckstrategy.h"

#include <QTest>

using namespace dfmplugin_burn;

class ISO9660CheckStrategyTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ISO9660CheckStrategy();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ISO9660CheckStrategy *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ISO9660CheckStrategyTest, ISO9660CheckStrategy)
{
    // Test constructor: ISO9660CheckStrategy((const QString &path, QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ISO9660CheckStrategyTest, validFileNameCharacters)
{
    // Test method: bool validFileNameCharacters((const QString &fileName))
    QString _arg0{};
    auto result = obj->validFileNameCharacters(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(ISO9660CheckStrategyTest, validFilePathDeepLength)
{
    // Test method: bool validFilePathDeepLength((const QString &filePath))
    QString _arg0{};
    auto result = obj->validFilePathDeepLength(_arg0);
    EXPECT_FALSE(result);

}

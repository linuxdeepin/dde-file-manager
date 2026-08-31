// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sqlitehelper.cpp
 * @brief Unit tests for SqliteHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/db/sqlitehelper.h"

#include <QTest>

using namespace src;

class SqliteHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SqliteHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SqliteHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SqliteHelperTest, excute)
{
    // Test bool getter: excute()
    bool result = obj->excute();
    EXPECT_FALSE(result);

}

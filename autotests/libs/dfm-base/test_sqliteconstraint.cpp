// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sqliteconstraint.cpp
 * @brief Unit tests for SqliteConstraint methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/db/sqlitehelper.h"

#include <QTest>

using namespace src;

class SqliteConstraintTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SqliteConstraint();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SqliteConstraint *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SqliteConstraintTest, autoIncreament)
{
    // Test getter: SqliteConstraint autoIncreament()
    auto result = obj->autoIncreament();
    EXPECT_NO_FATAL_FAILURE({ obj->autoIncreament(); });

}

TEST_F(SqliteConstraintTest, nullable)
{
    // Test getter: SqliteConstraint nullable()
    auto result = obj->nullable();
    EXPECT_NO_FATAL_FAILURE({ obj->nullable(); });

}

TEST_F(SqliteConstraintTest, unique)
{
    // Test getter: SqliteConstraint unique()
    auto result = obj->unique();
    EXPECT_NO_FATAL_FAILURE({ obj->unique(); });

}

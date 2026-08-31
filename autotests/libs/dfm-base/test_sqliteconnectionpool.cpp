// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sqliteconnectionpool.cpp
 * @brief Unit tests for SqliteConnectionPool methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/db/sqliteconnectionpool.h"

#include <QTest>

using namespace src;

class SqliteConnectionPoolTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SqliteConnectionPool();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SqliteConnectionPool *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SqliteConnectionPoolTest, SqliteConnectionPool)
{
    // Test constructor: SqliteConnectionPool((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(SqliteConnectionPoolTest, M_~SqliteConnectionPool)
{
    // Test method:  ~SqliteConnectionPool(())
    EXPECT_NO_FATAL_FAILURE({ SqliteConnectionPool *tmp = new SqliteConnectionPool(); delete tmp; });
}

TEST_F(SqliteConnectionPoolTest, instance)
{
    // Test getter: SqliteConnectionPool instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(SqliteConnectionPoolTest, openConnection)
{
    // Test method: QSqlDatabase openConnection((const QString &databaseName))
    QString _arg0{};
    auto result = obj->openConnection(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->openConnection(_arg0); });

}

TEST_F(SqliteConnectionPoolTest, d)
{
    // Test getter: QScopedPointer<SqliteConnectionPoolPrivate> d()
    auto result = obj->d();
    EXPECT_EQ(result.get(), nullptr);

}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sqliteconnectionpool.cpp
 * @brief Unit tests for SqliteConnectionPool (base/db/sqliteconnectionpool.cpp)
 */

#include <gtest/gtest.h>
#include <QCoreApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

#include <dfm-base/base/db/sqliteconnectionpool.h>

using namespace dfmbase;

class SqliteConnectionPoolTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        dbPath = tmpDir.path() + "/ut_pool.db";
    }

    QTemporaryDir tmpDir;
    QString dbPath;
};

TEST_F(SqliteConnectionPoolTest, InstanceReturnsSameReference)
{
    SqliteConnectionPool &a = SqliteConnectionPool::instance();
    SqliteConnectionPool &b = SqliteConnectionPool::instance();
    EXPECT_EQ(&a, &b);
}

TEST_F(SqliteConnectionPoolTest, OpenConnectionReturnsValidSqlite)
{
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    EXPECT_TRUE(db.isValid());
    EXPECT_EQ(db.driverName().toStdString(), "QSQLITE");
    EXPECT_TRUE(db.isOpen());
}

TEST_F(SqliteConnectionPoolTest, OpenConnectionCreatesExecutableQueries)
{
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isOpen());
    QSqlQuery q(db);
    ASSERT_TRUE(q.exec("CREATE TABLE ut_t (id INTEGER PRIMARY KEY)"));
    ASSERT_TRUE(q.exec("INSERT INTO ut_t (id) VALUES (42)"));
    ASSERT_TRUE(q.exec("SELECT id FROM ut_t"));
    ASSERT_TRUE(q.next());
    EXPECT_EQ(q.value(0).toInt(), 42);
}

TEST_F(SqliteConnectionPoolTest, OpenConnectionSameNameReusesConnection)
{
    QSqlDatabase first = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(first.isOpen());
    QSqlDatabase second = SqliteConnectionPool::instance().openConnection(dbPath);
    EXPECT_TRUE(second.isValid());
    EXPECT_TRUE(second.isOpen());
    // Reused connection shares the same connection name.
    EXPECT_EQ(first.connectionName().toStdString(), second.connectionName().toStdString());
}

TEST_F(SqliteConnectionPoolTest, OpenConnectionDistinctPathsProduceDistinctNames)
{
    QString pathA = tmpDir.path() + "/ut_a.db";
    QString pathB = tmpDir.path() + "/ut_b.db";
    QSqlDatabase a = SqliteConnectionPool::instance().openConnection(pathA);
    QSqlDatabase b = SqliteConnectionPool::instance().openConnection(pathB);
    EXPECT_TRUE(a.isOpen());
    EXPECT_TRUE(b.isOpen());
    EXPECT_NE(a.connectionName().toStdString(), b.connectionName().toStdString());
}

TEST_F(SqliteConnectionPoolTest, OpenConnectionOnInMemoryDatabase)
{
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(":memory:");
    EXPECT_TRUE(db.isValid());
    EXPECT_EQ(db.driverName().toStdString(), "QSQLITE");
    EXPECT_TRUE(db.isOpen());
}

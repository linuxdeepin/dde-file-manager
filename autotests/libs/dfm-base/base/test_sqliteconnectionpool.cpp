// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QThread>
#include <QCoreApplication>

#include "stubext.h"

#include <dfm-base/base/db/sqliteconnectionpool.h>

DFMBASE_USE_NAMESPACE

class TestSqliteConnectionPool : public testing::Test
{
public:
    void SetUp() override
    {
        // Initialize QCoreApplication if not already initialized
        if (!qApp) {
            int argc = 0;
            char **argv = nullptr;
            app = new QCoreApplication(argc, argv);
        }

        // Create a temporary database file
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        dbPath = tempDir->path() + "/test_database.db";
    }

    void TearDown() override
    {
        stub.clear();

        // Close all database connections
        QSqlDatabase::removeDatabase(QSqlDatabase::defaultConnection);

        tempDir.reset();

        if (app) {
            delete app;
            app = nullptr;
        }
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    QString dbPath;
    QCoreApplication *app { nullptr };
};

// ========== Basic Functionality Tests ==========

TEST_F(TestSqliteConnectionPool, instance_ReturnsSingleton)
{
    // Test that instance() returns the same singleton
    auto &instance1 = SqliteConnectionPool::instance();
    auto &instance2 = SqliteConnectionPool::instance();

    EXPECT_EQ(&instance1, &instance2);
}

TEST_F(TestSqliteConnectionPool, openConnection_ValidPath)
{
    // Test opening a connection with valid database path
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);

    EXPECT_TRUE(db.isValid());
    EXPECT_TRUE(db.isOpen());
    EXPECT_EQ(db.databaseName(), dbPath);
}

TEST_F(TestSqliteConnectionPool, openConnection_ReuseExistingConnection)
{
    // Test that opening the same database twice returns the same connection
    QSqlDatabase db1 = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db1.isValid());

    QString connectionName1 = db1.connectionName();

    QSqlDatabase db2 = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db2.isValid());

    QString connectionName2 = db2.connectionName();

    EXPECT_EQ(connectionName1, connectionName2);
}

TEST_F(TestSqliteConnectionPool, openConnection_ExecuteQuery)
{
    // Test executing a query on the opened connection
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());
    ASSERT_TRUE(db.isOpen());

    QSqlQuery query(db);
    bool success = query.exec("CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT);");

    EXPECT_TRUE(success);
    EXPECT_EQ(query.lastError().type(), QSqlError::NoError);
}

TEST_F(TestSqliteConnectionPool, openConnection_InsertAndQuery)
{
    // Test insert and query operations
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);

    // Create table
    ASSERT_TRUE(query.exec("CREATE TABLE users (id INTEGER PRIMARY KEY, username TEXT);"));

    // Insert data
    ASSERT_TRUE(query.exec("INSERT INTO users (id, username) VALUES (1, 'Alice');"));
    ASSERT_TRUE(query.exec("INSERT INTO users (id, username) VALUES (2, 'Bob');"));

    // Query data
    ASSERT_TRUE(query.exec("SELECT * FROM users;"));

    int rowCount = 0;
    while (query.next()) {
        rowCount++;
        int id = query.value(0).toInt();
        QString username = query.value(1).toString();

        EXPECT_TRUE(id == 1 || id == 2);
        EXPECT_TRUE(username == "Alice" || username == "Bob");
    }

    EXPECT_EQ(rowCount, 2);
}

TEST_F(TestSqliteConnectionPool, openConnection_MultipleConnections)
{
    // Test opening multiple different database connections
    QString dbPath2 = tempDir->path() + "/test_database2.db";

    QSqlDatabase db1 = SqliteConnectionPool::instance().openConnection(dbPath);
    QSqlDatabase db2 = SqliteConnectionPool::instance().openConnection(dbPath2);

    EXPECT_TRUE(db1.isValid());
    EXPECT_TRUE(db2.isValid());
    EXPECT_NE(db1.connectionName(), db2.connectionName());
    EXPECT_EQ(db1.databaseName(), dbPath);
    EXPECT_EQ(db2.databaseName(), dbPath2);
}

TEST_F(TestSqliteConnectionPool, openConnection_ConnectionNameFormat)
{
    // Test connection name format
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QString connectionName = db.connectionName();

    // Connection name should contain thread ID prefix
    EXPECT_TRUE(connectionName.startsWith("conn_"));
}

TEST_F(TestSqliteConnectionPool, openConnection_Transaction)
{
    // Test transaction support
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);
    query.exec("CREATE TABLE accounts (id INTEGER PRIMARY KEY, balance INTEGER);");

    // Start transaction
    EXPECT_TRUE(db.transaction());

    query.exec("INSERT INTO accounts (id, balance) VALUES (1, 100);");
    query.exec("INSERT INTO accounts (id, balance) VALUES (2, 200);");

    // Commit transaction
    EXPECT_TRUE(db.commit());

    // Verify data
    ASSERT_TRUE(query.exec("SELECT COUNT(*) FROM accounts;"));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 2);
}

TEST_F(TestSqliteConnectionPool, openConnection_Rollback)
{
    // Test transaction rollback
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);
    query.exec("CREATE TABLE logs (id INTEGER PRIMARY KEY, message TEXT);");
    query.exec("INSERT INTO logs (id, message) VALUES (1, 'initial');");

    // Start transaction
    EXPECT_TRUE(db.transaction());

    query.exec("INSERT INTO logs (id, message) VALUES (2, 'rollback_me');");

    // Rollback transaction
    EXPECT_TRUE(db.rollback());

    // Verify only initial data exists
    ASSERT_TRUE(query.exec("SELECT COUNT(*) FROM logs;"));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

TEST_F(TestSqliteConnectionPool, openConnection_IsolatedConnections)
{
    // Test that changes in one connection don't affect cached queries in another
    QSqlDatabase db1 = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db1.isValid());

    QSqlQuery query1(db1);
    query1.exec("CREATE TABLE items (id INTEGER PRIMARY KEY, name TEXT);");
    query1.exec("INSERT INTO items (id, name) VALUES (1, 'Item1');");

    // Open same database (should reuse connection)
    QSqlDatabase db2 = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db2.isValid());

    QSqlQuery query2(db2);
    ASSERT_TRUE(query2.exec("SELECT * FROM items;"));

    int count = 0;
    while (query2.next()) {
        count++;
    }

    EXPECT_EQ(count, 1);
}

// ========== Error Handling Tests ==========

TEST_F(TestSqliteConnectionPool, openConnection_InvalidSQL)
{
    // Test handling invalid SQL
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);
    bool success = query.exec("INVALID SQL STATEMENT;");

    EXPECT_FALSE(success);
    EXPECT_NE(query.lastError().type(), QSqlError::NoError);
}

TEST_F(TestSqliteConnectionPool, openConnection_TableAlreadyExists)
{
    // Test creating table twice
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);

    // Create table first time
    EXPECT_TRUE(query.exec("CREATE TABLE duplicate (id INTEGER);"));

    // Try to create again without IF NOT EXISTS - should fail
    EXPECT_FALSE(query.exec("CREATE TABLE duplicate (id INTEGER);"));
}

// ========== Persistence Tests ==========

TEST_F(TestSqliteConnectionPool, openConnection_DataPersistence)
{
    // Test data persistence across connection reopen
    {
        QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
        ASSERT_TRUE(db.isValid());

        QSqlQuery query(db);
        query.exec("CREATE TABLE persistent (id INTEGER PRIMARY KEY, value TEXT);");
        query.exec("INSERT INTO persistent (id, value) VALUES (1, 'persisted');");
    }

    // Reopen connection
    {
        QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
        ASSERT_TRUE(db.isValid());

        QSqlQuery query(db);
        ASSERT_TRUE(query.exec("SELECT * FROM persistent;"));
        ASSERT_TRUE(query.next());

        EXPECT_EQ(query.value(0).toInt(), 1);
        EXPECT_EQ(query.value(1).toString(), QString("persisted"));
    }
}

// ========== Edge Cases and Stress Tests ==========

TEST_F(TestSqliteConnectionPool, openConnection_EmptyTableOperations)
{
    // Test operations on empty table
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);
    query.exec("CREATE TABLE empty_table (id INTEGER, data TEXT);");

    ASSERT_TRUE(query.exec("SELECT * FROM empty_table;"));

    int rowCount = 0;
    while (query.next()) {
        rowCount++;
    }

    EXPECT_EQ(rowCount, 0);
}

TEST_F(TestSqliteConnectionPool, openConnection_LargeDataInsertion)
{
    // Test inserting multiple rows
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);
    query.exec("CREATE TABLE large_data (id INTEGER PRIMARY KEY, value INTEGER);");

    const int rowCount = 100;
    db.transaction();

    for (int i = 0; i < rowCount; ++i) {
        query.exec(QString("INSERT INTO large_data (id, value) VALUES (%1, %2);")
                           .arg(i).arg(i * 10));
    }

    db.commit();

    // Verify count
    ASSERT_TRUE(query.exec("SELECT COUNT(*) FROM large_data;"));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), rowCount);
}

TEST_F(TestSqliteConnectionPool, openConnection_SpecialCharactersInData)
{
    // Test handling special characters
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);
    query.exec("CREATE TABLE special_chars (id INTEGER PRIMARY KEY, text TEXT);");

    query.prepare("INSERT INTO special_chars (id, text) VALUES (?, ?);");
    query.addBindValue(1);
    query.addBindValue("Test's \"quoted\" text & symbols");
    EXPECT_TRUE(query.exec());

    // Verify data
    ASSERT_TRUE(query.exec("SELECT text FROM special_chars WHERE id = 1;"));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toString(), QString("Test's \"quoted\" text & symbols"));
}

TEST_F(TestSqliteConnectionPool, openConnection_NullValues)
{
    // Test NULL values
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);
    query.exec("CREATE TABLE nullable (id INTEGER PRIMARY KEY, optional_field TEXT);");

    query.prepare("INSERT INTO nullable (id, optional_field) VALUES (?, ?);");
    query.addBindValue(1);
    query.addBindValue(QVariant(QVariant::String));  // NULL value
    EXPECT_TRUE(query.exec());

    // Verify NULL
    ASSERT_TRUE(query.exec("SELECT optional_field FROM nullable WHERE id = 1;"));
    ASSERT_TRUE(query.next());
    EXPECT_TRUE(query.value(0).isNull());
}

// ========== Integration Tests ==========

TEST_F(TestSqliteConnectionPool, Integration_CompleteWorkflow)
{
    // Test complete workflow: create, insert, update, delete
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);

    // Create
    ASSERT_TRUE(query.exec("CREATE TABLE products (id INTEGER PRIMARY KEY, name TEXT, price REAL);"));

    // Insert
    ASSERT_TRUE(query.exec("INSERT INTO products (id, name, price) VALUES (1, 'Product1', 19.99);"));
    ASSERT_TRUE(query.exec("INSERT INTO products (id, name, price) VALUES (2, 'Product2', 29.99);"));

    // Update
    ASSERT_TRUE(query.exec("UPDATE products SET price = 24.99 WHERE id = 1;"));

    // Verify update
    ASSERT_TRUE(query.exec("SELECT price FROM products WHERE id = 1;"));
    ASSERT_TRUE(query.next());
    EXPECT_DOUBLE_EQ(query.value(0).toDouble(), 24.99);

    // Delete
    ASSERT_TRUE(query.exec("DELETE FROM products WHERE id = 2;"));

    // Verify delete
    ASSERT_TRUE(query.exec("SELECT COUNT(*) FROM products;"));
    ASSERT_TRUE(query.next());
    EXPECT_EQ(query.value(0).toInt(), 1);
}

TEST_F(TestSqliteConnectionPool, Integration_MultipleTablesJoin)
{
    // Test join operations between multiple tables
    QSqlDatabase db = SqliteConnectionPool::instance().openConnection(dbPath);
    ASSERT_TRUE(db.isValid());

    QSqlQuery query(db);

    // Create tables
    query.exec("CREATE TABLE authors (id INTEGER PRIMARY KEY, name TEXT);");
    query.exec("CREATE TABLE books (id INTEGER PRIMARY KEY, title TEXT, author_id INTEGER);");

    // Insert data
    query.exec("INSERT INTO authors (id, name) VALUES (1, 'Author1');");
    query.exec("INSERT INTO authors (id, name) VALUES (2, 'Author2');");
    query.exec("INSERT INTO books (id, title, author_id) VALUES (1, 'Book1', 1);");
    query.exec("INSERT INTO books (id, title, author_id) VALUES (2, 'Book2', 1);");
    query.exec("INSERT INTO books (id, title, author_id) VALUES (3, 'Book3', 2);");

    // Join query
    ASSERT_TRUE(query.exec("SELECT books.title, authors.name FROM books "
                           "JOIN authors ON books.author_id = authors.id "
                           "WHERE authors.id = 1;"));

    int resultCount = 0;
    while (query.next()) {
        resultCount++;
        QString authorName = query.value(1).toString();
        EXPECT_EQ(authorName, QString("Author1"));
    }

    EXPECT_EQ(resultCount, 2);
}

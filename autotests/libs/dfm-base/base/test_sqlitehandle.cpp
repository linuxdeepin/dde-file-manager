// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QObject>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QCoreApplication>

#include "stubext.h"

#include <dfm-base/base/db/sqlitehandle.h>
#include <dfm-base/base/db/sqlitehelper.h>

DFMBASE_USE_NAMESPACE

// Test entity for database operations
class User : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "users")

    Q_PROPERTY(int id READ id WRITE setId)
    Q_PROPERTY(QString username READ username WRITE setUsername)
    Q_PROPERTY(int age READ age WRITE setAge)
    Q_PROPERTY(double score READ score WRITE setScore)

public:
    explicit User(QObject *parent = nullptr) : QObject(parent) {}

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString username() const { return m_username; }
    void setUsername(const QString &name) { m_username = name; }

    int age() const { return m_age; }
    void setAge(int age) { m_age = age; }

    double score() const { return m_score; }
    void setScore(double score) { m_score = score; }

private:
    int m_id { 0 };
    QString m_username;
    int m_age { 0 };
    double m_score { 0.0 };
};

class TestSqliteHandle : public testing::Test
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

        // Create temporary database
        tempDir = std::make_unique<QTemporaryDir>();
        ASSERT_TRUE(tempDir->isValid());

        dbPath = tempDir->path() + "/test.db";
        handle = std::make_unique<SqliteHandle>(dbPath);
    }

    void TearDown() override
    {
        stub.clear();

        handle.reset();
        tempDir.reset();

        if (app) {
            delete app;
            app = nullptr;
        }
    }

protected:
    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    std::unique_ptr<SqliteHandle> handle;
    QString dbPath;
    QCoreApplication *app { nullptr };
};

// ========== Table Management Tests ==========

TEST_F(TestSqliteHandle, createTable_BasicTable)
{
    // Test creating a basic table
    bool result = handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    EXPECT_TRUE(result);

    // Verify table exists by inserting data
    User user;
    user.setUsername("test");
    user.setAge(25);
    user.setScore(90.5);

    int lastId = handle->insert(user);
    EXPECT_GT(lastId, 0);
}

TEST_F(TestSqliteHandle, createTable_WithConstraints)
{
    // Test creating table with multiple constraints
    bool result = handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"),
            SqliteConstraint::unique("username"));

    EXPECT_TRUE(result);
}

TEST_F(TestSqliteHandle, createTable_WithDefaultValue)
{
    // Test creating table with default value
    bool result = handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::defaultValue("age", 18));

    EXPECT_TRUE(result);
}

TEST_F(TestSqliteHandle, createTable_WithCheckConstraint)
{
    // Test creating table with check constraint
    Expression::ExprField ageField("", "age");
    bool result = handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::check(ageField >= 0));

    EXPECT_TRUE(result);
}

TEST_F(TestSqliteHandle, dropTable_ExistingTable)
{
    // Test dropping an existing table
    handle->createTable<User>(SqliteConstraint::primary("id"));

    bool result = handle->dropTable<User>();

    EXPECT_TRUE(result);
}

// ========== Insert Tests ==========

TEST_F(TestSqliteHandle, insert_BasicEntity)
{
    // Test inserting a basic entity
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user;
    user.setUsername("Alice");
    user.setAge(30);
    user.setScore(95.5);

    int lastId = handle->insert(user);

    EXPECT_GT(lastId, 0);
}

TEST_F(TestSqliteHandle, insert_MultipleEntities)
{
    // Test inserting multiple entities
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user1;
    user1.setUsername("Bob");
    user1.setAge(25);
    user1.setScore(80.0);

    User user2;
    user2.setUsername("Charlie");
    user2.setAge(35);
    user2.setScore(90.0);

    int id1 = handle->insert(user1);
    int id2 = handle->insert(user2);

    EXPECT_GT(id1, 0);
    EXPECT_GT(id2, 0);
    EXPECT_NE(id1, id2);
}

TEST_F(TestSqliteHandle, insert_WithCustomPK)
{
    // Test inserting with custom primary key
    handle->createTable<User>(SqliteConstraint::primary("id"));

    User user;
    user.setId(100);
    user.setUsername("CustomID");
    user.setAge(20);
    user.setScore(75.0);

    int lastId = handle->insert(user, true);  // customPK = true

    EXPECT_EQ(lastId, 100);
}

TEST_F(TestSqliteHandle, insert_SpecialCharacters)
{
    // Test inserting data with special characters
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user;
    user.setUsername("O'Brien");  // Single quote
    user.setAge(40);
    user.setScore(88.0);

    int lastId = handle->insert(user);

    EXPECT_GT(lastId, 0);

    // Verify data
    auto result = handle->query<User>()
                          .where(Expression::Field<User>("id") == lastId)
                          .toBean();

    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->username(), QString("O'Brien"));
}

// ========== Query Tests ==========

TEST_F(TestSqliteHandle, query_AllRecords)
{
    // Test querying all records
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user1;
    user1.setUsername("User1");
    user1.setAge(20);
    handle->insert(user1);

    User user2;
    user2.setUsername("User2");
    user2.setAge(30);
    handle->insert(user2);

    auto results = handle->query<User>().toBeans();

    EXPECT_EQ(results.size(), 2);
}

TEST_F(TestSqliteHandle, query_WithWhereClause)
{
    // Test query with WHERE clause
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user1;
    user1.setUsername("Alice");
    user1.setAge(25);
    handle->insert(user1);

    User user2;
    user2.setUsername("Bob");
    user2.setAge(35);
    handle->insert(user2);

    auto result = handle->query<User>()
                          .where(Expression::Field<User>("username") == QString("Alice"))
                          .toBean();

    ASSERT_TRUE(result != nullptr);
    EXPECT_EQ(result->username(), QString("Alice"));
    EXPECT_EQ(result->age(), 25);
}

TEST_F(TestSqliteHandle, query_WithComplexWhere)
{
    // Test query with complex WHERE clause
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    for (int i = 1; i <= 5; ++i) {
        User user;
        user.setUsername(QString("User%1").arg(i));
        user.setAge(20 + i * 5);
        user.setScore(50.0 + i * 10);
        handle->insert(user);
    }

    auto ageField = Expression::Field<User>("age");
    auto scoreField = Expression::Field<User>("score");

    auto results = handle->query<User>()
                           .where((ageField >= 25) && (scoreField > 60))
                           .toBeans();

    EXPECT_GT(results.size(), 0);

    for (const auto &user : results) {
        EXPECT_GE(user->age(), 25);
        EXPECT_GT(user->score(), 60.0);
    }
}

TEST_F(TestSqliteHandle, query_WithOrderBy)
{
    // Test query with ORDER BY
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user1;
    user1.setUsername("Charlie");
    user1.setAge(30);
    handle->insert(user1);

    User user2;
    user2.setUsername("Alice");
    user2.setAge(25);
    handle->insert(user2);

    User user3;
    user3.setUsername("Bob");
    user3.setAge(35);
    handle->insert(user3);

    auto results = handle->query<User>()
                           .orderBy(Expression::Field<User>("age"))
                           .toBeans();

    ASSERT_EQ(results.size(), 3);
    EXPECT_EQ(results[0]->age(), 25);
    EXPECT_EQ(results[1]->age(), 30);
    EXPECT_EQ(results[2]->age(), 35);
}

TEST_F(TestSqliteHandle, query_WithLimit)
{
    // Test query with LIMIT
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    for (int i = 0; i < 10; ++i) {
        User user;
        user.setUsername(QString("User%1").arg(i));
        user.setAge(20 + i);
        handle->insert(user);
    }

    auto results = handle->query<User>().take(5).toBeans();

    EXPECT_EQ(results.size(), 5);
}

TEST_F(TestSqliteHandle, query_WithSkip)
{
    // Test query with OFFSET (skip)
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    for (int i = 0; i < 10; ++i) {
        User user;
        user.setUsername(QString("User%1").arg(i));
        user.setAge(20 + i);
        handle->insert(user);
    }

    auto results = handle->query<User>()
                           .orderBy(Expression::Field<User>("id"))
                           .skip(5)
                           .toBeans();

    EXPECT_EQ(results.size(), 5);
}

// ========== Update Tests ==========

TEST_F(TestSqliteHandle, update_SingleField)
{
    // Test updating a single field
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user;
    user.setUsername("OldName");
    user.setAge(25);
    int userId = handle->insert(user);

    auto ageField = Expression::Field<User>("age");
    auto setExpr = ageField = 30;
    auto whereExpr = Expression::Field<User>("id") == userId;

    bool result = handle->update<User>(setExpr, whereExpr);

    EXPECT_TRUE(result);

    // Verify update
    auto updated = handle->query<User>()
                           .where(Expression::Field<User>("id") == userId)
                           .toBean();

    ASSERT_TRUE(updated != nullptr);
    EXPECT_EQ(updated->age(), 30);
}

TEST_F(TestSqliteHandle, update_MultipleFields)
{
    // Test updating multiple fields
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user;
    user.setUsername("Before");
    user.setAge(20);
    user.setScore(50.0);
    int userId = handle->insert(user);

    auto usernameField = Expression::Field<User>("username");
    auto ageField = Expression::Field<User>("age");

    auto setExpr = (usernameField = QString("After")) && (ageField = 25);
    auto whereExpr = Expression::Field<User>("id") == userId;

    bool result = handle->update<User>(setExpr, whereExpr);

    EXPECT_TRUE(result);
}

// ========== Delete Tests ==========

TEST_F(TestSqliteHandle, remove_ByEntity)
{
    // Test removing by entity
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user;
    user.setUsername("ToDelete");
    user.setAge(25);
    int userId = handle->insert(user);

    // Set the ID for removal
    user.setId(userId);

    bool result = handle->remove(user);

    EXPECT_TRUE(result);

    // Verify deletion
    auto found = handle->query<User>()
                         .where(Expression::Field<User>("id") == userId)
                         .toBean();

    EXPECT_TRUE(found == nullptr);
}

TEST_F(TestSqliteHandle, remove_ByExpression)
{
    // Test removing by expression
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user1;
    user1.setUsername("Keep");
    user1.setAge(20);
    handle->insert(user1);

    User user2;
    user2.setUsername("Delete");
    user2.setAge(30);
    handle->insert(user2);

    auto whereExpr = Expression::Field<User>("username") == QString("Delete");

    bool result = handle->remove<User>(whereExpr);

    EXPECT_TRUE(result);

    // Verify only one record remains
    auto results = handle->query<User>().toBeans();
    EXPECT_EQ(results.size(), 1);
    EXPECT_EQ(results[0]->username(), QString("Keep"));
}

TEST_F(TestSqliteHandle, remove_MultipleRecords)
{
    // Test removing multiple records
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    for (int i = 0; i < 5; ++i) {
        User user;
        user.setUsername(QString("User%1").arg(i));
        user.setAge(20 + i * 10);
        handle->insert(user);
    }

    auto whereExpr = Expression::Field<User>("age") >= 30;

    bool result = handle->remove<User>(whereExpr);

    EXPECT_TRUE(result);

    auto remaining = handle->query<User>().toBeans();
    EXPECT_LT(remaining.size(), 5);

    for (const auto &user : remaining) {
        EXPECT_LT(user->age(), 30);
    }
}

// ========== Transaction Tests ==========

TEST_F(TestSqliteHandle, transaction_Commit)
{
    // Test transaction commit
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    bool success = handle->transaction([this]() -> bool {
        User user1;
        user1.setUsername("TxUser1");
        user1.setAge(25);

        User user2;
        user2.setUsername("TxUser2");
        user2.setAge(30);

        return handle->insert(user1) > 0 && handle->insert(user2) > 0;
    });

    EXPECT_TRUE(success);

    auto results = handle->query<User>().toBeans();
    EXPECT_EQ(results.size(), 2);
}

TEST_F(TestSqliteHandle, transaction_Rollback)
{
    // Test transaction rollback
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    bool success = handle->transaction([this]() -> bool {
        User user;
        user.setUsername("RollbackUser");
        user.setAge(25);

        handle->insert(user);

        // Return false to trigger rollback
        return false;
    });

    EXPECT_FALSE(success);

    // Verify no data was inserted
    auto results = handle->query<User>().toBeans();
    EXPECT_EQ(results.size(), 0);
}

// ========== Utility Tests ==========

TEST_F(TestSqliteHandle, lastQuery_ReturnsExecutedSQL)
{
    // Test that lastQuery returns the last executed SQL
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    User user;
    user.setUsername("Test");
    user.setAge(25);
    handle->insert(user);

    QString lastSql = handle->lastQuery();

    EXPECT_FALSE(lastSql.isEmpty());
    EXPECT_TRUE(lastSql.contains("INSERT"));
}

TEST_F(TestSqliteHandle, excute_CustomSQL)
{
    // Test executing custom SQL
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    bool result = handle->excute("INSERT INTO users (username, age, score) VALUES ('Direct', 30, 85.5);");

    EXPECT_TRUE(result);

    auto user = handle->query<User>()
                        .where(Expression::Field<User>("username") == QString("Direct"))
                        .toBean();

    ASSERT_TRUE(user != nullptr);
    EXPECT_EQ(user->age(), 30);
}

// ========== Integration Tests ==========

TEST_F(TestSqliteHandle, Integration_CompleteWorkflow)
{
    // Test complete CRUD workflow
    // Create
    handle->createTable<User>(
            SqliteConstraint::primary("id"),
            SqliteConstraint::autoIncreament("id"));

    // Insert
    User user;
    user.setUsername("Integration");
    user.setAge(25);
    user.setScore(80.0);
    int userId = handle->insert(user);
    EXPECT_GT(userId, 0);

    // Query
    auto queried = handle->query<User>()
                           .where(Expression::Field<User>("id") == userId)
                           .toBean();
    ASSERT_TRUE(queried != nullptr);
    EXPECT_EQ(queried->username(), QString("Integration"));

    // Update
    auto ageField = Expression::Field<User>("age");
    handle->update<User>(ageField = 30, Expression::Field<User>("id") == userId);

    auto updated = handle->query<User>()
                           .where(Expression::Field<User>("id") == userId)
                           .toBean();
    ASSERT_TRUE(updated != nullptr);
    EXPECT_EQ(updated->age(), 30);

    // Delete
    handle->remove<User>(Expression::Field<User>("id") == userId);

    auto deleted = handle->query<User>()
                           .where(Expression::Field<User>("id") == userId)
                           .toBean();
    EXPECT_TRUE(deleted == nullptr);
}

#include "test_sqlitehandle.moc"

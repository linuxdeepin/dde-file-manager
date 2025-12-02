// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QObject>
#include <QVariant>
#include <QTemporaryFile>

#include "stubext.h"

#include <dfm-base/base/db/sqlitehelper.h>
#include <dfm-base/base/db/sqliteconnectionpool.h>

DFMBASE_USE_NAMESPACE

// Test entity class
class TestEntity : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "test_table")

    Q_PROPERTY(int id READ id WRITE setId)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(int age READ age WRITE setAge)
    Q_PROPERTY(double score READ score WRITE setScore)

public:
    explicit TestEntity(QObject *parent = nullptr) : QObject(parent) {}

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    int age() const { return m_age; }
    void setAge(int age) { m_age = age; }

    double score() const { return m_score; }
    void setScore(double score) { m_score = score; }

private:
    int m_id { 0 };
    QString m_name;
    int m_age { 0 };
    double m_score { 0.0 };
};

class TestSqliteHelper : public testing::Test
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

// ========== SerializationHelper Tests ==========

TEST_F(TestSqliteHelper, SerializationHelper_serialize_Integer)
{
    // Test serializing integer
    QString output;
    int value = 42;

    bool result = SerializationHelper::serialize(&output, value);

    EXPECT_TRUE(result);
    EXPECT_EQ(output, QString("42"));
}

TEST_F(TestSqliteHelper, SerializationHelper_serialize_String)
{
    // Test serializing string
    QString output;
    QString value = "test_string";

    bool result = SerializationHelper::serialize(&output, value);

    EXPECT_TRUE(result);
    EXPECT_EQ(output, QString("'test_string'"));
}

TEST_F(TestSqliteHelper, SerializationHelper_serialize_Double)
{
    // Test serializing double
    QString output;
    double value = 3.14;

    bool result = SerializationHelper::serialize(&output, value);

    EXPECT_TRUE(result);
    EXPECT_FALSE(output.isEmpty());
}

TEST_F(TestSqliteHelper, SerializationHelper_serialize_Bool)
{
    // Test serializing boolean
    QString output;
    bool value = true;

    bool result = SerializationHelper::serialize(&output, value);

    EXPECT_TRUE(result);
    EXPECT_EQ(output, QString("1"));
}

TEST_F(TestSqliteHelper, SerializationHelper_deserialize_ValidMap)
{
    // Test deserializing from variant map
    QVariantMap map;
    map["id"] = 1;
    map["name"] = "John";
    map["age"] = 25;
    map["score"] = 85.5;

    TestEntity *entity = SerializationHelper::deserialize<TestEntity>(map);

    ASSERT_TRUE(entity != nullptr);
    EXPECT_EQ(entity->id(), 1);
    EXPECT_EQ(entity->name(), QString("John"));
    EXPECT_EQ(entity->age(), 25);
    EXPECT_DOUBLE_EQ(entity->score(), 85.5);

    delete entity;
}

TEST_F(TestSqliteHelper, SerializationHelper_deserialize_EmptyMap)
{
    // Test deserializing from empty map
    QVariantMap map;

    TestEntity *entity = SerializationHelper::deserialize<TestEntity>(map);

    ASSERT_TRUE(entity != nullptr);
    delete entity;
}

// ========== Expression Tests ==========

TEST_F(TestSqliteHelper, Expression_SetExpr_SingleAssignment)
{
    // Test single assignment expression
    Expression::SetExpr expr("field1=10");

    QString result = expr.toString();

    EXPECT_EQ(result, QString("field1=10"));
}

TEST_F(TestSqliteHelper, Expression_SetExpr_MultipleAssignments)
{
    // Test multiple assignments with && operator
    Expression::SetExpr expr1("field1=10");
    Expression::SetExpr expr2("field2='value'");

    Expression::SetExpr combined = expr1 && expr2;

    QString result = combined.toString();
    EXPECT_EQ(result, QString("field1=10,field2='value'"));
}

TEST_F(TestSqliteHelper, Expression_ExprField_Assignment)
{
    // Test field assignment operator
    Expression::ExprField field("", "username");

    Expression::SetExpr setExpr = field = QVariant("John");

    QString result = setExpr.toString();
    EXPECT_TRUE(result.contains("username="));
    EXPECT_TRUE(result.contains("John"));
}

TEST_F(TestSqliteHelper, Expression_Expr_EqualOperator)
{
    // Test equal operator
    Expression::ExprField field("", "age");

    Expression::Expr expr = field == 25;

    QString result = expr.toString();
    EXPECT_EQ(result, QString("age=25"));
}

TEST_F(TestSqliteHelper, Expression_Expr_NotEqualOperator)
{
    // Test not equal operator
    Expression::ExprField field("", "status");

    Expression::Expr expr = field != 0;

    QString result = expr.toString();
    EXPECT_EQ(result, QString("status!=0"));
}

TEST_F(TestSqliteHelper, Expression_Expr_GreaterThanOperator)
{
    // Test greater than operator
    Expression::ExprField field("", "score");

    Expression::Expr expr = field > 60;

    QString result = expr.toString();
    EXPECT_EQ(result, QString("score>60"));
}

TEST_F(TestSqliteHelper, Expression_Expr_LessThanOperator)
{
    // Test less than operator
    Expression::ExprField field("", "score");

    Expression::Expr expr = field < 100;

    QString result = expr.toString();
    EXPECT_EQ(result, QString("score<100"));
}

TEST_F(TestSqliteHelper, Expression_Expr_GreaterOrEqualOperator)
{
    // Test greater or equal operator
    Expression::ExprField field("", "age");

    Expression::Expr expr = field >= 18;

    QString result = expr.toString();
    EXPECT_EQ(result, QString("age>=18"));
}

TEST_F(TestSqliteHelper, Expression_Expr_LessOrEqualOperator)
{
    // Test less or equal operator
    Expression::ExprField field("", "age");

    Expression::Expr expr = field <= 65;

    QString result = expr.toString();
    EXPECT_EQ(result, QString("age<=65"));
}

TEST_F(TestSqliteHelper, Expression_Expr_IsNull)
{
    // Test IS NULL operator
    Expression::ExprField field("", "deleted_at");

    Expression::Expr expr = field == nullptr;

    QString result = expr.toString();
    EXPECT_TRUE(result.contains("IS NULL"));
}

TEST_F(TestSqliteHelper, Expression_Expr_IsNotNull)
{
    // Test IS NOT NULL operator
    Expression::ExprField field("", "created_at");

    Expression::Expr expr = field != nullptr;

    QString result = expr.toString();
    EXPECT_TRUE(result.contains("IS NOT NULL"));
}

TEST_F(TestSqliteHelper, Expression_Expr_LikeOperator)
{
    // Test LIKE operator
    Expression::ExprField field("", "name");

    Expression::Expr expr = field & QString("%John%");

    QString result = expr.toString();
    EXPECT_TRUE(result.contains("LIKE"));
    EXPECT_TRUE(result.contains("John"));
}

TEST_F(TestSqliteHelper, Expression_Expr_NotLikeOperator)
{
    // Test NOT LIKE operator
    Expression::ExprField field("", "name");

    Expression::Expr expr = field | QString("%Admin%");

    QString result = expr.toString();
    EXPECT_TRUE(result.contains("NOT LIKE"));
    EXPECT_TRUE(result.contains("Admin"));
}

TEST_F(TestSqliteHelper, Expression_Expr_AndOperator)
{
    // Test AND operator
    Expression::ExprField field1("", "age");
    Expression::ExprField field2("", "score");

    Expression::Expr expr = (field1 >= 18) && (field2 > 60);

    QString result = expr.toString();
    EXPECT_TRUE(result.contains("AND"));
    EXPECT_TRUE(result.contains("age>=18"));
    EXPECT_TRUE(result.contains("score>60"));
}

TEST_F(TestSqliteHelper, Expression_Expr_OrOperator)
{
    // Test OR operator
    Expression::ExprField field1("", "status");
    Expression::ExprField field2("", "type");

    Expression::Expr expr = (field1 == 1) || (field2 == 2);

    QString result = expr.toString();
    EXPECT_TRUE(result.contains("OR"));
    EXPECT_TRUE(result.contains("status=1"));
    EXPECT_TRUE(result.contains("type=2"));
}

TEST_F(TestSqliteHelper, Expression_Expr_ComplexExpression)
{
    // Test complex expression with multiple operators
    Expression::ExprField age("", "age");
    Expression::ExprField score("", "score");
    Expression::ExprField name("", "name");

    Expression::Expr expr = ((age >= 18) && (score > 60)) || (name & QString("%VIP%"));

    QString result = expr.toString();
    EXPECT_FALSE(result.isEmpty());
}

TEST_F(TestSqliteHelper, Expression_Field_Helper)
{
    // Test Field helper function
    auto field = Expression::Field<TestEntity>("name");

    EXPECT_EQ(field.fieldName, QString("name"));
}

TEST_F(TestSqliteHelper, Expression_Aggregate_Count)
{
    // Test count() aggregate function
    auto agg = Expression::count();

    EXPECT_EQ(agg.fieldName, QString("COUNT (*)"));
}

TEST_F(TestSqliteHelper, Expression_Aggregate_CountField)
{
    // Test count(field) aggregate function
    auto agg = Expression::count("id");

    EXPECT_TRUE(agg.fieldName.contains("COUNT"));
    EXPECT_TRUE(agg.fieldName.contains("id"));
}

TEST_F(TestSqliteHelper, Expression_Aggregate_Sum)
{
    // Test sum() aggregate function
    auto agg = Expression::sum("price");

    EXPECT_TRUE(agg.fieldName.contains("SUM"));
    EXPECT_TRUE(agg.fieldName.contains("price"));
}

TEST_F(TestSqliteHelper, Expression_Aggregate_Avg)
{
    // Test avg() aggregate function
    auto agg = Expression::avg("score");

    EXPECT_TRUE(agg.fieldName.contains("AVG"));
    EXPECT_TRUE(agg.fieldName.contains("score"));
}

TEST_F(TestSqliteHelper, Expression_Aggregate_Max)
{
    // Test max() aggregate function
    auto agg = Expression::max("value");

    EXPECT_TRUE(agg.fieldName.contains("MAX"));
    EXPECT_TRUE(agg.fieldName.contains("value"));
}

TEST_F(TestSqliteHelper, Expression_Aggregate_Min)
{
    // Test min() aggregate function
    auto agg = Expression::min("value");

    EXPECT_TRUE(agg.fieldName.contains("MIN"));
    EXPECT_TRUE(agg.fieldName.contains("value"));
}

// ========== SqliteConstraint Tests ==========

TEST_F(TestSqliteHelper, SqliteConstraint_primary)
{
    // Test primary key constraint
    auto constraint = SqliteConstraint::primary("id");

    EXPECT_EQ(constraint.field, QString("id"));
    EXPECT_TRUE(constraint.constraint.contains("PRIMARY KEY"));
}

TEST_F(TestSqliteHelper, SqliteConstraint_autoIncreament)
{
    // Test auto increment constraint
    auto constraint = SqliteConstraint::autoIncreament("id");

    EXPECT_EQ(constraint.field, QString("id"));
    EXPECT_TRUE(constraint.constraint.contains("AUTOINCREMENT"));
}

TEST_F(TestSqliteHelper, SqliteConstraint_nullable)
{
    // Test nullable constraint
    auto constraint = SqliteConstraint::nullable("optional_field");

    EXPECT_EQ(constraint.field, QString("optional_field"));
}

TEST_F(TestSqliteHelper, SqliteConstraint_unique)
{
    // Test unique constraint
    auto constraint = SqliteConstraint::unique("email");

    EXPECT_TRUE(constraint.constraint.contains("UNIQUE"));
    EXPECT_TRUE(constraint.constraint.contains("email"));
}

TEST_F(TestSqliteHelper, SqliteConstraint_defaultValue)
{
    // Test default value constraint
    auto constraint = SqliteConstraint::defaultValue("status", 0);

    EXPECT_EQ(constraint.field, QString("status"));
    EXPECT_TRUE(constraint.constraint.contains("DEFAULT"));
}

TEST_F(TestSqliteHelper, SqliteConstraint_check)
{
    // Test check constraint
    Expression::ExprField field("", "age");
    Expression::Expr expr = field >= 0;

    auto constraint = SqliteConstraint::check(expr);

    EXPECT_TRUE(constraint.constraint.contains("CHECK"));
    EXPECT_TRUE(constraint.constraint.contains("age>=0"));
}

// ========== SqliteHelper Tests ==========

TEST_F(TestSqliteHelper, SqliteHelper_tableName)
{
    // Test table name extraction
    QString tableName = SqliteHelper::tableName<TestEntity>();

    EXPECT_EQ(tableName, QString("test_table"));
}

TEST_F(TestSqliteHelper, SqliteHelper_fieldNames)
{
    // Test field names extraction
    QStringList fields = SqliteHelper::fieldNames<TestEntity>();

    EXPECT_FALSE(fields.isEmpty());
    EXPECT_TRUE(fields.contains("id"));
    EXPECT_TRUE(fields.contains("name"));
    EXPECT_TRUE(fields.contains("age"));
    EXPECT_TRUE(fields.contains("score"));
    EXPECT_FALSE(fields.contains("objectName"));  // Should be removed
}

TEST_F(TestSqliteHelper, SqliteHelper_fieldTypesMap)
{
    // Test field types mapping
    QStringList fields = SqliteHelper::fieldNames<TestEntity>();
    QHash<QString, QString> typeMap;

    SqliteHelper::fieldTypesMap<TestEntity>(fields, &typeMap);

    EXPECT_FALSE(typeMap.isEmpty());
    EXPECT_TRUE(typeMap.contains("id"));
    EXPECT_TRUE(typeMap.contains("name"));
    EXPECT_TRUE(typeMap["id"].contains("INTEGER"));
    EXPECT_TRUE(typeMap["name"].contains("TEXT"));
}

TEST_F(TestSqliteHelper, SqliteHelper_typeString_Integer)
{
    // Test type string for integer
    QString typeStr = SqliteHelper::typeString(QVariant::Type::Int);

    EXPECT_TRUE(typeStr.contains("INTEGER"));
    EXPECT_TRUE(typeStr.contains("NOT NULL"));
}

TEST_F(TestSqliteHelper, SqliteHelper_typeString_Double)
{
    // Test type string for double
    QString typeStr = SqliteHelper::typeString(QVariant::Type::Double);

    EXPECT_TRUE(typeStr.contains("REAL"));
    EXPECT_TRUE(typeStr.contains("NOT NULL"));
}

TEST_F(TestSqliteHelper, SqliteHelper_typeString_String)
{
    // Test type string for string
    QString typeStr = SqliteHelper::typeString(QVariant::Type::String);

    EXPECT_TRUE(typeStr.contains("TEXT"));
    EXPECT_TRUE(typeStr.contains("NOT NULL"));
}

TEST_F(TestSqliteHelper, SqliteHelper_typeString_Bool)
{
    // Test type string for boolean
    QString typeStr = SqliteHelper::typeString(QVariant::Type::Bool);

    EXPECT_TRUE(typeStr.contains("INTEGER"));
}

TEST_F(TestSqliteHelper, SqliteHelper_visit)
{
    // Test visit function
    int propertyCount = 0;

    SqliteHelper::visit<TestEntity>([&propertyCount](const QMetaProperty &property) {
        if (property.isValid())
            propertyCount++;
    });

    EXPECT_GT(propertyCount, 0);
}

TEST_F(TestSqliteHelper, SqliteHelper_parseConstraint_EmptyArgs)
{
    // Test parseConstraint with no arguments
    QString tableFixes;
    QHash<QString, QString> fieldFixes;
    fieldFixes["id"] = " INTEGER NOT NULL";

    SqliteHelper::parseConstraint(&tableFixes, &fieldFixes);

    EXPECT_TRUE(tableFixes.isEmpty());
}

TEST_F(TestSqliteHelper, SqliteHelper_parseConstraint_SingleConstraint)
{
    // Test parseConstraint with single constraint
    QString tableFixes;
    QHash<QString, QString> fieldFixes;
    fieldFixes["id"] = " INTEGER NOT NULL";

    SqliteHelper::parseConstraint(&tableFixes, &fieldFixes,
                                   SqliteConstraint::primary("id"));

    EXPECT_TRUE(fieldFixes["id"].contains("PRIMARY KEY"));
    EXPECT_FALSE(fieldFixes["id"].contains("NOT NULL"));
}

TEST_F(TestSqliteHelper, SqliteHelper_parseConstraint_MultipleConstraints)
{
    // Test parseConstraint with multiple constraints
    QString tableFixes;
    QHash<QString, QString> fieldFixes;
    fieldFixes["id"] = " INTEGER NOT NULL";
    fieldFixes["email"] = " TEXT NOT NULL";

    SqliteHelper::parseConstraint(&tableFixes, &fieldFixes,
                                   SqliteConstraint::primary("id"),
                                   SqliteConstraint::unique("email"));

    EXPECT_TRUE(fieldFixes["id"].contains("PRIMARY KEY"));
    EXPECT_TRUE(tableFixes.contains("UNIQUE"));
}

// ========== Integration Tests ==========

TEST_F(TestSqliteHelper, Integration_CreateEntity)
{
    // Test creating and populating an entity
    TestEntity entity;
    entity.setId(1);
    entity.setName("Alice");
    entity.setAge(30);
    entity.setScore(95.5);

    EXPECT_EQ(entity.id(), 1);
    EXPECT_EQ(entity.name(), QString("Alice"));
    EXPECT_EQ(entity.age(), 30);
    EXPECT_DOUBLE_EQ(entity.score(), 95.5);
}

TEST_F(TestSqliteHelper, Integration_SerializeDeserializeRoundTrip)
{
    // Test serialize-deserialize round trip
    QVariantMap originalMap;
    originalMap["id"] = 42;
    originalMap["name"] = "Test";
    originalMap["age"] = 25;
    originalMap["score"] = 88.8;

    TestEntity *entity = SerializationHelper::deserialize<TestEntity>(originalMap);
    ASSERT_TRUE(entity != nullptr);

    // Verify values
    EXPECT_EQ(entity->id(), 42);
    EXPECT_EQ(entity->name(), QString("Test"));
    EXPECT_EQ(entity->age(), 25);
    EXPECT_DOUBLE_EQ(entity->score(), 88.8);

    delete entity;
}

TEST_F(TestSqliteHelper, Integration_ComplexWhereClause)
{
    // Test building complex WHERE clause
    auto age = Expression::Field<TestEntity>("age");
    auto score = Expression::Field<TestEntity>("score");
    auto name = Expression::Field<TestEntity>("name");

    auto whereClause = ((age >= 18) && (score > 60)) || (name & QString("%VIP%"));

    QString sql = whereClause.toString();
    EXPECT_FALSE(sql.isEmpty());
    EXPECT_TRUE(sql.contains("age>=18"));
    EXPECT_TRUE(sql.contains("score>60"));
    EXPECT_TRUE(sql.contains("VIP"));
}

#include "test_sqlitehelper.moc"

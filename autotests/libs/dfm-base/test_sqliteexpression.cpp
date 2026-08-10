// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sqliteexpression.cpp
 * @brief Unit tests for the pure-logic query-building helpers in
 *        src/dfm-base/base/db/sqlitehelper.h.
 *
 * Covers:
 *   - SerializationHelper::serialize / deserialize
 *   - Expression::SetExpr / ExprField / Aggregate / Expr and all operators
 *   - Expression::Field<T>() and aggregate factories (count/sum/avg/max/min)
 *   - SqliteConstraint static factories
 *   - SqliteHelper::typeString (both overloads)
 *
 * These helpers build SQL fragments from in-memory values; none of them touch
 * the database, so the tests are fully deterministic and require no SQLite
 * connection. The compile target enables -fno-access-control, which lets the
 * tests read the protected members of SqliteConstraint to assert on the
 * generated constraint strings.
 */

#include <gtest/gtest.h>

#include <dfm-base/base/db/sqlitehelper.h>

#include <QList>
#include <QMetaProperty>
#include <QObject>
#include <QString>
#include <QVariant>

using namespace dfmbase;
using namespace dfmbase::Expression;

// ---------------------------------------------------------------------------
// SerializationHelper
// ---------------------------------------------------------------------------

TEST(SerializationHelperTest, SerializeStringWrapsValueInSingleQuotes)
{
    QString out;
    EXPECT_TRUE(SerializationHelper::serialize(&out, QString("bob")));
    EXPECT_EQ(out.toStdString(), "'bob'");
}

TEST(SerializationHelperTest, SerializeIntReturnsPlainDecimal)
{
    QString out;
    EXPECT_TRUE(SerializationHelper::serialize(&out, 42));
    EXPECT_EQ(out.toStdString(), "42");
}

TEST(SerializationHelperTest, SerializeNonConvertibleReturnsFalse)
{
    // QList<int> cannot be converted to QString (verified), so serialize must
    // bail out with false and leave the output untouched.
    QString out = "untouched";
    QVariant bad = QVariant::fromValue(QList<int> { 1, 2, 3 });
    EXPECT_FALSE(SerializationHelper::serialize(&out, bad));
    EXPECT_EQ(out.toStdString(), "untouched");
}

TEST(SerializationHelperTest, DeserializePopulatesPropertiesOnQObject)
{
    // QObject exposes setProperty() for dynamic properties, so a plain QObject
    // can serve as the deserialization target without a custom Q_OBJECT bean.
    QVariantMap map;
    map.insert("foo", 123);
    map.insert("bar", QString("hello"));
    QObject *bean = SerializationHelper::deserialize<QObject>(map);
    ASSERT_NE(bean, nullptr);
    EXPECT_EQ(bean->property("foo").toInt(), 123);
    EXPECT_EQ(bean->property("bar").toString().toStdString(), "hello");
    delete bean;
}

TEST(SerializationHelperTest, DeserializeEmptyMapYieldsBlankBean)
{
    QObject *bean = SerializationHelper::deserialize<QObject>(QVariantMap {});
    ASSERT_NE(bean, nullptr);
    delete bean;
}

// ---------------------------------------------------------------------------
// Expression::SetExpr
// ---------------------------------------------------------------------------

TEST(SetExprTest, ToStringReturnsRawExpression)
{
    EXPECT_EQ(SetExpr("a=1").toString().toStdString(), "a=1");
}

TEST(SetExprTest, AndOperatorConcatenatesWithComma)
{
    SetExpr combined = SetExpr("a=1") && SetExpr("b=2");
    EXPECT_EQ(combined.toString().toStdString(), "a=1,b=2");
}

// ---------------------------------------------------------------------------
// Expression::ExprField
// ---------------------------------------------------------------------------

TEST(ExprFieldTest, ConstructorStoresTableNameAndFieldName)
{
    ExprField f("tbl", "col");
    EXPECT_EQ(f.tableName.toStdString(), "tbl");
    EXPECT_EQ(f.fieldName.toStdString(), "col");
}

TEST(ExprFieldTest, AssignIntValueProducesFieldEqualsDecimal)
{
    ExprField f("tbl", "col");
    SetExpr se = f = QVariant(5);
    EXPECT_EQ(se.toString().toStdString(), "col=5");
}

TEST(ExprFieldTest, AssignStringValueProducesFieldEqualsQuoted)
{
    ExprField f("tbl", "col");
    SetExpr se = f = QVariant(QString("x"));
    EXPECT_EQ(se.toString().toStdString(), "col='x'");
}

// ---------------------------------------------------------------------------
// Expression::Aggregate
// ---------------------------------------------------------------------------

TEST(AggregateTest, FunctionOnlyConstructorStoresFunctionAsField)
{
    Aggregate a("COUNT");
    EXPECT_EQ(a.fieldName.toStdString(), "COUNT");
}

TEST(AggregateTest, FunctionWithFieldConstructorStoresFunctionalCall)
{
    Aggregate a("MAX", "salary");
    EXPECT_EQ(a.fieldName.toStdString(), "MAX(salary)");
}

// ---------------------------------------------------------------------------
// Expression::Expr
// ---------------------------------------------------------------------------

TEST(ExprTest, FieldNameAndOperatorConstructor)
{
    EXPECT_EQ(Expr("name", "=").toString().toStdString(), "name=");
}

TEST(ExprTest, ExprFieldAndOperatorConstructorUsesFieldName)
{
    ExprField f("tbl", "age");
    EXPECT_EQ(Expr(f, ">").toString().toStdString(), "age>");
}

TEST(ExprTest, FieldNameOperatorAndStringVariantValueSerializesQuoted)
{
    Expr e("name", "=", QVariant(QString("bob")));
    EXPECT_EQ(e.toString().toStdString(), "name='bob'");
}

TEST(ExprTest, FieldNameOperatorAndIntVariantValueSerializesPlain)
{
    Expr e("age", ">", QVariant(30));
    EXPECT_EQ(e.toString().toStdString(), "age>30");
}

TEST(ExprTest, ExprFieldOperatorAndVariantValueUsesFieldName)
{
    ExprField f("tbl", "age");
    Expr e(f, ">", QVariant(30));
    EXPECT_EQ(e.toString().toStdString(), "age>30");
}

TEST(ExprTest, AndOperatorWrapsWithAndKeyword)
{
    Expr left("name", "=", QVariant(QString("bob")));
    Expr right("age", ">");
    Expr combined = left && right;
    EXPECT_EQ(combined.toString().toStdString(), "(name='bob' AND age>)");
}

TEST(ExprTest, OrOperatorWrapsWithOrKeyword)
{
    Expr left("name", "=", QVariant(QString("bob")));
    Expr right("age", ">");
    Expr combined = left || right;
    EXPECT_EQ(combined.toString().toStdString(), "(name='bob' OR age>)");
}

// ---------------------------------------------------------------------------
// Expression comparison operators (ExprField, QVariant)
// ---------------------------------------------------------------------------

TEST(ExpressionOperatorsTest, EqualityOperatorProducesEqualsComparison)
{
    ExprField col("t", "c");
    EXPECT_EQ((col == QVariant(1)).toString().toStdString(), "c=1");
}

TEST(ExpressionOperatorsTest, InequalityOperatorProducesNotEqualsComparison)
{
    ExprField col("t", "c");
    EXPECT_EQ((col != QVariant(1)).toString().toStdString(), "c!=1");
}

TEST(ExpressionOperatorsTest, GreaterThanOperatorProducesGtComparison)
{
    ExprField col("t", "c");
    EXPECT_EQ((col > QVariant(1)).toString().toStdString(), "c>1");
}

TEST(ExpressionOperatorsTest, LessThanOperatorProducesLtComparison)
{
    ExprField col("t", "c");
    EXPECT_EQ((col < QVariant(1)).toString().toStdString(), "c<1");
}

TEST(ExpressionOperatorsTest, GreaterEqualOperatorProducesGeComparison)
{
    ExprField col("t", "c");
    EXPECT_EQ((col >= QVariant(1)).toString().toStdString(), "c>=1");
}

TEST(ExpressionOperatorsTest, LessEqualOperatorProducesLeComparison)
{
    ExprField col("t", "c");
    EXPECT_EQ((col <= QVariant(1)).toString().toStdString(), "c<=1");
}

TEST(ExpressionOperatorsTest, EqualityWithNullProducesIsNull)
{
    ExprField col("t", "c");
    EXPECT_EQ((col == nullptr).toString().toStdString(), "c IS NULL");
}

TEST(ExpressionOperatorsTest, InequalityWithNullProducesIsNotNull)
{
    ExprField col("t", "c");
    EXPECT_EQ((col != nullptr).toString().toStdString(), "c IS NOT NULL");
}

TEST(ExpressionOperatorsTest, BitwiseAndOperatorProducesLike)
{
    ExprField col("t", "c");
    EXPECT_EQ((col & QString("a%")).toString().toStdString(), "c LIKE 'a%'");
}

TEST(ExpressionOperatorsTest, BitwiseOrOperatorProducesNotLike)
{
    ExprField col("t", "c");
    EXPECT_EQ((col | QString("a%")).toString().toStdString(), "c NOT LIKE 'a%'");
}

// ---------------------------------------------------------------------------
// Expression::Field<T>() and aggregate factories
// ---------------------------------------------------------------------------

TEST(ExpressionFactoryTest, FieldReturnsExprFieldWithBlankTable)
{
    ExprField f = Expression::Field<QObject>("colname");
    EXPECT_EQ(f.tableName.toStdString(), "");
    EXPECT_EQ(f.fieldName.toStdString(), "colname");
}

TEST(ExpressionFactoryTest, CountWithoutArgumentProducesCountStar)
{
    EXPECT_EQ(Expression::count().fieldName.toStdString(), "COUNT (*)");
}

TEST(ExpressionFactoryTest, CountWithArgumentProducesCountField)
{
    EXPECT_EQ(Expression::count("c").fieldName.toStdString(), "COUNT(c)");
}

TEST(ExpressionFactoryTest, SumFactoryProducesSumAggregate)
{
    EXPECT_EQ(Expression::sum("c").fieldName.toStdString(), "SUM(c)");
}

TEST(ExpressionFactoryTest, AvgFactoryProducesAvgAggregate)
{
    EXPECT_EQ(Expression::avg("c").fieldName.toStdString(), "AVG(c)");
}

TEST(ExpressionFactoryTest, MaxFactoryProducesMaxAggregate)
{
    EXPECT_EQ(Expression::max("c").fieldName.toStdString(), "MAX(c)");
}

TEST(ExpressionFactoryTest, MinFactoryProducesMinAggregate)
{
    EXPECT_EQ(Expression::min("c").fieldName.toStdString(), "MIN(c)");
}

// ---------------------------------------------------------------------------
// SqliteConstraint (protected members read via -fno-access-control)
// ---------------------------------------------------------------------------

TEST(SqliteConstraintTest, PrimaryFactoryProducesPrimaryKeyConstraint)
{
    auto c = SqliteConstraint::primary("id");
    EXPECT_EQ(c.field.toStdString(), "id");
    EXPECT_EQ(c.constraint.toStdString(), " PRIMARY KEY");
}

TEST(SqliteConstraintTest, AutoIncreamentFactoryProducesAutoincrementConstraint)
{
    auto c = SqliteConstraint::autoIncreament("id");
    EXPECT_EQ(c.field.toStdString(), "id");
    EXPECT_EQ(c.constraint.toStdString(), " AUTOINCREMENT");
}

TEST(SqliteConstraintTest, NullableFactoryProducesNullableConstraint)
{
    auto c = SqliteConstraint::nullable("id");
    EXPECT_EQ(c.field.toStdString(), "id");
    EXPECT_EQ(c.constraint.toStdString(), "NULLABLE");
}

TEST(SqliteConstraintTest, UniqueFactoryProducesUniqueConstraint)
{
    auto c = SqliteConstraint::unique("id");
    EXPECT_TRUE(c.field.isEmpty());
    EXPECT_EQ(c.constraint.toStdString(), "UNIQUE (id)");
}

TEST(SqliteConstraintTest, DefaultValueWithIntProducesDefaultConstraint)
{
    auto c = SqliteConstraint::defaultValue("id", 0);
    EXPECT_EQ(c.field.toStdString(), "id");
    EXPECT_EQ(c.constraint.toStdString(), " DEFAULT 0");
}

TEST(SqliteConstraintTest, DefaultValueWithStringProducesQuotedDefaultConstraint)
{
    auto c = SqliteConstraint::defaultValue("name", QString("x"));
    EXPECT_EQ(c.field.toStdString(), "name");
    EXPECT_EQ(c.constraint.toStdString(), " DEFAULT 'x'");
}

TEST(SqliteConstraintTest, CheckFactoryProducesCheckConstraint)
{
    auto c = SqliteConstraint::check(Expr("age", ">", QVariant(0)));
    EXPECT_TRUE(c.field.isEmpty());
    EXPECT_EQ(c.constraint.toStdString(), "CHECK (age>0)");
}

// ---------------------------------------------------------------------------
// SqliteHelper::typeString
// ---------------------------------------------------------------------------

TEST(SqliteHelperTypeStringTest, VariantTypeIntMapsToIntegerNotNull)
{
    EXPECT_EQ(SqliteHelper::typeString(QVariant::Int).toStdString(), " INTEGER NOT NULL");
}

TEST(SqliteHelperTypeStringTest, VariantTypeBoolMapsToIntegerNotNull)
{
    EXPECT_EQ(SqliteHelper::typeString(QVariant::Bool).toStdString(), " INTEGER NOT NULL");
}

TEST(SqliteHelperTypeStringTest, VariantTypeDoubleMapsToRealNotNull)
{
    EXPECT_EQ(SqliteHelper::typeString(QVariant::Double).toStdString(), " REAL NOT NULL");
}

TEST(SqliteHelperTypeStringTest, VariantTypeStringMapsToTextNotNull)
{
    EXPECT_EQ(SqliteHelper::typeString(QVariant::String).toStdString(), " TEXT NOT NULL");
}

TEST(SqliteHelperTypeStringTest, InvalidMetaPropertyReturnsEmpty)
{
    QMetaProperty invalid;
    EXPECT_TRUE(SqliteHelper::typeString(invalid).isEmpty());
}

TEST(SqliteHelperTypeStringTest, ObjectNameMetaPropertyMapsToTextNotNull)
{
    // QObject::objectName is a valid String-typed meta-property.
    QMetaProperty objectName = QObject::staticMetaObject.property(0);
    ASSERT_FALSE(objectName.name() == nullptr);
    EXPECT_EQ(std::string(objectName.name()), "objectName");
    EXPECT_EQ(SqliteHelper::typeString(objectName).toStdString(), " TEXT NOT NULL");
}

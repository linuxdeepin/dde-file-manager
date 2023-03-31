// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "ut_testobj_user.h"
#include <dfm-base/base/db/sqlitequeryable.h>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace TestObj;

class UT_SqliteQueryable : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

public:
    stub_ext::StubExt stub;
};

TEST_F(UT_SqliteQueryable, constructor)
{
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };

    EXPECT_EQ(querable.databaseName, "dbname");
    EXPECT_EQ(querable.sqlFrom, " FROM User");
    EXPECT_EQ(querable.sqlSelect, "SELECT ");
    EXPECT_EQ(querable.sqlTarget, "*");

    EXPECT_TRUE(querable.sqlWhere.isEmpty());
    EXPECT_TRUE(querable.sqlGroupBy.isEmpty());
    EXPECT_TRUE(querable.sqlHaving.isEmpty());

    EXPECT_TRUE(querable.sqlOrderBy.isEmpty());
    EXPECT_TRUE(querable.sqlLimit.isEmpty());
    EXPECT_TRUE(querable.sqlOffset.isEmpty());
}

TEST_F(UT_SqliteQueryable, disctinct)
{
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    querable.disctinct();
    EXPECT_EQ(querable.sqlSelect, "SELECT DISTINCT ");
}

TEST_F(UT_SqliteQueryable, where)
{
    auto field = Expression::Field<User>;
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    Expression::Expr expr(field("weight") == nullptr);
    querable.where(expr);
    QString sqlWhere { " WHERE " + expr.toString() };
    EXPECT_EQ(sqlWhere, querable.sqlWhere);
}

TEST_F(UT_SqliteQueryable, groupBy)
{
    auto field = Expression::Field<User>;
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    auto exprField = field("name");
    querable.groupBy(exprField);
    QString sqlGroupBy { " GROUP BY " + exprField.fieldName };
    EXPECT_EQ(sqlGroupBy, querable.sqlGroupBy);
}

TEST_F(UT_SqliteQueryable, having)
{
    auto field = Expression::Field<User>;
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    Expression::Expr expr(field("weight") == nullptr);
    querable.having(expr);
    QString sqlHaving { " HAVING " + expr.toString() };
    EXPECT_EQ(sqlHaving, querable.sqlHaving);
}

TEST_F(UT_SqliteQueryable, take)
{
    int count { 3 };
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    querable.take(count);
    QString sqlLimit { " LIMIT " + QString::number(count) };
    EXPECT_EQ(sqlLimit, querable.sqlLimit);
}

TEST_F(UT_SqliteQueryable, skip)
{
    int count { 0 };
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    querable.skip(count);
    QString sqlLimit { " LIMIT ~0" };
    EXPECT_EQ(sqlLimit, querable.sqlLimit);

    count = 3;
    querable.skip(count);
    QString sqlOffset { " OFFSET " + QString::number(count) };
    EXPECT_EQ(sqlOffset, querable.sqlOffset);
}

TEST_F(UT_SqliteQueryable, orderBy)
{
    auto field = Expression::Field<User>;
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    auto exprField = field("name");
    querable.orderBy(exprField);
    QString sqlOrderBy { " ORDER BY " + exprField.fieldName };
    EXPECT_EQ(sqlOrderBy, querable.sqlOrderBy);

    exprField = field("height");
    sqlOrderBy += "," + exprField.fieldName;
    querable.orderBy(exprField);
    EXPECT_EQ(sqlOrderBy, querable.sqlOrderBy);
}

TEST_F(UT_SqliteQueryable, orderByDescending)
{
    auto field = Expression::Field<User>;
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    auto exprField = field("name");
    querable.orderByDescending(exprField);
    QString sqlOrderBy { " ORDER BY " + exprField.fieldName + " DESC" };
    EXPECT_EQ(sqlOrderBy, querable.sqlOrderBy);

    exprField = field("height");
    sqlOrderBy += "," + exprField.fieldName + " DESC";
    querable.orderByDescending(exprField);
    EXPECT_EQ(sqlOrderBy, querable.sqlOrderBy);
}

TEST_F(UT_SqliteQueryable, toBean)
{
}

TEST_F(UT_SqliteQueryable, toBeans)
{
}

TEST_F(UT_SqliteQueryable, toMap)
{
}

TEST_F(UT_SqliteQueryable, toMaps)
{
}

TEST_F(UT_SqliteQueryable, aggregate)
{
    stub.set_lamda(ADDR(SqliteHelper, excute), []() {
        return true;
    });
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    auto val { querable.aggregate(Expression::count()) };
    EXPECT_TRUE(val.toString().isEmpty());
}

TEST_F(UT_SqliteQueryable, getFromSql)
{
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    EXPECT_EQ(querable.getFromSql(), " FROM User");
}

TEST_F(UT_SqliteQueryable, getLimit)
{
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    EXPECT_TRUE(querable.getLimit().isEmpty());
}

TEST_F(UT_SqliteQueryable, queryToMaps)
{
    int count { 0 };
    stub.set_lamda(ADDR(QSqlQuery, next), [&count]() {
        if (count >= 5)
            return false;
        ++count;
        return true;
    });
    auto valueFunc = static_cast<QVariant (QSqlQuery::*)(const QString &) const>(&QSqlQuery::value);
    stub.set_lamda(valueFunc, [](QSqlQuery *, const QString &name) {
        return name.toUpper();
    });

    QSqlQuery query;
    SqliteQueryable<User> querable { "dbname", " FROM " + SqliteHelper::tableName<User>() };
    QList<QVariantMap> maps { querable.queryToMaps(&query) };

    EXPECT_TRUE(!maps.isEmpty());
    EXPECT_TRUE(maps.size() == 5);
    auto map { maps.value(0) };
    for (const QString &key : map.keys())
        EXPECT_EQ(key.toUpper(), map.value(key));
}

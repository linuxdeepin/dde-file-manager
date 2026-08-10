// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_sqlitequeryable.cpp
 * @brief Unit tests for the SQLite data-access layer in
 *        src/dfm-base/base/db/{sqlitequeryable,sqlitehandle,sqlitehelper}.h.
 *
 * Covers:
 *   - SqliteQueryable<T> query-builder chain (distinct/where/groupBy/having/
 *     take/skip/orderBy/orderByDescending) and execution (toMaps/toMap/
 *     toBeans/toBean/aggregate) plus private helpers getFromSql/getLimit/
 *     queryToMaps.
 *   - SqliteHelper statics: tableName/visit/fieldNames/fieldTypesMap/
 *     parseConstraint/excute.
 *   - SqliteHandle CRUD: transaction/createTable/dropTable/insert/update/
 *     query/remove (both overloads)/excute/lastQuery.
 *
 * The build target enables -fno-access-control, so the builder tests can read
 * the private SQL-fragment members of SqliteQueryable directly. A tiny
 * Q_OBJECT bean (UtQueryableBean) is defined in this translation unit and
 * moc'd via the trailing #include of the generated moc file.
 */

#include <gtest/gtest.h>

#include <dfm-base/base/db/sqliteconnectionpool.h>
#include <dfm-base/base/db/sqlitehandle.h>
#include <dfm-base/base/db/sqlitehelper.h>
#include <dfm-base/base/db/sqlitequeryable.h>

#include <QHash>
#include <QList>
#include <QMetaProperty>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QVariant>
#include <QVariantMap>

#include "ut_sqlitebean.h"

using namespace dfmbase;
using namespace dfmbase::Expression;

// ---------------------------------------------------------------------------
// Fixture: a fresh on-disk database per test.
// ---------------------------------------------------------------------------

class SqliteQueryableTest : public testing::Test
{
protected:
    void SetUp() override
    {
        ASSERT_TRUE(tmpDir.isValid());
        dbPath = tmpDir.path() + "/ut_queryable.db";
    }
    QTemporaryDir tmpDir;
    QString dbPath;
};

// ---------------------------------------------------------------------------
// SqliteQueryable<T> builder chain (private members read via -fno-access-control)
// ---------------------------------------------------------------------------

TEST_F(SqliteQueryableTest, DistinctBuilderSetsSelectDistinct)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.disctinct();
    EXPECT_EQ(q.sqlSelect.toStdString(), "SELECT DISTINCT ");
}

TEST_F(SqliteQueryableTest, WhereBuilderSerializesWhereExpression)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    ExprField col = Field<UtQueryableBean>("name");
    q.where(col == QVariant(QString("alice")));
    EXPECT_EQ(q.sqlWhere.toStdString(), " WHERE name='alice'");
}

TEST_F(SqliteQueryableTest, GroupByBuilderAppendsGroupByClause)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.groupBy(Field<UtQueryableBean>("name"));
    EXPECT_EQ(q.sqlGroupBy.toStdString(), " GROUP BY name");
}

TEST_F(SqliteQueryableTest, HavingBuilderAppendsHavingClause)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.having(Field<UtQueryableBean>("name") == QVariant(QString("alice")));
    EXPECT_EQ(q.sqlHaving.toStdString(), " HAVING name='alice'");
}

TEST_F(SqliteQueryableTest, TakeBuilderSetsLimit)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.take(5);
    EXPECT_EQ(q.sqlLimit.toStdString(), " LIMIT 5");
}

TEST_F(SqliteQueryableTest, SkipBuilderSetsOffsetAndInfiniteLimitWhenEmpty)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.skip(3);
    EXPECT_EQ(q.sqlLimit.toStdString(), " LIMIT ~0");
    EXPECT_EQ(q.sqlOffset.toStdString(), " OFFSET 3");
}

TEST_F(SqliteQueryableTest, SkipAfterTakePreservesTakeLimit)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.take(5).skip(2);
    EXPECT_EQ(q.sqlLimit.toStdString(), " LIMIT 5");
    EXPECT_EQ(q.sqlOffset.toStdString(), " OFFSET 2");
}

TEST_F(SqliteQueryableTest, OrderByBuilderSetsOrderByAscending)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.orderBy(Field<UtQueryableBean>("name"));
    EXPECT_EQ(q.sqlOrderBy.toStdString(), " ORDER BY name");
}

TEST_F(SqliteQueryableTest, OrderByBuilderAppendsSecondFieldWithComma)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.orderBy(Field<UtQueryableBean>("name")).orderBy(Field<UtQueryableBean>("id"));
    EXPECT_EQ(q.sqlOrderBy.toStdString(), " ORDER BY name,id");
}

TEST_F(SqliteQueryableTest, OrderByDescendingBuilderSetsOrderByDesc)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.orderByDescending(Field<UtQueryableBean>("name"));
    EXPECT_EQ(q.sqlOrderBy.toStdString(), " ORDER BY name DESC");
}

TEST_F(SqliteQueryableTest, OrderByDescendingBuilderAppendsSecondFieldDesc)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.orderByDescending(Field<UtQueryableBean>("name"))
            .orderByDescending(Field<UtQueryableBean>("id"));
    EXPECT_EQ(q.sqlOrderBy.toStdString(), " ORDER BY name DESC,id DESC");
}

TEST_F(SqliteQueryableTest, PrivateFromSqlAssemblesFromWhereGroupByHaving)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable", "SELECT ", "*", " WHERE id>0");
    EXPECT_EQ(q.getFromSql().toStdString(), " FROM ut_queryable WHERE id>0");
}

TEST_F(SqliteQueryableTest, PrivateLimitAssemblesOrderByLimitOffset)
{
    SqliteQueryable<UtQueryableBean> q(dbPath, " FROM ut_queryable");
    q.orderBy(Field<UtQueryableBean>("name")).take(10).skip(2);
    EXPECT_EQ(q.getLimit().toStdString(), " ORDER BY name LIMIT 10 OFFSET 2");
}

// ---------------------------------------------------------------------------
// SqliteHelper statics (bean-aware)
// ---------------------------------------------------------------------------

TEST_F(SqliteQueryableTest, TableNameReturnsClassInfoValue)
{
    EXPECT_EQ(SqliteHelper::tableName<UtQueryableBean>().toStdString(), "ut_queryable");
}

TEST_F(SqliteQueryableTest, FieldNamesExcludesObjectName)
{
    QStringList names = SqliteHelper::fieldNames<UtQueryableBean>();
    ASSERT_EQ(names.size(), 2);
    EXPECT_EQ(names[0].toStdString(), "id");
    EXPECT_EQ(names[1].toStdString(), "name");
}

TEST_F(SqliteQueryableTest, VisitInvokesCallbackPerProperty)
{
    int count = 0;
    SqliteHelper::visit<UtQueryableBean>([&count](const QMetaProperty &) { ++count; });
    // objectName + id + name
    EXPECT_EQ(count, 3);
}

TEST_F(SqliteQueryableTest, FieldTypesMapMapsFieldsToSqlTypes)
{
    QStringList fields = SqliteHelper::fieldNames<UtQueryableBean>();
    QHash<QString, QString> map;
    SqliteHelper::fieldTypesMap<UtQueryableBean>(fields, &map);
    EXPECT_EQ(map.size(), 2);
    EXPECT_EQ(map.value("id").toStdString(), " INTEGER NOT NULL");
    EXPECT_EQ(map.value("name").toStdString(), " TEXT NOT NULL");
}

TEST_F(SqliteQueryableTest, ParseConstraintEmptyBaseCaseIsNoop)
{
    QString tableFixes;
    QHash<QString, QString> fieldFixes;
    SqliteHelper::parseConstraint(&tableFixes, &fieldFixes);
    EXPECT_TRUE(tableFixes.isEmpty());
    EXPECT_TRUE(fieldFixes.isEmpty());
}

TEST_F(SqliteQueryableTest, ParseConstraintPrimaryKeyModifiesField)
{
    QString tableFixes;
    QHash<QString, QString> fieldFixes;
    fieldFixes.insert("id", " INTEGER NOT NULL");
    fieldFixes.insert("name", " TEXT NOT NULL");
    SqliteHelper::parseConstraint(&tableFixes, &fieldFixes, SqliteConstraint::primary("id"));
    EXPECT_EQ(fieldFixes.value("id").toStdString(), " INTEGER PRIMARY KEY");
    EXPECT_TRUE(tableFixes.isEmpty());
}

TEST_F(SqliteQueryableTest, ParseConstraintUniqueAppendsTableFix)
{
    QString tableFixes;
    QHash<QString, QString> fieldFixes;
    fieldFixes.insert("id", " INTEGER NOT NULL");
    SqliteHelper::parseConstraint(&tableFixes, &fieldFixes, SqliteConstraint::unique("id"));
    EXPECT_EQ(tableFixes.toStdString(), "UNIQUE (id),");
}

TEST_F(SqliteQueryableTest, ParseConstraintNullableRelaxesNotNull)
{
    QString tableFixes;
    QHash<QString, QString> fieldFixes;
    fieldFixes.insert("name", " TEXT NOT NULL");
    SqliteHelper::parseConstraint(&tableFixes, &fieldFixes, SqliteConstraint::nullable("name"));
    // NULLABLE removes " NOT NULL" and is NOT appended.
    EXPECT_EQ(fieldFixes.value("name").toStdString(), " TEXT");
    EXPECT_TRUE(tableFixes.isEmpty());
}

TEST_F(SqliteQueryableTest, ExcuteValidSqlReturnsTrueAndInvokesCallback)
{
    QString lastQuery;
    bool callbackCalled = false;
    EXPECT_TRUE(SqliteHelper::excute(dbPath, "CREATE TABLE ut_helper (x INTEGER);",
                                    &lastQuery,
                                    [&callbackCalled](QSqlQuery *) { callbackCalled = true; }));
    EXPECT_TRUE(callbackCalled);
}

TEST_F(SqliteQueryableTest, ExcuteInvalidSqlReturnsFalse)
{
    EXPECT_FALSE(SqliteHelper::excute(dbPath, "THIS IS NOT SQL;"));
}

TEST_F(SqliteQueryableTest, ExcuteWithDefaultArgsRunsValidSql)
{
    EXPECT_TRUE(SqliteHelper::excute(dbPath, "CREATE TABLE ut_helper2 (x INTEGER);"));
}

// ---------------------------------------------------------------------------
// SqliteHandle CRUD round-trip on a real database
// ---------------------------------------------------------------------------

TEST_F(SqliteQueryableTest, HandleCreatesAndDropsTable)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    ASSERT_TRUE(handle.dropTable<UtQueryableBean>());
}

TEST_F(SqliteQueryableTest, InsertAutoPkReturnsLastInsertId)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean bean;
    bean.setName("alice");
    int id = handle.insert<UtQueryableBean>(bean);
    EXPECT_GT(id, 0);
}

TEST_F(SqliteQueryableTest, InsertCustomPkIncludesPrimaryKey)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean bean;
    bean.setId(100);
    bean.setName("bob");
    int id = handle.insert<UtQueryableBean>(bean, /*customPK=*/true);
    EXPECT_EQ(id, 100);
}

TEST_F(SqliteQueryableTest, QueryToMapsReturnsAllRows)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean b1;
    b1.setName("alice");
    handle.insert<UtQueryableBean>(b1);
    UtQueryableBean b2;
    b2.setName("bob");
    handle.insert<UtQueryableBean>(b2);

    QList<QVariantMap> maps = handle.query<UtQueryableBean>().toMaps();
    ASSERT_EQ(maps.size(), 2);
    EXPECT_EQ(maps[0].value("name").toString().toStdString(), "alice");
    EXPECT_EQ(maps[1].value("name").toString().toStdString(), "bob");
}

TEST_F(SqliteQueryableTest, QueryToMapReturnsFirstRow)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean b;
    b.setName("alice");
    handle.insert<UtQueryableBean>(b);

    QVariantMap m = handle.query<UtQueryableBean>().toMap();
    ASSERT_FALSE(m.isEmpty());
    EXPECT_EQ(m.value("name").toString().toStdString(), "alice");
}

TEST_F(SqliteQueryableTest, QueryToMapOnEmptyTableReturnsEmptyMap)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    QVariantMap m = handle.query<UtQueryableBean>().toMap();
    EXPECT_TRUE(m.isEmpty());
}

TEST_F(SqliteQueryableTest, QueryToBeansDeserializesRows)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean b1;
    b1.setName("alice");
    handle.insert<UtQueryableBean>(b1);
    UtQueryableBean b2;
    b2.setName("bob");
    handle.insert<UtQueryableBean>(b2);

    QList<QSharedPointer<UtQueryableBean>> beans = handle.query<UtQueryableBean>().toBeans();
    ASSERT_EQ(beans.size(), 2);
    EXPECT_EQ(beans[0]->name().toStdString(), "alice");
    EXPECT_EQ(beans[1]->name().toStdString(), "bob");
}

TEST_F(SqliteQueryableTest, QueryToBeanReturnsFirstBean)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean b;
    b.setName("alice");
    handle.insert<UtQueryableBean>(b);

    QSharedPointer<UtQueryableBean> bean = handle.query<UtQueryableBean>().toBean();
    ASSERT_NE(bean, nullptr);
    EXPECT_EQ(bean->name().toStdString(), "alice");
}

TEST_F(SqliteQueryableTest, QueryToBeanOnEmptyTableReturnsNull)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    QSharedPointer<UtQueryableBean> bean = handle.query<UtQueryableBean>().toBean();
    EXPECT_EQ(bean, nullptr);
}

TEST_F(SqliteQueryableTest, AggregateCountReturnsRowCount)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean empty;
    handle.insert<UtQueryableBean>(empty);   // name empty
    UtQueryableBean b;
    b.setName("x");
    handle.insert<UtQueryableBean>(b);

    QVariant result = handle.query<UtQueryableBean>().aggregate(Expression::count());
    EXPECT_EQ(result.toInt(), 2);
}

TEST_F(SqliteQueryableTest, QueryWithWhereFiltersRows)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean b1;
    b1.setName("alice");
    handle.insert<UtQueryableBean>(b1);
    UtQueryableBean b2;
    b2.setName("bob");
    handle.insert<UtQueryableBean>(b2);

    QList<QVariantMap> maps = handle.query<UtQueryableBean>()
                                       .where(Field<UtQueryableBean>("name") == QVariant(QString("bob")))
                                       .toMaps();
    ASSERT_EQ(maps.size(), 1);
    EXPECT_EQ(maps[0].value("name").toString().toStdString(), "bob");
}

TEST_F(SqliteQueryableTest, UpdateModifiesMatchingRow)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean b;
    b.setName("alice");
    int id = handle.insert<UtQueryableBean>(b);
    ASSERT_GT(id, 0);

    ASSERT_TRUE(handle.update<UtQueryableBean>(
            SetExpr("name='alice2'"), Expr(Field<UtQueryableBean>("id"), "=", QVariant(id))));

    QList<QVariantMap> maps = handle.query<UtQueryableBean>()
                                       .where(Field<UtQueryableBean>("id") == QVariant(id))
                                       .toMaps();
    ASSERT_EQ(maps.size(), 1);
    EXPECT_EQ(maps[0].value("name").toString().toStdString(), "alice2");
}

TEST_F(SqliteQueryableTest, RemoveByEntityDeletesRow)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean b1;
    b1.setName("alice");
    int id1 = handle.insert<UtQueryableBean>(b1);
    UtQueryableBean b2;
    b2.setName("bob");
    handle.insert<UtQueryableBean>(b2);

    UtQueryableBean rm;
    rm.setId(id1);
    ASSERT_TRUE(handle.remove<UtQueryableBean>(rm));

    EXPECT_EQ(handle.query<UtQueryableBean>().aggregate(Expression::count()).toInt(), 1);
}

TEST_F(SqliteQueryableTest, RemoveByExprDeletesMatchingRows)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    UtQueryableBean b1;
    b1.setName("alice");
    handle.insert<UtQueryableBean>(b1);
    UtQueryableBean b2;
    b2.setName("bob");
    handle.insert<UtQueryableBean>(b2);

    ASSERT_TRUE(handle.remove<UtQueryableBean>(
            Field<UtQueryableBean>("name") == QVariant(QString("alice"))));

    EXPECT_EQ(handle.query<UtQueryableBean>().aggregate(Expression::count()).toInt(), 1);
}

TEST_F(SqliteQueryableTest, TransactionCommitReturnsTrueWhenFuncSucceeds)
{
    SqliteHandle handle(dbPath);
    EXPECT_TRUE(handle.transaction([] { return true; }));
}

TEST_F(SqliteQueryableTest, TransactionRollbackPathWhenFuncFails)
{
    SqliteHandle handle(dbPath);
    // rollback() succeeds and returns true when there is an active transaction.
    EXPECT_TRUE(handle.transaction([] { return false; }));
}

TEST_F(SqliteQueryableTest, LastQueryReflectsMostRecentSql)
{
    SqliteHandle handle(dbPath);
    ASSERT_TRUE(handle.createTable<UtQueryableBean>(SqliteConstraint::primary("id")));
    EXPECT_FALSE(handle.lastQuery().isEmpty());
    EXPECT_TRUE(handle.lastQuery().contains("CREATE TABLE"));
}

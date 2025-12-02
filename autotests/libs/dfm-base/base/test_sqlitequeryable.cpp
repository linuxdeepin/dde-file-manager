// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QObject>
#include <QTemporaryDir>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QCoreApplication>

#include "stubext.h"

#include <dfm-base/base/db/sqlitequeryable.h>
#include <dfm-base/base/db/sqlitehandle.h>
#include <dfm-base/base/db/sqlitehelper.h>

DFMBASE_USE_NAMESPACE

// Test entity for queryable operations
class Product : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("TableName", "products")

    Q_PROPERTY(int id READ id WRITE setId)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(double price READ price WRITE setPrice)
    Q_PROPERTY(int stock READ stock WRITE setStock)
    Q_PROPERTY(QString category READ category WRITE setCategory)

public:
    explicit Product(QObject *parent = nullptr) : QObject(parent) {}

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }

    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }

    double price() const { return m_price; }
    void setPrice(double price) { m_price = price; }

    int stock() const { return m_stock; }
    void setStock(int stock) { m_stock = stock; }

    QString category() const { return m_category; }
    void setCategory(const QString &category) { m_category = category; }

private:
    int m_id { 0 };
    QString m_name;
    double m_price { 0.0 };
    int m_stock { 0 };
    QString m_category;
};

class TestSqliteQueryable : public testing::Test
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

        // Create table and insert test data
        handle->createTable<Product>(
                SqliteConstraint::primary("id"),
                SqliteConstraint::autoIncreament("id"));

        insertTestData();
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
    void insertTestData()
    {
        // Insert sample products
        struct ProductData {
            QString name;
            double price;
            int stock;
            QString category;
        };

        QList<ProductData> products = {
            { "Laptop", 999.99, 10, "Electronics" },
            { "Mouse", 29.99, 50, "Electronics" },
            { "Keyboard", 79.99, 30, "Electronics" },
            { "Monitor", 299.99, 15, "Electronics" },
            { "Desk", 199.99, 5, "Furniture" },
            { "Chair", 149.99, 8, "Furniture" },
            { "Lamp", 39.99, 20, "Furniture" },
            { "Notebook", 4.99, 100, "Stationery" },
            { "Pen", 1.99, 200, "Stationery" },
            { "Eraser", 0.99, 150, "Stationery" }
        };

        for (const auto &data : products) {
            Product product;
            product.setName(data.name);
            product.setPrice(data.price);
            product.setStock(data.stock);
            product.setCategory(data.category);
            handle->insert(product);
        }
    }

    stub_ext::StubExt stub;
    std::unique_ptr<QTemporaryDir> tempDir;
    std::unique_ptr<SqliteHandle> handle;
    QString dbPath;
    QCoreApplication *app { nullptr };
};

// ========== Basic Query Tests ==========

TEST_F(TestSqliteQueryable, toBean_ReturnsFirstRecord)
{
    // Test toBean returns first record
    auto product = handle->query<Product>().toBean();

    ASSERT_TRUE(product != nullptr);
    EXPECT_GT(product->id(), 0);
    EXPECT_FALSE(product->name().isEmpty());
}

TEST_F(TestSqliteQueryable, toBeans_ReturnsAllRecords)
{
    // Test toBeans returns all records
    auto products = handle->query<Product>().toBeans();

    EXPECT_EQ(products.size(), 10);

    for (const auto &product : products) {
        EXPECT_GT(product->id(), 0);
        EXPECT_FALSE(product->name().isEmpty());
    }
}

TEST_F(TestSqliteQueryable, toMap_ReturnsFirstRecordAsMap)
{
    // Test toMap returns first record as QVariantMap
    QVariantMap map = handle->query<Product>().toMap();

    EXPECT_FALSE(map.isEmpty());
    EXPECT_TRUE(map.contains("id"));
    EXPECT_TRUE(map.contains("name"));
    EXPECT_TRUE(map.contains("price"));
}

TEST_F(TestSqliteQueryable, toMaps_ReturnsAllRecordsAsMaps)
{
    // Test toMaps returns all records as list of maps
    QList<QVariantMap> maps = handle->query<Product>().toMaps();

    EXPECT_EQ(maps.size(), 10);

    for (const auto &map : maps) {
        EXPECT_TRUE(map.contains("id"));
        EXPECT_TRUE(map.contains("name"));
        EXPECT_TRUE(map.contains("price"));
        EXPECT_TRUE(map.contains("stock"));
        EXPECT_TRUE(map.contains("category"));
    }
}

// ========== WHERE Clause Tests ==========

TEST_F(TestSqliteQueryable, where_EqualCondition)
{
    // Test WHERE with equal condition
    auto products = handle->query<Product>()
                            .where(Expression::Field<Product>("category") == QString("Electronics"))
                            .toBeans();

    EXPECT_EQ(products.size(), 4);

    for (const auto &product : products) {
        EXPECT_EQ(product->category(), QString("Electronics"));
    }
}

TEST_F(TestSqliteQueryable, where_GreaterThan)
{
    // Test WHERE with greater than condition
    auto products = handle->query<Product>()
                            .where(Expression::Field<Product>("price") > 100.0)
                            .toBeans();

    EXPECT_GT(products.size(), 0);

    for (const auto &product : products) {
        EXPECT_GT(product->price(), 100.0);
    }
}

TEST_F(TestSqliteQueryable, where_LessThan)
{
    // Test WHERE with less than condition
    auto products = handle->query<Product>()
                            .where(Expression::Field<Product>("stock") < 20)
                            .toBeans();

    EXPECT_GT(products.size(), 0);

    for (const auto &product : products) {
        EXPECT_LT(product->stock(), 20);
    }
}

TEST_F(TestSqliteQueryable, where_ComplexCondition)
{
    // Test WHERE with complex AND/OR conditions
    auto priceField = Expression::Field<Product>("price");
    auto categoryField = Expression::Field<Product>("category");

    auto products = handle->query<Product>()
                            .where((priceField > 50.0) && (categoryField == QString("Electronics")))
                            .toBeans();

    EXPECT_GT(products.size(), 0);

    for (const auto &product : products) {
        EXPECT_GT(product->price(), 50.0);
        EXPECT_EQ(product->category(), QString("Electronics"));
    }
}

TEST_F(TestSqliteQueryable, where_OrCondition)
{
    // Test WHERE with OR condition
    auto categoryField = Expression::Field<Product>("category");

    auto products = handle->query<Product>()
                            .where((categoryField == QString("Furniture")) ||
                                   (categoryField == QString("Stationery")))
                            .toBeans();

    EXPECT_EQ(products.size(), 6);

    for (const auto &product : products) {
        EXPECT_TRUE(product->category() == QString("Furniture") ||
                    product->category() == QString("Stationery"));
    }
}

TEST_F(TestSqliteQueryable, where_LikeOperator)
{
    // Test WHERE with LIKE operator
    auto nameField = Expression::Field<Product>("name");

    auto products = handle->query<Product>()
                            .where(nameField & QString("%book%"))
                            .toBeans();

    EXPECT_GT(products.size(), 0);

    for (const auto &product : products) {
        EXPECT_TRUE(product->name().contains("book", Qt::CaseInsensitive));
    }
}

// ========== ORDER BY Tests ==========

TEST_F(TestSqliteQueryable, orderBy_Ascending)
{
    // Test ORDER BY ascending
    auto products = handle->query<Product>()
                            .orderBy(Expression::Field<Product>("price"))
                            .toBeans();

    EXPECT_EQ(products.size(), 10);

    for (int i = 1; i < products.size(); ++i) {
        EXPECT_GE(products[i]->price(), products[i - 1]->price());
    }
}

TEST_F(TestSqliteQueryable, orderByDescending_Descending)
{
    // Test ORDER BY descending
    auto products = handle->query<Product>()
                            .orderByDescending(Expression::Field<Product>("price"))
                            .toBeans();

    EXPECT_EQ(products.size(), 10);

    for (int i = 1; i < products.size(); ++i) {
        EXPECT_LE(products[i]->price(), products[i - 1]->price());
    }
}

TEST_F(TestSqliteQueryable, orderBy_MultipleFields)
{
    // Test ORDER BY with multiple fields
    auto products = handle->query<Product>()
                            .orderBy(Expression::Field<Product>("category"))
                            .orderBy(Expression::Field<Product>("price"))
                            .toBeans();

    EXPECT_EQ(products.size(), 10);

    // Verify ordering by category then price
    for (int i = 1; i < products.size(); ++i) {
        if (products[i]->category() == products[i - 1]->category()) {
            EXPECT_GE(products[i]->price(), products[i - 1]->price());
        }
    }
}

TEST_F(TestSqliteQueryable, orderBy_MixedAscDesc)
{
    // Test ORDER BY with mixed ascending and descending
    auto products = handle->query<Product>()
                            .orderBy(Expression::Field<Product>("category"))
                            .orderByDescending(Expression::Field<Product>("stock"))
                            .toBeans();

    EXPECT_EQ(products.size(), 10);
}

// ========== LIMIT and OFFSET Tests ==========

TEST_F(TestSqliteQueryable, take_LimitRecords)
{
    // Test LIMIT (take)
    auto products = handle->query<Product>()
                            .take(5)
                            .toBeans();

    EXPECT_EQ(products.size(), 5);
}

TEST_F(TestSqliteQueryable, skip_OffsetRecords)
{
    // Test OFFSET (skip)
    auto allProducts = handle->query<Product>()
                               .orderBy(Expression::Field<Product>("id"))
                               .toBeans();

    auto skippedProducts = handle->query<Product>()
                                   .orderBy(Expression::Field<Product>("id"))
                                   .skip(3)
                                   .toBeans();

    EXPECT_EQ(skippedProducts.size(), allProducts.size() - 3);

    // Verify skipped records
    for (int i = 0; i < skippedProducts.size(); ++i) {
        EXPECT_EQ(skippedProducts[i]->id(), allProducts[i + 3]->id());
    }
}

TEST_F(TestSqliteQueryable, take_and_skip_Pagination)
{
    // Test pagination with LIMIT and OFFSET
    int pageSize = 3;
    int pageNumber = 2;  // Second page (0-indexed: skip first 3, take next 3)

    auto products = handle->query<Product>()
                            .orderBy(Expression::Field<Product>("id"))
                            .skip(pageNumber * pageSize)
                            .take(pageSize)
                            .toBeans();

    EXPECT_EQ(products.size(), pageSize);
}

TEST_F(TestSqliteQueryable, take_ZeroRecords)
{
    // Test LIMIT 0
    auto products = handle->query<Product>()
                            .take(0)
                            .toBeans();

    EXPECT_EQ(products.size(), 0);
}

TEST_F(TestSqliteQueryable, skip_AllRecords)
{
    // Test skipping all records
    auto products = handle->query<Product>()
                            .skip(100)  // More than total records
                            .toBeans();

    EXPECT_EQ(products.size(), 0);
}

// ========== GROUP BY and HAVING Tests ==========

TEST_F(TestSqliteQueryable, groupBy_SingleField)
{
    // Test GROUP BY
    auto results = handle->query<Product>()
                           .groupBy(Expression::Field<Product>("category"))
                           .toMaps();

    // Results should be grouped by category
    EXPECT_GT(results.size(), 0);
}

TEST_F(TestSqliteQueryable, having_WithGroupBy)
{
    // Test HAVING clause with GROUP BY
    auto stockField = Expression::Field<Product>("stock");

    auto results = handle->query<Product>()
                           .groupBy(Expression::Field<Product>("category"))
                           .having(stockField > 10)
                           .toMaps();

    EXPECT_GT(results.size(), 0);
}

// ========== DISTINCT Tests ==========

TEST_F(TestSqliteQueryable, disctinct_UniqueCategories)
{
    // Test DISTINCT
    auto results = handle->query<Product>()
                           .disctinct()
                           .toMaps();

    EXPECT_GT(results.size(), 0);
}

// ========== Aggregate Function Tests ==========

TEST_F(TestSqliteQueryable, aggregate_Count)
{
    // Test COUNT aggregate
    QVariant count = handle->query<Product>()
                             .aggregate(Expression::count());

    EXPECT_TRUE(count.isValid());
    EXPECT_EQ(count.toInt(), 10);
}

TEST_F(TestSqliteQueryable, aggregate_CountWithField)
{
    // Test COUNT(field)
    QVariant count = handle->query<Product>()
                             .aggregate(Expression::count("id"));

    EXPECT_TRUE(count.isValid());
    EXPECT_EQ(count.toInt(), 10);
}

TEST_F(TestSqliteQueryable, aggregate_Sum)
{
    // Test SUM aggregate
    QVariant sumStock = handle->query<Product>()
                                .aggregate(Expression::sum("stock"));

    EXPECT_TRUE(sumStock.isValid());
    EXPECT_GT(sumStock.toInt(), 0);
}

TEST_F(TestSqliteQueryable, aggregate_Avg)
{
    // Test AVG aggregate
    QVariant avgPrice = handle->query<Product>()
                                .aggregate(Expression::avg("price"));

    EXPECT_TRUE(avgPrice.isValid());
    EXPECT_GT(avgPrice.toDouble(), 0.0);
}

TEST_F(TestSqliteQueryable, aggregate_Max)
{
    // Test MAX aggregate
    QVariant maxPrice = handle->query<Product>()
                                .aggregate(Expression::max("price"));

    EXPECT_TRUE(maxPrice.isValid());
    EXPECT_GE(maxPrice.toDouble(), 999.99);
}

TEST_F(TestSqliteQueryable, aggregate_Min)
{
    // Test MIN aggregate
    QVariant minPrice = handle->query<Product>()
                                .aggregate(Expression::min("price"));

    EXPECT_TRUE(minPrice.isValid());
    EXPECT_LE(minPrice.toDouble(), 1.0);
}

TEST_F(TestSqliteQueryable, aggregate_WithWhere)
{
    // Test aggregate with WHERE clause
    QVariant count = handle->query<Product>()
                             .where(Expression::Field<Product>("category") == QString("Electronics"))
                             .aggregate(Expression::count());

    EXPECT_TRUE(count.isValid());
    EXPECT_EQ(count.toInt(), 4);
}

// ========== Complex Query Tests ==========

TEST_F(TestSqliteQueryable, ComplexQuery_WhereOrderLimit)
{
    // Test complex query with WHERE, ORDER BY, and LIMIT
    auto products = handle->query<Product>()
                            .where(Expression::Field<Product>("price") > 50.0)
                            .orderBy(Expression::Field<Product>("price"))
                            .take(3)
                            .toBeans();

    EXPECT_LE(products.size(), 3);

    for (const auto &product : products) {
        EXPECT_GT(product->price(), 50.0);
    }

    // Verify ordering
    for (int i = 1; i < products.size(); ++i) {
        EXPECT_GE(products[i]->price(), products[i - 1]->price());
    }
}

TEST_F(TestSqliteQueryable, ComplexQuery_MultipleConditionsAndOrdering)
{
    // Test complex query with multiple conditions
    auto priceField = Expression::Field<Product>("price");
    auto stockField = Expression::Field<Product>("stock");

    auto products = handle->query<Product>()
                            .where((priceField < 100.0) && (stockField > 10))
                            .orderByDescending(Expression::Field<Product>("stock"))
                            .take(5)
                            .toBeans();

    EXPECT_LE(products.size(), 5);

    for (const auto &product : products) {
        EXPECT_LT(product->price(), 100.0);
        EXPECT_GT(product->stock(), 10);
    }
}

TEST_F(TestSqliteQueryable, ComplexQuery_Pagination)
{
    // Test pagination scenario
    int pageSize = 4;
    int totalCount = handle->query<Product>()
                             .aggregate(Expression::count())
                             .toInt();

    int totalPages = (totalCount + pageSize - 1) / pageSize;

    for (int page = 0; page < totalPages; ++page) {
        auto products = handle->query<Product>()
                                .orderBy(Expression::Field<Product>("id"))
                                .skip(page * pageSize)
                                .take(pageSize)
                                .toBeans();

        EXPECT_LE(products.size(), pageSize);

        if (page < totalPages - 1) {
            EXPECT_EQ(products.size(), pageSize);
        }
    }
}

// ========== Edge Cases Tests ==========

TEST_F(TestSqliteQueryable, EdgeCase_EmptyResult)
{
    // Test query with no matching results
    auto products = handle->query<Product>()
                            .where(Expression::Field<Product>("price") > 10000.0)
                            .toBeans();

    EXPECT_EQ(products.size(), 0);
}

TEST_F(TestSqliteQueryable, EdgeCase_SingleResult)
{
    // Test query returning single result
    auto product = handle->query<Product>()
                           .where(Expression::Field<Product>("name") == QString("Laptop"))
                           .toBean();

    ASSERT_TRUE(product != nullptr);
    EXPECT_EQ(product->name(), QString("Laptop"));
}

TEST_F(TestSqliteQueryable, EdgeCase_ChainedMethods)
{
    // Test chaining multiple query methods
    auto products = handle->query<Product>()
                            .where(Expression::Field<Product>("category") == QString("Electronics"))
                            .orderBy(Expression::Field<Product>("price"))
                            .skip(1)
                            .take(2)
                            .toBeans();

    EXPECT_LE(products.size(), 2);
}

// ========== Integration Tests ==========

TEST_F(TestSqliteQueryable, Integration_SearchAndSort)
{
    // Test search and sort integration
    auto nameField = Expression::Field<Product>("name");
    auto products = handle->query<Product>()
                            .where(nameField & QString("%e%"))  // Contains 'e'
                            .orderBy(Expression::Field<Product>("name"))
                            .toBeans();

    EXPECT_GT(products.size(), 0);

    for (const auto &product : products) {
        EXPECT_TRUE(product->name().contains("e", Qt::CaseInsensitive));
    }
}

TEST_F(TestSqliteQueryable, Integration_FilterAndAggregate)
{
    // Test filter with aggregate
    QVariant avgPrice = handle->query<Product>()
                                .where(Expression::Field<Product>("category") == QString("Electronics"))
                                .aggregate(Expression::avg("price"));

    EXPECT_TRUE(avgPrice.isValid());
    EXPECT_GT(avgPrice.toDouble(), 0.0);
}

TEST_F(TestSqliteQueryable, Integration_CompleteWorkflow)
{
    // Test complete query workflow
    auto categoryField = Expression::Field<Product>("category");
    auto priceField = Expression::Field<Product>("price");

    // Get electronics products, ordered by price, paginated
    auto products = handle->query<Product>()
                            .where((categoryField == QString("Electronics")) && (priceField > 50.0))
                            .orderByDescending(Expression::Field<Product>("price"))
                            .skip(0)
                            .take(3)
                            .toBeans();

    EXPECT_LE(products.size(), 3);

    // Verify all conditions met
    for (const auto &product : products) {
        EXPECT_EQ(product->category(), QString("Electronics"));
        EXPECT_GT(product->price(), 50.0);
    }

    // Verify descending order
    for (int i = 1; i < products.size(); ++i) {
        EXPECT_LE(products[i]->price(), products[i - 1]->price());
    }
}

#include "test_sqlitequeryable.moc"

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexcontentmigrator.cpp
 * @brief Unit tests for IndexContentMigrator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/indexcontentmigrator.h"

#include <QTest>

using namespace src;

class IndexContentMigratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IndexContentMigrator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IndexContentMigrator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IndexContentMigratorTest, cleanup)
{
    // Test method: void cleanup(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanup());
}

TEST_F(IndexContentMigratorTest, lookupContent)
{
    // Test method: std::optional<QString> lookupContent((const QString &filePath))
    QString _arg0{};
    auto result = obj->lookupContent(_arg0);
    EXPECT_FALSE(result.has_value());

}

// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_indexcontentmigrator.cpp
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

TEST_F(IndexContentMigratorTest, IndexContentMigrator)
{
    // Test constructor: IndexContentMigrator(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(IndexContentMigratorTest, prepare)
{
    // Test method: DFM_SEARCH_USE_NS prepare((const QString &indexDir, const IndexProfile &profile))
    QString _arg0{};
    IndexProfile _arg1{};
    auto result = obj->prepare(_arg0, _arg1);
    EXPECT_NO_FATAL_FAILURE({ obj->prepare(_arg0, _arg1); });

}

TEST_F(IndexContentMigratorTest, cleanup)
{
    // Test method: void cleanup(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanup());
}

TEST_F(IndexContentMigratorTest, operator=)
{
    // Test getter: IndexContentMigrator operator=()
    EXPECT_NO_FATAL_FAILURE({ obj->operator=(); });
}

TEST_F(IndexContentMigratorTest, lookupContent)
{
    // Test method: std::optional<QString> lookupContent((const QString &filePath))
    QString _arg0{};
    auto result = obj->lookupContent(_arg0);
    EXPECT_FALSE(result.has_value());

}

TEST_F(IndexContentMigratorTest, isActive)
{
    // Test bool getter: isActive()
    bool result = obj->isActive();
    EXPECT_FALSE(result);

}

TEST_F(IndexContentMigratorTest, hasResidue)
{
    // Test method: bool hasResidue((const QString &indexDir))
    QString _arg0{};
    auto result = obj->hasResidue(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(IndexContentMigratorTest, m_reader)
{
    // Test getter: Lucene::IndexReaderPtr m_reader()
    auto result = obj->m_reader();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(IndexContentMigratorTest, m_searcher)
{
    // Test getter: Lucene::SearcherPtr m_searcher()
    auto result = obj->m_searcher();
    EXPECT_EQ(result.get(), nullptr);

}

TEST_F(IndexContentMigratorTest, m_pathField)
{
    // Test getter: wchar_t m_pathField()
    auto result = obj->m_pathField();
    EXPECT_NO_FATAL_FAILURE({ obj->m_pathField(); });

}

TEST_F(IndexContentMigratorTest, m_contentField)
{
    // Test getter: wchar_t m_contentField()
    auto result = obj->m_contentField();
    EXPECT_NO_FATAL_FAILURE({ obj->m_contentField(); });

}

TEST_F(IndexContentMigratorTest, m_oldIndexDir)
{
    // Test getter: QString m_oldIndexDir()
    auto result = obj->m_oldIndexDir();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(IndexContentMigratorTest, m_modifyTimeField)
{
    // Test getter: wchar_t m_modifyTimeField()
    auto result = obj->m_modifyTimeField();
    EXPECT_NO_FATAL_FAILURE({ obj->m_modifyTimeField(); });

}

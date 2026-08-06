// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexcontentmigrator.cpp
 * @brief Unit tests for IndexContentMigrator (task/indexcontentmigrator.cpp)
 *        — the dependency-light subset: default state (isActive/lookupContent/
 *        cleanup as no-ops), prepare() with empty/nonexistent index dir (early
 *        returns false, no lucene), and hasResidue() static check. The full
 *        lucene open path is intentionally not exercised.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QString>
#include <optional>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/task/indexcontentmigrator.h"
#include "services/textindex/profile/indexprofile.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

static IndexProfile makeMigratorProfile()
{
    return IndexProfile(IndexProfile::Type::Content,
                        "migrator_test",
                        "migrator_status.json",
                        "migrator_version",
                        1,
                        []() -> QString { return "/tmp/dfm_migrator_test"; },
                        []() -> bool { return true; },
                        [](const QString &) -> bool { return true; },
                        [](const QString &) -> bool { return true; });
}

TEST(IndexContentMigratorTest, DefaultNotActive)
{
    IndexContentMigrator m;
    EXPECT_FALSE(m.isActive());
}

TEST(IndexContentMigratorTest, LookupContentWhenNotActiveReturnsNullopt)
{
    IndexContentMigrator m;
    auto result = m.lookupContent("/some/file.txt");
    EXPECT_FALSE(result.has_value());
}

TEST(IndexContentMigratorTest, CleanupWhenNotActiveIsNoOp)
{
    IndexContentMigrator m;
    EXPECT_NO_FATAL_FAILURE({ m.cleanup(); });
}

TEST(IndexContentMigratorTest, PrepareEmptyIndexDirReturnsFalse)
{
    IndexContentMigrator m;
    EXPECT_FALSE(m.prepare(QString(), makeMigratorProfile()));
    EXPECT_FALSE(m.isActive());
}

TEST(IndexContentMigratorTest, PrepareNonexistentIndexDirReturnsFalse)
{
    IndexContentMigrator m;
    EXPECT_FALSE(m.prepare("/nonexistent/dfm/index/dir", makeMigratorProfile()));
    EXPECT_FALSE(m.isActive());
}

TEST(IndexContentMigratorTest, HasResidueOnNonexistentDirReturnsFalse)
{
    EXPECT_FALSE(IndexContentMigrator::hasResidue("/nonexistent/dfm/index/dir"));
}

TEST(IndexContentMigratorTest, HasResidueOnEmptyDirReturnsFalse)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    EXPECT_FALSE(IndexContentMigrator::hasResidue(tmp.path()));
}

TEST(IndexContentMigratorTest, HasResidueDetectsOldDir)
{
    QTemporaryDir tmp;
    ASSERT_TRUE(tmp.isValid());
    QDir().mkpath(tmp.path() + ".old");
    EXPECT_TRUE(IndexContentMigrator::hasResidue(tmp.path()));
}

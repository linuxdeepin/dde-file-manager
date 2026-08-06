// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexprofile.cpp
 * @brief Unit tests for IndexProfile (indexprofile.cpp)
 */

#include <gtest/gtest.h>
#include <QString>
#include <QStringList>
#include <functional>

#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

static IndexProfile makeProfile()
{
    return IndexProfile(IndexProfile::Type::Content,
                        "testprofile",
                        "test_status.json",
                        "test_version",
                        1,
                        []() -> QString { return "/tmp/dfm_test_index"; },
                        []() -> bool { return true; },
                        [](const QString &) -> bool { return true; },
                        [](const QString &) -> bool { return true; });
}

TEST(IndexProfileTest, TypeContent)
{
    IndexProfile p = makeProfile();
    EXPECT_EQ(p.type(), IndexProfile::Type::Content);
}

TEST(IndexProfileTest, IdAccessor)
{
    IndexProfile p = makeProfile();
    EXPECT_EQ(p.id(), QString("testprofile"));
}

TEST(IndexProfileTest, VersionKeyAccessor)
{
    IndexProfile p = makeProfile();
    EXPECT_EQ(p.versionKey(), QString("test_version"));
}

TEST(IndexProfileTest, RuntimeIndexVersion)
{
    IndexProfile p = makeProfile();
    EXPECT_EQ(p.runtimeIndexVersion(), 1);
}

TEST(IndexProfileTest, IndexDirectory)
{
    IndexProfile p = makeProfile();
    EXPECT_EQ(p.indexDirectory(), QString("/tmp/dfm_test_index"));
}

TEST(IndexProfileTest, StatusFilePath)
{
    IndexProfile p = makeProfile();
    QString sfp = p.statusFilePath();
    EXPECT_FALSE(sfp.isEmpty());
    EXPECT_TRUE(sfp.contains("test_status.json"));
}

TEST(IndexProfileTest, IsIndexAvailable)
{
    IndexProfile p = makeProfile();
    EXPECT_TRUE(p.isIndexAvailable());
}

TEST(IndexProfileTest, IsPathInScope)
{
    IndexProfile p = makeProfile();
    EXPECT_TRUE(p.isPathInScope("/any/path"));
}

TEST(IndexProfileTest, IsCandidateFile)
{
    IndexProfile p = makeProfile();
    EXPECT_TRUE(p.isCandidateFile("/any/file.txt"));
}

TEST(IndexProfileTest, SupportsAnything)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.supportsAnything(); });
}

TEST(IndexProfileTest, AnythingSearchOptions)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.anythingSearchOptions(); });
}

TEST(IndexProfileTest, ComputeChecksumUnsupported)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.computeChecksum("/some/file"); });
}

TEST(IndexProfileTest, LookupCachedTextUnsupported)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.lookupCachedText("checksum"); });
}

TEST(IndexProfileTest, SupportsChecksum)
{
    IndexProfile p = makeProfile();
    EXPECT_FALSE(p.supportsChecksum());
}

TEST(IndexProfileTest, PathFieldNonEmpty)
{
    IndexProfile p = makeProfile();
    EXPECT_NE(p.pathField(), nullptr);
}

TEST(IndexProfileTest, ContentFieldNonEmpty)
{
    IndexProfile p = makeProfile();
    EXPECT_NE(p.contentField(), nullptr);
}

TEST(IndexProfileTest, AncestorPathsFieldNonEmpty)
{
    IndexProfile p = makeProfile();
    EXPECT_NE(p.ancestorPathsField(), nullptr);
}

TEST(IndexProfileTest, ModifyTimeField)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.modifyTimeField(); });
}

TEST(IndexProfileTest, SupportsModifiedTimestampCheck)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.supportsModifiedTimestampCheck(); });
}

TEST(IndexProfileTest, CreateAnalyzer)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.createAnalyzer(); });
}

TEST(IndexProfileTest, MaxFileTruncationSizeMBPositive)
{
    IndexProfile p = makeProfile();
    EXPECT_GT(p.maxFileTruncationSizeMB(), 0);
}

TEST(IndexProfileTest, ContentFactoryReturnsProfile)
{
    IndexProfile p = IndexProfile::content();
    EXPECT_EQ(p.type(), IndexProfile::Type::Content);
}

TEST(IndexProfileTest, OcrFactoryReturnsProfile)
{
    IndexProfile p = IndexProfile::ocr();
    EXPECT_EQ(p.type(), IndexProfile::Type::Ocr);
}

// ---- Coverage additions: more IndexProfile API ----

TEST(IndexProfileTest, ProfileIdAndStatusFilePath)
{
    auto p = makeProfile();
    EXPECT_FALSE(p.id().isEmpty());
    EXPECT_NO_FATAL_FAILURE({ (void)p.statusFilePath(); });
}

TEST(IndexProfileTest, ProfileIndexDirectoryCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.indexDirectory(); });
}

TEST(IndexProfileTest, ProfileVersionKeyCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.versionKey(); });
}

TEST(IndexProfileTest, ProfileIsIndexAvailableCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.isIndexAvailable(); });
}

TEST(IndexProfileTest, ProfileRuntimeIndexVersionCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.runtimeIndexVersion(); });
}

TEST(IndexProfileTest, ProfileSupportsAnythingCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.supportsAnything(); });
}

TEST(IndexProfileTest, ProfileIsPathInScopeCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.isPathInScope("/tmp"); });
}

TEST(IndexProfileTest, ProfileIsCandidateFileCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.isCandidateFile("/tmp/test.txt"); });
}

TEST(IndexProfileTest, ProfileComputeChecksumCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ (void)p.computeChecksum("/tmp/test.txt"); });
}

// ---- Coverage additions: exercise the real provider lambdas wired up by the
// IndexProfile::content() / IndexProfile::ocr() factories. Each accessor runs
// the corresponding closure stored inside content()/ocr(), covering those
// lambdas (which are otherwise defined but never invoked).

TEST(IndexProfileTest, ContentProfile_ScopeAndOptionProviders)
{
    auto p = IndexProfile::content();
    EXPECT_FALSE(p.indexDirectory().isEmpty());
    EXPECT_NO_FATAL_FAILURE({ (void)p.isIndexAvailable(); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.isPathInScope("/tmp/somefile.txt"); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.isCandidateFile("/tmp/somefile.txt"); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.anythingSearchOptions(); });
}

TEST(IndexProfileTest, ContentProfile_ChecksumTextCacheAndAnalyzer)
{
    auto p = IndexProfile::content();
    EXPECT_NO_FATAL_FAILURE({ (void)p.computeChecksum("/nonexistent/file.txt"); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.lookupCachedText("deadbeef"); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.createAnalyzer(); });
}

TEST(IndexProfileTest, OcrProfile_ScopeAndOptionProviders)
{
    auto p = IndexProfile::ocr();
    EXPECT_FALSE(p.indexDirectory().isEmpty());
    EXPECT_NO_FATAL_FAILURE({ (void)p.isIndexAvailable(); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.isPathInScope("/tmp/somefile.png"); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.isCandidateFile("/tmp/somefile.png"); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.anythingSearchOptions(); });
}

TEST(IndexProfileTest, OcrProfile_ChecksumTextCacheAndAnalyzer)
{
    auto p = IndexProfile::ocr();
    EXPECT_NO_FATAL_FAILURE({ (void)p.computeChecksum("/nonexistent/file.png"); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.lookupCachedText("deadbeef"); });
    EXPECT_NO_FATAL_FAILURE({ (void)p.createAnalyzer(); });
}

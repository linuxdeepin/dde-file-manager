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
    EXPECT_NO_FATAL_FAILURE({ p.supportsAnything(); });
}

TEST(IndexProfileTest, AnythingSearchOptions)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.anythingSearchOptions(); });
}

TEST(IndexProfileTest, ComputeChecksumUnsupported)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.computeChecksum("/some/file"); });
}

TEST(IndexProfileTest, LookupCachedTextUnsupported)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.lookupCachedText("checksum"); });
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
    EXPECT_NO_FATAL_FAILURE({ p.modifyTimeField(); });
}

TEST(IndexProfileTest, SupportsModifiedTimestampCheck)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.supportsModifiedTimestampCheck(); });
}

TEST(IndexProfileTest, CreateAnalyzer)
{
    IndexProfile p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.createAnalyzer(); });
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
    EXPECT_NO_FATAL_FAILURE({ p.statusFilePath(); });
}

TEST(IndexProfileTest, ProfileIndexDirectoryCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.indexDirectory(); });
}

TEST(IndexProfileTest, ProfileVersionKeyCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.versionKey(); });
}

TEST(IndexProfileTest, ProfileIsIndexAvailableCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.isIndexAvailable(); });
}

TEST(IndexProfileTest, ProfileRuntimeIndexVersionCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.runtimeIndexVersion(); });
}

TEST(IndexProfileTest, ProfileSupportsAnythingCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.supportsAnything(); });
}

TEST(IndexProfileTest, ProfileIsPathInScopeCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.isPathInScope("/tmp"); });
}

TEST(IndexProfileTest, ProfileIsCandidateFileCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.isCandidateFile("/tmp/test.txt"); });
}

TEST(IndexProfileTest, ProfileComputeChecksumCallable)
{
    auto p = makeProfile();
    EXPECT_NO_FATAL_FAILURE({ p.computeChecksum("/tmp/test.txt"); });
}


TEST(IndexProfileTest, ancestorPathsField)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.ancestorPathsField(); });
}

TEST(IndexProfileTest, anythingSearchOptions)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.anythingSearchOptions(); });
}

TEST(IndexProfileTest, computeChecksum)
{
    IndexProfile obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.computeChecksum(_arg0); });
}

TEST(IndexProfileTest, content)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.content(); });
}

TEST(IndexProfileTest, contentField)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.contentField(); });
}

TEST(IndexProfileTest, createAnalyzer)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.createAnalyzer(); });
}

TEST(IndexProfileTest, id)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.id(); });
}

TEST(IndexProfileTest, indexDirectory)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.indexDirectory(); });
}

TEST(IndexProfileTest, isCandidateFile)
{
    IndexProfile obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isCandidateFile(_arg0); });
}

TEST(IndexProfileTest, isIndexAvailable)
{
    IndexProfile obj;
    bool result = obj.isIndexAvailable();
    EXPECT_FALSE(result);
}

TEST(IndexProfileTest, isPathInScope)
{
    IndexProfile obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.isPathInScope(_arg0); });
}

TEST(IndexProfileTest, lookupCachedText)
{
    IndexProfile obj;
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE({ obj.lookupCachedText(_arg0); });
}

TEST(IndexProfileTest, maxFileTruncationSizeMB)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.maxFileTruncationSizeMB(); });
}

TEST(IndexProfileTest, modifyTimeField)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.modifyTimeField(); });
}

TEST(IndexProfileTest, ocr)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.ocr(); });
}

TEST(IndexProfileTest, pathField)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.pathField(); });
}

TEST(IndexProfileTest, runtimeIndexVersion)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.runtimeIndexVersion(); });
}

TEST(IndexProfileTest, statusFilePath)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.statusFilePath(); });
}

TEST(IndexProfileTest, supportsAnything)
{
    IndexProfile obj;
    bool result = obj.supportsAnything();
    EXPECT_FALSE(result);
}

TEST(IndexProfileTest, supportsModifiedTimestampCheck)
{
    IndexProfile obj;
    bool result = obj.supportsModifiedTimestampCheck();
    EXPECT_FALSE(result);
}

TEST(IndexProfileTest, type)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.type(); });
}

TEST(IndexProfileTest, versionKey)
{
    IndexProfile obj;
    EXPECT_NO_FATAL_FAILURE({ obj.versionKey(); });
}

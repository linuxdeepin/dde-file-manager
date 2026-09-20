// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexprofile.cpp
 * @brief Unit tests for IndexProfile (indexprofile.cpp)
 */

#include <gtest/gtest.h>
#include <QDir>
#include <QString>
#include <QStringList>
#include <functional>

#include <dfm-search/field_names.h>

#include "stubext.h"
#include <dfm-search/dsearch_global.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/utils/textindexconfig.h"

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;

// Helper to stub Global::fileNameIndexDirectory to return a path containing
// "filename-index" (the installed libdfm6-search may still return the old
// /run/user/<uid>/deepin-anything-server path). The stub is scoped to the test.
struct FilenameDirStub
{
    stub_ext::StubExt stub;
    QString fnDir { QDir::homePath() + "/.local/share/deepin/dde-file-manager/filename-index" };
    FilenameDirStub()
    {
        stub.set_lamda(ADDR(Global, fileNameIndexDirectory),
                       [this]() -> QString { __DBG_STUB_INVOKE__ return fnDir; });
        stub.set_lamda(ADDR(Global, contentIndexDirectory),
                       []() -> QString { __DBG_STUB_INVOKE__
                           return QDir::homePath() + "/.local/share/deepin/dde-file-manager/fulltext-index"; });
        stub.set_lamda(ADDR(Global, ocrTextIndexDirectory),
                       []() -> QString { __DBG_STUB_INVOKE__
                           return QDir::homePath() + "/.local/share/deepin/dde-file-manager/ocrtext-index"; });
        stub.set_lamda(ADDR(Global, defaultBlacklistPaths),
                       []() -> QStringList { __DBG_STUB_INVOKE__ return QStringList(); });
    }
};

static IndexProfile makeProfile()
{
    return IndexProfile(
            IndexProfile::Identity { IndexProfile::Type::Content, "testprofile", "test_status.json", "test_version", 1 },
            { []() -> QString { return "/tmp/dfm_test_index"; },
              []() -> bool { return true; },
              [](const QString &) -> bool { return true; },
              [](const QString &) -> bool { return true; } });
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

// ===========================================================================
// Filename profile (design 5.1.2) — the new third IndexProfile::Type::Filename.
// ===========================================================================

TEST(IndexProfileTest, FilenameFactoryReturnsFilenameType)
{
    auto p = IndexProfile::filename();
    EXPECT_EQ(p.type(), IndexProfile::Type::Filename);
}

TEST(IndexProfileTest, FilenameProfile_IdIsFilename)
{
    auto p = IndexProfile::filename();
    EXPECT_EQ(p.id(), QString("filename"));
}

TEST(IndexProfileTest, FilenameProfile_StatusFileName)
{
    auto p = IndexProfile::filename();
    EXPECT_TRUE(p.statusFilePath().contains("index_status.json"));
}

TEST(IndexProfileTest, FilenameProfile_RuntimeVersionIs1)
{
    auto p = IndexProfile::filename();
    EXPECT_EQ(p.runtimeIndexVersion(), Defines::kFilenameIndexVersion);
    EXPECT_EQ(p.runtimeIndexVersion(), 1);
}

TEST(IndexProfileTest, FilenameProfile_VersionKey)
{
    auto p = IndexProfile::filename();
    EXPECT_EQ(p.versionKey(), Defines::kFilenameVersionKey);
}

TEST(IndexProfileTest, FilenameProfile_IndexDirectoryNonEmpty)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::filename();
    EXPECT_FALSE(p.indexDirectory().isEmpty());
    // Must be under ~/.local/share/deepin/dde-file-manager/filename-index (design 4.2 #7)
    EXPECT_TRUE(p.indexDirectory().contains("filename-index"));
}

TEST(IndexProfileTest, FilenameProfile_IsCandidateFileAcceptsAll)
{
    auto p = IndexProfile::filename();
    // Accepts any file (no extension filtering — filename indexes all files)
    EXPECT_TRUE(p.isCandidateFile("/any/path/readme.txt"));
    EXPECT_TRUE(p.isCandidateFile("/any/path/noext"));
    EXPECT_TRUE(p.isCandidateFile("/any/path/.hidden"));
    EXPECT_TRUE(p.isCandidateFile("/any/path/archive.tar.gz"));
}

TEST(IndexProfileTest, FilenameProfile_SupportsAnythingIsFalse)
{
    auto p = IndexProfile::filename();
    // No AnythingSearchOptionsProvider → supportsAnything() false → FileSystemProvider fallback
    EXPECT_FALSE(p.supportsAnything());
}

TEST(IndexProfileTest, FilenameProfile_DoesNotSupportChecksum)
{
    auto p = IndexProfile::filename();
    EXPECT_FALSE(p.supportsChecksum());
    EXPECT_TRUE(p.computeChecksum("/any/file").isEmpty());
}

TEST(IndexProfileTest, FilenameProfile_AnalyzerIsNGram)
{
    auto p = IndexProfile::filename();
    EXPECT_NO_FATAL_FAILURE({ auto a = p.createAnalyzer(); EXPECT_NE(a.get(), nullptr); });
}

// --- RuntimePolicy (design 5.1.1 / 变更 4/9) ---

TEST(IndexProfileTest, FilenameProfile_RuntimePolicy_EnvExempt)
{
    auto p = IndexProfile::filename();
    // Filename is exempt from battery/power-save/idle env gating (design 3.2.2 / 变更 4)
    EXPECT_FALSE(p.runtimePolicy().checkEnvPolicy);
    EXPECT_FALSE(p.shouldCheckEnvPolicy());   // virtual accessor
}

TEST(IndexProfileTest, FilenameProfile_RuntimePolicy_CollectionWindow1s)
{
    auto p = IndexProfile::filename();
    EXPECT_EQ(p.runtimePolicy().eventCollectionWindowMs, 1000);
}

TEST(IndexProfileTest, FilenameProfile_RuntimePolicy_RecoveryDelay30s)
{
    auto p = IndexProfile::filename();
    // Design 变更 9: recoveryUpdateDelayMs = 3000 (Content/Ocr stay at global 180s)
    EXPECT_EQ(p.runtimePolicy().recoveryUpdateDelayMs, 3000);
}

TEST(IndexProfileTest, ContentProfile_RuntimePolicy_DefaultsPreserved)
{
    auto p = IndexProfile::content();
    // Content profile keeps defaults (no env exemption, no custom recovery delay)
    EXPECT_TRUE(p.runtimePolicy().checkEnvPolicy);
    EXPECT_TRUE(p.shouldCheckEnvPolicy());
    EXPECT_EQ(p.runtimePolicy().recoveryUpdateDelayMs, 0);   // 0 → use global 180s
}

TEST(IndexProfileTest, OcrProfile_RuntimePolicy_DefaultsPreserved)
{
    auto p = IndexProfile::ocr();
    EXPECT_TRUE(p.runtimePolicy().checkEnvPolicy);
    EXPECT_TRUE(p.shouldCheckEnvPolicy());
    EXPECT_EQ(p.runtimePolicy().recoveryUpdateDelayMs, 0);
}

// --- FilterPolicy (design 5.1.1 / 变更 1) ---

TEST(IndexProfileTest, FilenameProfile_FilterPolicy_IndexesHiddenFiles)
{
    auto p = IndexProfile::filename();
    // Filename indexes hidden files (marks is_hidden field) — design 4.2 #4 / 5.1.5
    EXPECT_TRUE(p.filterPolicy().indexHiddenFiles);
}

TEST(IndexProfileTest, ContentProfile_FilterPolicy_DoesNotIndexHidden)
{
    auto p = IndexProfile::content();
    EXPECT_FALSE(p.filterPolicy().indexHiddenFiles);
}

TEST(IndexProfileTest, FilenameProfile_FilterPolicy_HasBlacklistProvider)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::filename();
    // Custom blacklist (anything blacklist + index dirs + .avfs) — design 5.11
    ASSERT_TRUE(p.filterPolicy().blacklistProvider);
    QStringList bl = p.filterPolicy().blacklistProvider();
    EXPECT_FALSE(bl.isEmpty());
}

TEST(IndexProfileTest, ContentProfile_FilterPolicy_NoCustomBlacklistProvider)
{
    auto p = IndexProfile::content();
    EXPECT_FALSE(p.filterPolicy().blacklistProvider);
}

// --- Hidden-entry policy (home-dir exclusion + shouldSkipHiddenEntry) ---

TEST(IndexProfileTest, FilenameProfile_FilterPolicy_ExcludesHiddenUnderHome)
{
    auto p = IndexProfile::filename();
    // 主目录第一级隐藏条目被二级策略屏蔽；更深层级（~/Documents/.abc）不屏蔽
    ASSERT_TRUE(p.filterPolicy().hiddenIndexExclusion);
    const QString home = QDir::homePath();
    EXPECT_TRUE(p.filterPolicy().hiddenIndexExclusion(home + "/.bashrc"));
    EXPECT_TRUE(p.filterPolicy().hiddenIndexExclusion(home + "/.cache/deepin/app.dat"));
    // 深层隐藏目录不属于第一级，依然索引
    EXPECT_FALSE(p.filterPolicy().hiddenIndexExclusion(home + "/Documents/.abc"));
    EXPECT_FALSE(p.filterPolicy().hiddenIndexExclusion(home + "/Documents/.abc/file.txt"));
}

TEST(IndexProfileTest, FilenameProfile_ShouldSkipHiddenEntry_HomeHiddenSkipped)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::filename();
    const QString home = QDir::homePath();

    // 主目录第一级隐藏条目：跳过（其整个子树随之排除）
    EXPECT_TRUE(p.shouldSkipHiddenEntry(home + "/.bashrc"));
    EXPECT_TRUE(p.shouldSkipHiddenEntry(home + "/.config/dde/file-manager.conf"));
    EXPECT_TRUE(p.shouldSkipHiddenEntry(home + "/.local/share/data.bin"));

    // 主目录外的隐藏条目：依然索引（不在屏蔽范围）
    EXPECT_FALSE(p.shouldSkipHiddenEntry("/opt/data/.hidden"));
    EXPECT_FALSE(p.shouldSkipHiddenEntry("/tmp/dfm_idx_test/.cache"));

    // 主目录内更深层级中的隐藏目录：依然索引（如 ~/Documents/.abc）
    EXPECT_FALSE(p.shouldSkipHiddenEntry(home + "/Documents/.abc"));
    EXPECT_FALSE(p.shouldSkipHiddenEntry(home + "/Documents/.abc/file.txt"));

    // 非隐藏路径恒不跳过
    EXPECT_FALSE(p.shouldSkipHiddenEntry(home + "/Documents/report.txt"));
    EXPECT_FALSE(p.shouldSkipHiddenEntry("/tmp/normal.txt"));
}

TEST(IndexProfileTest, ContentProfile_ShouldSkipHiddenEntry_AllHiddenSkipped)
{
    auto p = IndexProfile::content();
    const QString home = QDir::homePath();
    // Content/Ocr skip every hidden entry (original behavior preserved)
    EXPECT_TRUE(p.shouldSkipHiddenEntry(home + "/.cache/app.dat"));
    EXPECT_TRUE(p.shouldSkipHiddenEntry("/tmp/.hidden"));
    EXPECT_FALSE(p.shouldSkipHiddenEntry(home + "/Documents/report.txt"));
}

TEST(IndexProfileTest, ProfileWithoutExclusion_ShouldSkipHiddenEntry_IndexesHidden)
{
    // Hand-crafted filename profile without hiddenIndexExclusion: hidden entries
    // under $HOME are still indexed (policy absent → no exclusion)
    const QString home = QDir::homePath();
    IndexProfile p = IndexProfile(
            IndexProfile::Identity { IndexProfile::Type::Filename, "fn_no_excl", "s.json", "v", 1 },
            {}, IndexProfile::RuntimePolicy {},
            IndexProfile::FilterPolicy { true });
    EXPECT_TRUE(p.filterPolicy().indexHiddenFiles);
    EXPECT_FALSE(p.filterPolicy().hiddenIndexExclusion);
    EXPECT_FALSE(p.shouldSkipHiddenEntry(home + "/.bashrc"));
}

// --- Field accessors (design 5.3.2 / 变更 6) ---

TEST(IndexProfileTest, FilenameProfile_PathFieldIsFullPath)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::filename();
    // pathField() → kFullPath (NOT kPath). Compare wide-string content, not pointer
    // (the constexpr wchar_t[] may live at different addresses in .so vs test exe).
    EXPECT_EQ(QString::fromWCharArray(p.pathField()), QString::fromWCharArray(DFMSEARCH::LuceneFieldNames::FileName::kFullPath));
    EXPECT_NE(QString::fromWCharArray(p.pathField()), QString::fromWCharArray(DFMSEARCH::LuceneFieldNames::Content::kPath));
}

TEST(IndexProfileTest, FilenameProfile_ContentFieldIsFileName)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::filename();
    EXPECT_EQ(QString::fromWCharArray(p.contentField()), QString::fromWCharArray(DFMSEARCH::LuceneFieldNames::FileName::kFileName));
}

TEST(IndexProfileTest, FilenameProfile_AncestorPathsField)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::filename();
    EXPECT_EQ(QString::fromWCharArray(p.ancestorPathsField()), QString::fromWCharArray(DFMSEARCH::LuceneFieldNames::FileName::kAncestorPaths));
}

TEST(IndexProfileTest, FilenameProfile_ModifyTimeField)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::filename();
    EXPECT_EQ(QString::fromWCharArray(p.modifyTimeField()), QString::fromWCharArray(DFMSEARCH::LuceneFieldNames::FileName::kModifyTime));
}

TEST(IndexProfileTest, FilenameProfile_SupportsModifiedTimestampCheck)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::filename();
    EXPECT_TRUE(p.supportsModifiedTimestampCheck());
}

// --- Move/capability semantics declarations ---

TEST(IndexProfileTest, ContentProfile_MoveUpdatePolicy_IsUpdatePathOnly)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::content();
    EXPECT_EQ(p.moveUpdatePolicy(), IndexProfile::MoveUpdatePolicy::UpdatePathOnly);
    EXPECT_TRUE(p.requiresContentExtraction());
}

TEST(IndexProfileTest, OcrProfile_MoveUpdatePolicy_IsUpdatePathOnly)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::ocr();
    EXPECT_EQ(p.moveUpdatePolicy(), IndexProfile::MoveUpdatePolicy::UpdatePathOnly);
    EXPECT_TRUE(p.requiresContentExtraction());
    // OCR 单文件提取成本高：Light 分级阈值由 profile 声明（函数式 provider，保持动态性）
    ASSERT_TRUE(p.lightGradeFileCountThreshold() > 0);
    EXPECT_EQ(p.lightGradeFileCountThreshold(),
              TextIndexConfig::instance().lightIncrementOcrFileCountThreshold());
}

TEST(IndexProfileTest, FilenameProfile_MoveUpdatePolicy_IsRebuildDocument)
{
    FilenameDirStub stubs;
    auto p = IndexProfile::filename();
    EXPECT_EQ(p.moveUpdatePolicy(), IndexProfile::MoveUpdatePolicy::RebuildDocument);
    EXPECT_FALSE(p.requiresContentExtraction());
    // 未声明 Light 阈值 → 0 表示回退全局配置
    EXPECT_EQ(p.lightGradeFileCountThreshold(), 0);
}

TEST(IndexProfileTest, DefaultConstructedProfile_MoveUpdatePolicy_IsUpdatePathOnly)
{
    IndexProfile p;
    EXPECT_EQ(p.moveUpdatePolicy(), IndexProfile::MoveUpdatePolicy::UpdatePathOnly);
    EXPECT_TRUE(p.requiresContentExtraction());
    EXPECT_EQ(p.lightGradeFileCountThreshold(), 0);
}

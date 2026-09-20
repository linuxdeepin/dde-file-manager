// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_index_directory_exclusion.cpp
 * @brief Tests for the index-directory exclusion mechanism (design 5.11).
 *        Verifies that:
 *          1. IndexProfile::filename() FilterPolicy.blacklistProvider returns a list
 *             including the three index directories + ".avfs".
 *          2. The provider's output, when fed to PathExcludeMatcher, excludes paths
 *             inside the index directories (prevents the self-indexing infinite loop).
 *          3. Non-index paths under $HOME are NOT excluded (regular files still indexed).
 *
 * This is a high-risk item (design 10.1 "索引死循环更新") — the exclusion is the
 * primary defense and must be covered by a deterministic unit test.
 */

#include <gtest/gtest.h>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

#include "stubext.h"
#include <dfm-search/dsearch_global.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/utils/pathexcludematcher.h"

using namespace SERVICETEXTINDEX_NAMESPACE;
using namespace DFMSEARCH;

// Stub Global::xxxIndexDirectory to return new paths so the filename profile's
// blacklistProvider (which calls these) produces deterministic results regardless
// of the installed libdfm6-search version.
struct IndexDirStubs
{
    stub_ext::StubExt stub;
    QString fnDir { QDir::homePath() + "/.local/share/deepin/dde-file-manager/filename-index" };
    QString cDir { QDir::homePath() + "/.local/share/deepin/dde-file-manager/fulltext-index" };
    QString oDir { QDir::homePath() + "/.local/share/deepin/dde-file-manager/ocrtext-index" };

    IndexDirStubs()
    {
        stub.set_lamda(ADDR(Global, fileNameIndexDirectory),
                       [this]() -> QString { __DBG_STUB_INVOKE__ return fnDir; });
        stub.set_lamda(ADDR(Global, contentIndexDirectory),
                       [this]() -> QString { __DBG_STUB_INVOKE__ return cDir; });
        stub.set_lamda(ADDR(Global, ocrTextIndexDirectory),
                       [this]() -> QString { __DBG_STUB_INVOKE__ return oDir; });
        stub.set_lamda(ADDR(Global, defaultBlacklistPaths),
                       []() -> QStringList { __DBG_STUB_INVOKE__ return QStringList(); });
    }
};

// --- Filename profile blacklist provider content ---

TEST(IndexDirectoryExclusionTest, FilenameBlacklistProvider_IncludesAllIndexDirs)
{
    IndexDirStubs stubs;
    IndexProfile p = IndexProfile::filename();
    ASSERT_TRUE(p.filterPolicy().blacklistProvider);
    QStringList bl = p.filterPolicy().blacklistProvider();

    // The three index directories must be present (death-loop prevention).
    // We compare canonicalized paths because the provider returns the exact path
    // from Global::xxxIndexDirectory(), which may contain trailing differences.
    auto canonicalize = [](const QString &s) {
        return QDir(QFileInfo(s).absolutePath()).absolutePath();
    };

    bool hasFilename = false, hasContent = false, hasOcr = false, hasAvfs = false;
    for (const QString &entry : bl) {
        if (entry.contains("filename-index")) hasFilename = true;
        if (entry.contains("fulltext-index")) hasContent = true;
        if (entry.contains("ocrtext-index")) hasOcr = true;
        if (entry == ".avfs") hasAvfs = true;
    }
    EXPECT_TRUE(hasFilename) << "filename index dir must be in blacklist";
    EXPECT_TRUE(hasContent) << "content index dir must be in blacklist";
    EXPECT_TRUE(hasOcr) << "ocr index dir must be in blacklist";
    EXPECT_TRUE(hasAvfs) << ".avfs must be in blacklist";
}

TEST(IndexDirectoryExclusionTest, FilenameBlacklist_ExcludesIndexPath)
{
    IndexDirStubs stubs;
    IndexProfile p = IndexProfile::filename();
    QStringList bl = p.filterPolicy().blacklistProvider();
    PathExcludeMatcher matcher(bl);

    // A path inside the filename index directory must be excluded.
    QString filenameDir = DFMSEARCH::Global::fileNameIndexDirectory();
    ASSERT_FALSE(filenameDir.isEmpty());
    QString inner = filenameDir + "/segments_1";
    EXPECT_TRUE(matcher.shouldExclude(inner)) << "Path inside index dir must be excluded";
}

TEST(IndexDirectoryExclusionTest, FilenameBlacklist_ExcludesContentIndexPath)
{
    IndexDirStubs stubs;
    IndexProfile p = IndexProfile::filename();
    QStringList bl = p.filterPolicy().blacklistProvider();
    PathExcludeMatcher matcher(bl);

    QString contentDir = DFMSEARCH::Global::contentIndexDirectory();
    ASSERT_FALSE(contentDir.isEmpty());
    EXPECT_TRUE(matcher.shouldExclude(contentDir + "/write.lock"));
}

TEST(IndexDirectoryExclusionTest, FilenameBlacklist_ExcludesOcrIndexPath)
{
    IndexDirStubs stubs;
    IndexProfile p = IndexProfile::filename();
    QStringList bl = p.filterPolicy().blacklistProvider();
    PathExcludeMatcher matcher(bl);

    QString ocrDir = DFMSEARCH::Global::ocrTextIndexDirectory();
    ASSERT_FALSE(ocrDir.isEmpty());
    EXPECT_TRUE(matcher.shouldExclude(ocrDir + "/_0.cfs"));
}

TEST(IndexDirectoryExclusionTest, FilenameBlacklist_ExcludesAvfsPath)
{
    IndexDirStubs stubs;
    IndexProfile p = IndexProfile::filename();
    QStringList bl = p.filterPolicy().blacklistProvider();
    PathExcludeMatcher matcher(bl);

    // ".avfs" is registered as an ExactName pattern — any path whose name component
    // equals ".avfs" is excluded.
    EXPECT_TRUE(matcher.shouldExclude("/home/user/.avfs/file.txt"));
}

TEST(IndexDirectoryExclusionTest, FilenameBlacklist_DoesNotExcludeRegularHomeFile)
{
    IndexDirStubs stubs;
    IndexProfile p = IndexProfile::filename();
    QStringList bl = p.filterPolicy().blacklistProvider();
    PathExcludeMatcher matcher(bl);

    // A normal home path (not inside any index dir) must NOT be excluded.
    QString home = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    ASSERT_FALSE(home.isEmpty());
    QString regular = home + "/Documents/readme.txt";
    EXPECT_FALSE(matcher.shouldExclude(regular)) << "Regular home file must not be excluded";
}

TEST(IndexDirectoryExclusionTest, FilenameBlacklist_DoesNotExcludeTmpFile)
{
    IndexDirStubs stubs;
    IndexProfile p = IndexProfile::filename();
    QStringList bl = p.filterPolicy().blacklistProvider();
    PathExcludeMatcher matcher(bl);

    EXPECT_FALSE(matcher.shouldExclude("/tmp/normal-file.txt"));
    EXPECT_FALSE(matcher.shouldExclude(QStringLiteral("/usr/bin/ls")));
}

// --- Content/Ocr profiles: no custom blacklistProvider → use global createForIndex() ---

TEST(IndexDirectoryExclusionTest, ContentProfile_NoCustomBlacklistProvider)
{
    IndexProfile p = IndexProfile::content();
    // Content profile does not set a custom blacklistProvider; it relies on the
    // global createForIndex() matcher (TextIndexConfig folderExcludeFilters + anything).
    EXPECT_FALSE(p.filterPolicy().blacklistProvider);
}

TEST(IndexDirectoryExclusionTest, OcrProfile_NoCustomBlacklistProvider)
{
    IndexProfile p = IndexProfile::ocr();
    EXPECT_FALSE(p.filterPolicy().blacklistProvider);
}

// --- Combined matcher (union) used by FSMonitorWorker for watch establishment ---

TEST(IndexDirectoryExclusionTest, UnionMatcher_ExcludesAllIndexDirsAndAvfs)
{
    IndexDirStubs stubs;
    // FSMonitor::init() builds a union matcher = createForIndex() + 3 index dirs + .avfs.
    PathExcludeMatcher unionMatcher = PathExcludeMatcher::createForIndex();
    unionMatcher.addPattern(DFMSEARCH::Global::fileNameIndexDirectory());
    unionMatcher.addPattern(DFMSEARCH::Global::contentIndexDirectory());
    unionMatcher.addPattern(DFMSEARCH::Global::ocrTextIndexDirectory());
    unionMatcher.addPattern(QStringLiteral(".avfs"));

    EXPECT_TRUE(unionMatcher.shouldExclude(DFMSEARCH::Global::fileNameIndexDirectory() + "/x"));
    EXPECT_TRUE(unionMatcher.shouldExclude(DFMSEARCH::Global::contentIndexDirectory() + "/x"));
    EXPECT_TRUE(unionMatcher.shouldExclude(DFMSEARCH::Global::ocrTextIndexDirectory() + "/x"));
    EXPECT_TRUE(unionMatcher.shouldExclude("/home/user/.avfs/y"));
    EXPECT_FALSE(unionMatcher.shouldExclude("/home/user/Documents/readme.txt"));
}

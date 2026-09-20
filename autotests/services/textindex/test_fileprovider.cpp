// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fileprovider.cpp
 * @brief Unit tests for the FileProvider hierarchy (task/fileprovider.cpp)
 *        Covers FileSystemProvider, DirectFileListProvider and
 *        MixedPathListProvider constructors, traverse() execution (including
 *        the ScopeGuard cleanup lambdas) and totalCount()/name().
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QString>
#include <QStringList>

#include <dfm-search/searchresult.h>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/profile/indexprofile.h"
#include "services/textindex/profile/lowercasengramanalyzer.h"
#include "services/textindex/task/fileprovider.h"
#include "services/textindex/utils/taskstate.h"

#include <lucene++/LuceneHeaders.h>
#include <boost/shared_ptr.hpp>

using namespace SERVICETEXTINDEX_NAMESPACE;

class FileProviderTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    IndexProfile makeProfile()
    {
        return IndexProfile({ IndexProfile::Type::Content,
                              "fp_test",
                              "fp_status.json",
                              "fp_version",
                              1 },
                            { [this]() -> QString { return tmp.path(); },
                              []() -> bool { return true; },
                              [](const QString &) -> bool { return true; },
                              [](const QString &) -> bool { return true; } });
    }

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        // Create a small directory tree so traverse() opens real directories.
        QDir root(tmp.path());
        ASSERT_TRUE(root.mkpath("subdir"));
        QFile f(root.filePath("a.txt"));
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write("hello");
        f.close();
        QFile f2(root.filePath("subdir/note.txt"));
        ASSERT_TRUE(f2.open(QIODevice::WriteOnly));
        f2.close();
    }
};

TEST_F(FileProviderTest, FileSystemProvider_NameAndTotalCount)
{
    FileSystemProvider p(makeProfile(), tmp.path());
    EXPECT_EQ(p.name(), QString("FileSystemProvider"));
    EXPECT_EQ(p.totalCount(), 0);   // base default
}

TEST_F(FileProviderTest, FileSystemProvider_TraverseVisitsTree)
{
    FileSystemProvider p(makeProfile(), tmp.path());
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });
    // traverse() runs to completion over the temp tree; handler may or may not
    // fire depending on extension filtering, but the traversal body (and the
    // ScopeGuard cleanup lambda) is exercised either way.
    SUCCEED();
}

TEST_F(FileProviderTest, FileSystemProvider_TraverseStopsWhenStateStops)
{
    FileSystemProvider p(makeProfile(), tmp.path());
    TaskState state;   // not started -> isRunning() false
    QStringList visited;
    EXPECT_NO_FATAL_FAILURE({ p.traverse(state, [&visited](const QString &) {}); });
}

TEST_F(FileProviderTest, DirectFileListProvider_ConstructsAndCounts)
{
    dfmsearch::SearchResultList list;
    list.append(dfmsearch::SearchResult(tmp.path() + "/file1.txt"));
    list.append(dfmsearch::SearchResult(tmp.path() + "/file2.doc"));
    DirectFileListProvider p(list);
    EXPECT_EQ(p.name(), QString("DirectFileListProvider"));
    EXPECT_EQ(p.totalCount(), 2);
}

TEST_F(FileProviderTest, DirectFileListProvider_TraverseInvokesHandler)
{
    dfmsearch::SearchResultList list;
    list.append(dfmsearch::SearchResult(tmp.path() + "/file1.txt"));
    list.append(dfmsearch::SearchResult(tmp.path() + "/file2.doc"));
    DirectFileListProvider p(list);
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });
    EXPECT_EQ(visited.size(), 2);
}

TEST_F(FileProviderTest, DirectFileListProvider_TraverseRespectsStop)
{
    dfmsearch::SearchResultList list;
    for (int i = 0; i < 6; ++i)
        list.append(dfmsearch::SearchResult(tmp.path() + "/f" + QString::number(i) + ".txt"));
    DirectFileListProvider p(list);
    TaskState state;
    state.start();
    int n = 0;
    p.traverse(state, [&n, &state](const QString &) {
        n++;
        if (n >= 3)
            state.stop();
    });
    EXPECT_EQ(n, 3);
}

TEST_F(FileProviderTest, MixedPathListProvider_NameAndTraverse)
{
    MixedPathListProvider p(makeProfile(), { tmp.path() });
    EXPECT_EQ(p.name(), QString("MixedPathListProvider"));
    TaskState state;
    state.start();
    EXPECT_NO_FATAL_FAILURE({ p.traverse(state, [](const QString &) {}); });
}

TEST_F(FileProviderTest, MixedPathListProvider_TraverseMixedEntries)
{
    QStringList entries { tmp.path() + "/a.txt", tmp.path() + "/subdir", tmp.path() + "/missing" };
    MixedPathListProvider p(makeProfile(), entries);
    TaskState state;
    state.start();
    QStringList visited;
    EXPECT_NO_FATAL_FAILURE({ p.traverse(state, [&visited](const QString &path) { visited.append(path); }); });
}

TEST_F(FileProviderTest, MixedPathListProvider_TraverseRespectsStop)
{
    MixedPathListProvider p(makeProfile(), { tmp.path() });
    TaskState state;
    // Not started -> isRunning() false
    QStringList visited;
    EXPECT_NO_FATAL_FAILURE({ p.traverse(state, [&visited](const QString &) {}); });
}

TEST_F(FileProviderTest, MixedPathListProvider_TraverseEmptyList)
{
    MixedPathListProvider p(makeProfile(), {});
    TaskState state;
    state.start();
    int count = 0;
    p.traverse(state, [&count](const QString &) { count++; });
    EXPECT_EQ(count, 0);
}

TEST_F(FileProviderTest, MixedPathListProvider_TraverseNonExistentPaths)
{
    MixedPathListProvider p(makeProfile(), { "/nonexistent/path1", "/nonexistent/path2" });
    TaskState state;
    state.start();
    int count = 0;
    EXPECT_NO_FATAL_FAILURE({ p.traverse(state, [&count](const QString &) { count++; }); });
}

TEST_F(FileProviderTest, MixedPathListProvider_TraverseFileOnly)
{
    MixedPathListProvider p(makeProfile(), { tmp.path() + "/a.txt" });
    TaskState state;
    state.start();
    QStringList visited;
    EXPECT_NO_FATAL_FAILURE({ p.traverse(state, [&visited](const QString &path) { visited.append(path); }); });
    // a.txt exists and is a valid candidate file
    EXPECT_FALSE(visited.isEmpty());
}

TEST_F(FileProviderTest, FileSystemProvider_TraverseNonExistentRoot)
{
    FileSystemProvider p(makeProfile(), "/nonexistent/root/path");
    TaskState state;
    state.start();
    QStringList visited;
    EXPECT_NO_FATAL_FAILURE({ p.traverse(state, [&visited](const QString &path) { visited.append(path); }); });
    // Root doesn't exist, so nothing should be visited
    EXPECT_TRUE(visited.isEmpty());
}

TEST_F(FileProviderTest, DirectFileListProvider_TraverseEmptyList)
{
    dfmsearch::SearchResultList list;
    DirectFileListProvider p(list);
    EXPECT_EQ(p.totalCount(), 0);
    TaskState state;
    state.start();
    int count = 0;
    p.traverse(state, [&count](const QString &) { count++; });
    EXPECT_EQ(count, 0);
}

TEST_F(FileProviderTest, DirectFileListProvider_TraverseNotStarted)
{
    dfmsearch::SearchResultList list;
    list.append(dfmsearch::SearchResult("/some/file.txt"));
    DirectFileListProvider p(list);
    TaskState state;  // not started
    int count = 0;
    p.traverse(state, [&count](const QString &) { count++; });
    EXPECT_EQ(count, 0);
}

// ===========================================================================
// Filename profile per-policy traversal (design 5.6 / 5.1.5 / 5.11):
//   - Hidden files are indexed (not skipped) when indexHiddenFiles=true
//   - Index directories are excluded from traversal (death-loop prevention)
// ===========================================================================

class FileProviderFilenameTest : public testing::Test
{
protected:
    // Use a directory under $HOME (not /tmp, which IndexTraverseUtils::shouldSkipDirectory
    // excludes) so FileSystemProvider traversal actually visits the tree.
    QString rootPath;
    QSharedPointer<QTemporaryDir> tmp;

    IndexProfile makeFilenameProfile(bool excludeHomeHidden = false)
    {
        // Filename profile: indexHiddenFiles/indexSymlinks=true, custom blacklist
        // excludes the index dir (mirrors IndexProfile::filename() policy).
        // excludeHomeHidden mirrors IndexProfile::filename()'s secondary policy
        // (rootPath acting as the "home"): only FIRST-LEVEL hidden entries under
        // rootPath are excluded (their whole subtree is skipped); hidden dirs in
        // deeper levels (e.g. subdir/.abc) are still indexed.
        IndexProfile::FilterPolicy fp;
        fp.indexHiddenFiles = true;
        fp.indexSymlinks = true;
        if (excludeHomeHidden) {
            fp.hiddenIndexExclusion = [this](const QString &hiddenPath) {
                if (!hiddenPath.startsWith(rootPath + QLatin1Char('/')))
                    return false;
                const QString relative = QString(hiddenPath).mid(rootPath.size() + 1);
                const int slash = relative.indexOf(QLatin1Char('/'));
                const QString firstSegment = slash < 0 ? relative : relative.left(slash);
                return firstSegment.startsWith(QLatin1Char('.'));
            };
        }
        fp.blacklistProvider = [this]() -> QStringList {
            return { rootPath + "/index-dir" };   // exclude a fake index directory
        };
        IndexProfile::RuntimePolicy rp;
        rp.checkEnvPolicy = false;
        rp.eventCollectionWindowMs = 300;
        rp.recoveryUpdateDelayMs = 30000;

        return IndexProfile({ IndexProfile::Type::Filename, "fp_fn", "fp_fn_status.json",
                              "fp_fn_ver", 1 },
                            { [this]() -> QString { return rootPath; },
                              []() -> bool { return true; },
                              [](const QString &) -> bool { return true; },
                              [](const QString &) -> bool { return true; },
                              {}, {}, {},
                              []() -> boost::shared_ptr<void> {
                                  return Lucene::newLucene<LowerCaseNGramAnalyzer>(1, 2);
                              } },
                            rp, fp);
    }

    void SetUp() override
    {
        // 参考 vfsmonitor 测试：在 $HOME 下用点前缀隐藏目录模板直接创建专属测试目录，
        // 由 QTemporaryDir 析构时自动清理，不在主目录残留。
        // （不用 /tmp，IndexTraverseUtils::shouldSkipDirectory 会将其排除；
        //   旧实现先 mkpath 外层 dfm_fp_test_* 目录再建临时目录，外层目录无人清理）
        QString templatePath = QDir::homePath() + "/.dfm_fp_test_XXXXXX";
        tmp = QSharedPointer<QTemporaryDir>::create(templatePath);
        ASSERT_TRUE(tmp->isValid());
        rootPath = tmp->path();

        QDir root(rootPath);
        ASSERT_TRUE(root.mkpath("subdir"));
        ASSERT_TRUE(root.mkpath("index-dir"));
        ASSERT_TRUE(root.mkpath(".hidden-dir"));
        // 深层隐藏目录（主目录下非隐藏目录内的隐藏条目）——屏蔽策略不影响它
        ASSERT_TRUE(root.mkpath("subdir/.abc"));
        createFile("visible.txt");
        createFile(".hidden.txt");
        createFile("subdir/note.txt");
        createFile(".hidden-dir/inner.txt");
        createFile("subdir/.abc/inner.txt");
        createFile("index-dir/segment_1");   // must be excluded
    }

    void createFile(const QString &relPath, const QString &content = "x")
    {
        QFile f(rootPath + "/" + relPath);
        ASSERT_TRUE(f.open(QIODevice::WriteOnly));
        f.write(content.toUtf8());
        f.close();
    }
};

TEST_F(FileProviderFilenameTest, FileSystemProvider_TraverseIndexesHiddenFiles)
{
    // Filename profile indexes hidden files (marks is_hidden), so .hidden.txt should
    // be visited by traverse — unlike Content/Ocr which skip hidden files.
    FileSystemProvider p(makeFilenameProfile(), rootPath);
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });

    bool visitedHiddenFile = false;
    for (const QString &v : visited) {
        if (v.endsWith(".hidden.txt") || v.contains("/.hidden-dir/"))
            visitedHiddenFile = true;
    }
    EXPECT_TRUE(visitedHiddenFile) << "Filename profile must index hidden files (design 4.2 #4)";
}

TEST_F(FileProviderFilenameTest, FileSystemProvider_TraverseExcludesIndexDirectory)
{
    // The per-profile blacklist excludes index-dir; traversal must not descend into it.
    FileSystemProvider p(makeFilenameProfile(), rootPath);
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });

    for (const QString &v : visited) {
        EXPECT_FALSE(v.contains("/index-dir/"))
            << "Index directory must be excluded (death-loop prevention)";
    }
}

TEST_F(FileProviderFilenameTest, FileSystemProvider_NameIsCorrect)
{
    FileSystemProvider p(makeFilenameProfile(), rootPath);
    EXPECT_EQ(p.name(), QString("FileSystemProvider"));
}

TEST_F(FileProviderFilenameTest, FileSystemProvider_TraverseSkipsFirstLevelHiddenUnderHome)
{
    // 二级策略（IndexProfile::filename() 的 hiddenIndexExclusion）：
    // 仅屏蔽"主目录"第一级隐藏条目（rootPath 模拟主目录，rootPath/.hidden.txt、
    // rootPath/.hidden-dir 整棵子树跳过）；更深层级中的隐藏目录
    // （subdir/.abc，rootPath 第一级非隐藏目录内的隐藏条目）依然索引。
    FileSystemProvider p(makeFilenameProfile(true), rootPath);
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });

    for (const QString &v : visited) {
        EXPECT_FALSE(v.endsWith("/.hidden.txt"))
            << "First-level hidden file must be skipped by the exclusion policy";
        EXPECT_FALSE(v.contains("/.hidden-dir/"))
            << "First-level hidden directory must not be traversed";
    }

    auto contains = [&visited](const QString &suffix) {
        for (const QString &v : visited) {
            if (v.endsWith(suffix))
                return true;
        }
        return false;
    };
    // 第一级非隐藏条目正常索引
    EXPECT_TRUE(contains("/visible.txt"));
    EXPECT_TRUE(contains("/note.txt"));
    // 更深层级中的隐藏目录依然索引（屏蔽范围只到第一级）
    EXPECT_TRUE(contains("subdir/.abc/inner.txt"))
        << "Hidden entries in deeper levels must still be indexed";
}

TEST_F(FileProviderFilenameTest, MixedPathListProvider_TraverseSkipsFirstLevelHiddenUnderHome)
{
    // MixedPathListProvider 与 FileSystemProvider 的隐藏条目判定保持一致：
    // 第一级隐藏条目跳过，深层隐藏目录（subdir/.abc）照常处理
    auto profile = makeFilenameProfile(true);
    QStringList entries { rootPath + "/.hidden.txt", rootPath + "/.hidden-dir",
                          rootPath + "/visible.txt", rootPath + "/subdir" };
    MixedPathListProvider p(profile, entries);
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });

    for (const QString &v : visited) {
        EXPECT_FALSE(v.endsWith("/.hidden.txt"))
            << "First-level hidden file must be skipped per exclusion policy";
        EXPECT_FALSE(v.contains("/.hidden-dir/"))
            << "First-level hidden directory subtree must be skipped per exclusion policy";
    }

    bool deepHiddenVisited = false;
    for (const QString &v : visited) {
        if (v.endsWith("subdir/.abc/inner.txt"))
            deepHiddenVisited = true;
    }
    EXPECT_TRUE(deepHiddenVisited)
        << "Hidden entries in deeper levels must still be indexed";
}
TEST_F(FileProviderFilenameTest, MixedPathListProvider_FilenameExcludesIndexDir)
{
    // MixedPathListProvider used for incremental updates (directory changes).
    // Verify index-dir entries are not traversed.
    MixedPathListProvider p(makeFilenameProfile(), { rootPath });
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });

    for (const QString &v : visited) {
        EXPECT_FALSE(v.contains("/index-dir/"))
            << "MixedPathListProvider must also exclude index dirs";
    }
}

TEST_F(FileProviderFilenameTest, MixedPathListProvider_FilenameIncludesHiddenFiles)
{
    // Provide a hidden file explicitly in the path list; it should be visited
    // (filename indexes hidden files, does not filter them).
    QStringList entries { rootPath + "/.hidden.txt", rootPath + "/subdir" };
    MixedPathListProvider p(makeFilenameProfile(), entries);
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });

    bool foundHidden = false;
    for (const QString &v : visited) {
        if (v.endsWith("/.hidden.txt"))
            foundHidden = true;
    }
    EXPECT_TRUE(foundHidden) << "Explicitly-listed hidden file must be visited by filename profile";
}

// --- Content profile: hidden files filtered (behavior preserved) ---

TEST_F(FileProviderFilenameTest, FileSystemProvider_ContentProfileSkipsHiddenFiles)
{
    // Content profile (indexHiddenFiles=false) must continue to skip hidden files
    // after the FSMonitor/fsevent filtering refactor (regression check, design 变更 2).
    IndexProfile contentProfile({ IndexProfile::Type::Content, "fp_content", "fp_c_status.json",
                                  "fp_c_ver", 1 },
                                { [this]() -> QString { return rootPath; },
                                  []() -> bool { return true; },
                                  [](const QString &) -> bool { return true; },
                                  [](const QString &) -> bool { return true; } });
    FileSystemProvider p(contentProfile, rootPath);
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });

    for (const QString &v : visited) {
        EXPECT_FALSE(v.endsWith(".hidden.txt"))
            << "Content profile must still skip hidden files";
        EXPECT_FALSE(v.contains("/.hidden-dir/"))
            << "Content profile must still skip files in hidden dirs";
    }
}

// --- Symlink entries: link itself indexed (filename profile), never followed ---

TEST_F(FileProviderFilenameTest, FileSystemProvider_TraverseIndexesSymlinkEntriesWithoutFollowing)
{
    // 设计契约：link 条目本身入索引（file link / dir link / dangling link），
    // 但绝不跟随——dir link 内部内容不经链接路径入索引（防索引环）。
    ASSERT_TRUE(QFile::link(rootPath + "/visible.txt", rootPath + "/link_file.txt"));
    ASSERT_TRUE(QFile::link(rootPath + "/subdir", rootPath + "/link_dir"));
    ASSERT_TRUE(QFile::link(rootPath + "/no_such_target.txt", rootPath + "/link_dangling.txt"));

    FileSystemProvider p(makeFilenameProfile(), rootPath);
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });

    EXPECT_TRUE(visited.contains(rootPath + "/link_file.txt"))
        << "File symlink entry must be indexed by filename profile";
    EXPECT_TRUE(visited.contains(rootPath + "/link_dir"))
        << "Directory symlink entry must be indexed by filename profile";
    EXPECT_TRUE(visited.contains(rootPath + "/link_dangling.txt"))
        << "Dangling symlink entry must be indexed by filename profile";

    // 真实目标内容仍按真实路径入索引
    EXPECT_TRUE(visited.contains(rootPath + "/subdir/note.txt"));

    // dir link 内部内容绝不经链接路径入索引
    EXPECT_FALSE(visited.contains(rootPath + "/link_dir/note.txt"))
        << "Contents must not be indexed through the dir symlink path";

    // link 不改变真实目录的遍历（无重复访问/环）
    EXPECT_EQ(visited.count(rootPath + "/subdir/note.txt"), 1);
}

TEST_F(FileProviderFilenameTest, MixedPathListProvider_TraverseIndexesSymlinkEntriesWithoutFollowing)
{
    ASSERT_TRUE(QFile::link(rootPath + "/visible.txt", rootPath + "/link_file.txt"));
    ASSERT_TRUE(QFile::link(rootPath + "/subdir", rootPath + "/link_dir"));
    ASSERT_TRUE(QFile::link(rootPath + "/no_such_target.txt", rootPath + "/link_dangling.txt"));

    // FileList 任务路径：dangling link 的 exists()/isFile() 均为 false，
    // 必须作为 link 条目被收录而非按"路径不存在"丢弃
    MixedPathListProvider p(makeFilenameProfile(),
                            { rootPath + "/link_file.txt", rootPath + "/link_dir",
                              rootPath + "/link_dangling.txt", rootPath + "/subdir/note.txt" });
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });

    EXPECT_TRUE(visited.contains(rootPath + "/link_file.txt"));
    EXPECT_TRUE(visited.contains(rootPath + "/link_dir"))
        << "Directory symlink in the list must be indexed as an entry";
    EXPECT_TRUE(visited.contains(rootPath + "/link_dangling.txt"))
        << "Dangling symlink must be indexed as an entry";
    EXPECT_TRUE(visited.contains(rootPath + "/subdir/note.txt"));
    EXPECT_FALSE(visited.contains(rootPath + "/link_dir/note.txt"))
        << "Dir symlink must never be traversed";
}

TEST_F(FileProviderFilenameTest, FileSystemProvider_ContentProfileSkipsSymlinks)
{
    // Content profile（indexSymlinks=false）行为不变：link 条目照旧被排除
    ASSERT_TRUE(QFile::link(rootPath + "/visible.txt", rootPath + "/link_file.txt"));
    ASSERT_TRUE(QFile::link(rootPath + "/subdir", rootPath + "/link_dir"));

    IndexProfile contentProfile({ IndexProfile::Type::Content, "fp_content2", "fp_c2_status.json",
                                  "fp_c2_ver", 1 },
                                { [this]() -> QString { return rootPath; },
                                  []() -> bool { return true; },
                                  [](const QString &) -> bool { return true; },
                                  [](const QString &) -> bool { return true; } });
    FileSystemProvider p(contentProfile, rootPath);
    TaskState state;
    state.start();
    QStringList visited;
    p.traverse(state, [&visited](const QString &path) { visited.append(path); });

    for (const QString &v : visited) {
        EXPECT_NE(v, rootPath + "/link_file.txt")
            << "Content profile must still skip symlink entries";
        EXPECT_NE(v, rootPath + "/link_dir")
            << "Content profile must still skip symlink entries";
    }
}

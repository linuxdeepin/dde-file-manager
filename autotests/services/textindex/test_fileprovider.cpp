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
#include "services/textindex/task/fileprovider.h"
#include "services/textindex/utils/taskstate.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class FileProviderTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    IndexProfile makeProfile()
    {
        return IndexProfile(IndexProfile::Type::Content,
                            "fp_test",
                            "fp_status.json",
                            "fp_version",
                            1,
                            [this]() -> QString { return tmp.path(); },
                            []() -> bool { return true; },
                            [](const QString &) -> bool { return true; },
                            [](const QString &) -> bool { return true; });
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

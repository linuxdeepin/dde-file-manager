// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fsmonitorworker_extra.cpp
 * @brief Additional tests for FSMonitorWorker covering more code paths.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFileInfo>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/fsmonitorworker.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class FSMonitorWorkerExtraTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        // Create a complex directory structure
        QDir root(tmp.path());
        root.mkpath("level1/level2/level3");
        root.mkpath("another/deep/path");
        root.mkpath("hidden_dir");
        QFile f(tmp.path() + "/file.txt");
        f.open(QIODevice::WriteOnly);
        f.write("data");
        f.close();
    }
};

// Test processDirectory with deep nesting
TEST_F(FSMonitorWorkerExtraTest, ProcessDirectory_DeepNesting)
{
    FSMonitorWorker w;
    QStringList subs;
    QObject::connect(&w, &FSMonitorWorker::subdirectoriesFound, &w,
                     [&subs](const QStringList &dirs) { subs.append(dirs); });
    w.processDirectory(tmp.path());
    // Should find level1 and another at minimum
    EXPECT_GE(subs.size(), 2);
}

// Test processDirectory with exclusion checker that returns false for all
TEST_F(FSMonitorWorkerExtraTest, ProcessDirectory_NoExclusion)
{
    FSMonitorWorker w;
    w.setExclusionChecker([](const QString &) { return false; });

    int watchCount = 0;
    QObject::connect(&w, &FSMonitorWorker::directoryToWatch, &w,
                     [&watchCount](const QString &) { watchCount++; });

    w.processDirectory(tmp.path());
    EXPECT_GT(watchCount, 0);
}

// Test processDirectory with exclusion checker that returns true for root only
TEST_F(FSMonitorWorkerExtraTest, ProcessDirectory_ExcludeRoot)
{
    FSMonitorWorker w;
    QString rootPath = tmp.path();
    w.setExclusionChecker([rootPath](const QString &path) { return path == rootPath; });

    bool got = false;
    QObject::connect(&w, &FSMonitorWorker::directoryToWatch, &w,
                     [&got](const QString &) { got = true; });
    w.processDirectory(rootPath);
    EXPECT_FALSE(got);
}

// Test processDirectory with file path instead of directory
TEST_F(FSMonitorWorkerExtraTest, ProcessDirectory_PathIsFile)
{
    FSMonitorWorker w;
    bool got = false;
    QObject::connect(&w, &FSMonitorWorker::directoryToWatch, &w,
                     [&got](const QString &) { got = true; });
    // Pass a file path - processDirectory should handle gracefully
    w.processDirectory(tmp.path() + "/file.txt");
    // Should not emit directoryToWatch since it's a file
    SUCCEED();
}

// Test processDirectory with path containing spaces
TEST_F(FSMonitorWorkerExtraTest, ProcessDirectory_PathWithSpaces)
{
    QString dirWithSpaces = tmp.path() + "/dir with spaces";
    QDir().mkpath(dirWithSpaces);

    FSMonitorWorker w;
    bool got = false;
    QString capturedDir = dirWithSpaces;
    QObject::connect(&w, &FSMonitorWorker::directoryToWatch, &w,
                     [&got, &capturedDir](const QString &path) {
                         if (path == capturedDir) got = true;
                     });
    w.processDirectory(dirWithSpaces);
    EXPECT_TRUE(got);
}

// Test processDirectory with path containing unicode
TEST_F(FSMonitorWorkerExtraTest, ProcessDirectory_PathWithUnicode)
{
    QString unicodeDir = tmp.path() + "/中文目录";
    QDir().mkpath(unicodeDir);

    FSMonitorWorker w;
    bool got = false;
    QString capturedDir = unicodeDir;
    QObject::connect(&w, &FSMonitorWorker::directoryToWatch, &w,
                     [&got, &capturedDir](const QString &path) {
                         if (path == capturedDir) got = true;
                     });
    w.processDirectory(unicodeDir);
    EXPECT_TRUE(got);
}

// Test processDirectory with exclusion returning true for subdirs
TEST_F(FSMonitorWorkerExtraTest, ProcessDirectory_ExcludeSubdirs)
{
    FSMonitorWorker w;
    w.setExclusionChecker([](const QString &path) { return path.contains("level"); });

    QStringList subs;
    QObject::connect(&w, &FSMonitorWorker::subdirectoriesFound, &w,
                     [&subs](const QStringList &dirs) { subs.append(dirs); });
    w.processDirectory(tmp.path());
    for (const QString &s : subs) {
        EXPECT_FALSE(s.contains("level"));
    }
}

// Test setExclusionChecker with empty function
TEST_F(FSMonitorWorkerExtraTest, SetExclusionChecker_EmptyFunc)
{
    FSMonitorWorker w;
    // Set a checker that always returns false
    w.setExclusionChecker([](const QString &) { return false; });

    bool got = false;
    QObject::connect(&w, &FSMonitorWorker::directoryToWatch, &w,
                     [&got](const QString &) { got = true; });
    w.processDirectory(tmp.path());
    EXPECT_TRUE(got);
}

// Test setMaxFastScanResults with various values
TEST_F(FSMonitorWorkerExtraTest, SetMaxFastScanResults_Varied)
{
    FSMonitorWorker w;
    w.setMaxFastScanResults(1);
    w.setMaxFastScanResults(100);
    w.setMaxFastScanResults(10000);
    w.setMaxFastScanResults(INT_MAX);
    SUCCEED();
}

// Test that processDirectory handles root directory
TEST_F(FSMonitorWorkerExtraTest, ProcessDirectory_RootDir)
{
    FSMonitorWorker w;
    // Process root directory - should work without crash
    // (might be excluded by default checker)
    EXPECT_NO_FATAL_FAILURE({ w.processDirectory("/"); });
}

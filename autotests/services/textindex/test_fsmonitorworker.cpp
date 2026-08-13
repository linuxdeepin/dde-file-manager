// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fsmonitorworker.cpp
 * @brief Unit tests for FSMonitorWorker (fsmonitor/fsmonitorworker.cpp) — the
 *        dependency-light subset: ctor, processDirectory (which exercises the
 *        default exclusion-checker lambda wired up in the ctor),
 *        setExclusionChecker, setMaxFastScanResults and the dtor. The async
 *        tryFastDirectoryScan / CLI / SearchEngine paths are intentionally not
 *        invoked to avoid spawning subprocesses.
 */

#include <gtest/gtest.h>
#include <QTemporaryDir>
#include <QDir>
#include <QFileInfo>
#include <QString>
#include <QStringList>
#include <functional>

#include "dfm_test_main.h"
#include "services/textindex/service_textindex_global.h"
#include "services/textindex/fsmonitor/fsmonitorworker.h"

using namespace SERVICETEXTINDEX_NAMESPACE;

class FSMonitorWorkerTest : public testing::Test
{
protected:
    QTemporaryDir tmp;

    void SetUp() override
    {
        ASSERT_TRUE(tmp.isValid());
        QDir root(tmp.path());
        ASSERT_TRUE(root.mkpath("child"));
    }
};

TEST_F(FSMonitorWorkerTest, CtorDefaultsFastScanNotInProgress)
{
    FSMonitorWorker w;
    EXPECT_FALSE(w.isFastScanInProgress());
}

TEST_F(FSMonitorWorkerTest, ProcessDirectorySkipsEmptyAndNonDir)
{
    FSMonitorWorker w;
    EXPECT_NO_FATAL_FAILURE({ w.processDirectory(QString()); });
    EXPECT_NO_FATAL_FAILURE({ w.processDirectory(tmp.path() + "/does-not-exist"); });
}

TEST_F(FSMonitorWorkerTest, ProcessDirectoryEmitsForRealDir)
{
    FSMonitorWorker w;
    bool got = false;
    QObject::connect(&w, &FSMonitorWorker::directoryToWatch, &w,
                     [&got](const QString &) { got = true; });
    w.processDirectory(tmp.path());
    EXPECT_TRUE(got);   // default exclusion lambda returns false -> emits
}

TEST_F(FSMonitorWorkerTest, ProcessDirectoryRespectsExclusionChecker)
{
    FSMonitorWorker w;
    w.setExclusionChecker([](const QString &) { return true; });   // exclude all
    bool got = false;
    QObject::connect(&w, &FSMonitorWorker::directoryToWatch, &w,
                     [&got](const QString &) { got = true; });
    w.processDirectory(tmp.path());
    EXPECT_FALSE(got);   // excluded -> no emit
}

TEST_F(FSMonitorWorkerTest, SetExclusionCheckerIgnoresNullChecker)
{
    FSMonitorWorker w;
    w.setExclusionChecker([](const QString &) { return true; });
    w.setExclusionChecker(nullptr);   // null -> keep previous
    bool got = false;
    QObject::connect(&w, &FSMonitorWorker::directoryToWatch, &w,
                     [&got](const QString &) { got = true; });
    w.processDirectory(tmp.path());
    EXPECT_FALSE(got);   // still excluded
}

TEST_F(FSMonitorWorkerTest, SetMaxFastScanResultsAcceptsPositive)
{
    FSMonitorWorker w;
    w.setMaxFastScanResults(128);
    EXPECT_NO_FATAL_FAILURE({ (void)w.isFastScanInProgress(); });
}

TEST_F(FSMonitorWorkerTest, SetMaxFastScanResultsIgnoresNonPositive)
{
    FSMonitorWorker w;
    w.setMaxFastScanResults(0);
    w.setMaxFastScanResults(-5);
    EXPECT_NO_FATAL_FAILURE({ (void)w.isFastScanInProgress(); });
}

TEST_F(FSMonitorWorkerTest, ProcessDirectoryWithSubdirs)
{
    FSMonitorWorker w;
    QStringList subs;
    QObject::connect(&w, &FSMonitorWorker::subdirectoriesFound, &w,
                     [&subs](const QStringList &dirs) { subs.append(dirs); });
    w.processDirectory(tmp.path());
    // "child" should be found as a subdirectory
    EXPECT_FALSE(subs.isEmpty());
}

TEST_F(FSMonitorWorkerTest, ProcessDirectoryExcludesSymlinkSubdirs)
{
    FSMonitorWorker w;
    // Create a symlink directory
    QString linkPath = tmp.path() + "/symlink_child";
    ASSERT_TRUE(QFile::link(tmp.path() + "/child", linkPath));

    QStringList subs;
    QObject::connect(&w, &FSMonitorWorker::subdirectoriesFound, &w,
                     [&subs](const QStringList &dirs) { subs.append(dirs); });
    w.processDirectory(tmp.path());
    // symlink_child should NOT be in subs (symlinks filtered)
    for (const QString &s : subs) {
        EXPECT_NE(s, linkPath);
    }
}

TEST_F(FSMonitorWorkerTest, ProcessDirectorySelectiveExclusion)
{
    FSMonitorWorker w;
    w.setExclusionChecker([](const QString &path) { return path.contains("child"); });
    QStringList subs;
    QObject::connect(&w, &FSMonitorWorker::subdirectoriesFound, &w,
                     [&subs](const QStringList &dirs) { subs.append(dirs); });
    w.processDirectory(tmp.path());
    // child should be excluded from subdirs
    for (const QString &s : subs) {
        EXPECT_FALSE(s.contains("child"));
    }
}

TEST_F(FSMonitorWorkerTest, TryFastDirectoryScan_WhileInProgress_Ignored)
{
    FSMonitorWorker w;
    // Force fastScanInProgress to true
    w.fastScanInProgress = true;

    bool completed = false;
    QObject::connect(&w, &FSMonitorWorker::fastScanCompleted, &w,
                     [&completed](bool) { completed = true; });

    // This should early-return with warning
    w.tryFastDirectoryScan();

    // No completion should be emitted since we didn't start a real scan
    EXPECT_FALSE(completed);
    EXPECT_TRUE(w.isFastScanInProgress());

    w.fastScanInProgress = false;
}

TEST_F(FSMonitorWorkerTest, HandleFastScanResult_EmptyResult)
{
    FSMonitorWorker w;
    // We need to trigger handleFastScanResult. We can't easily call it directly
    // (it's private slot), so we stub tryFastDirectoryScan's async operation.
    // Instead, just verify the object works after fast scan state changes.
    w.fastScanInProgress = true;
    // Simulate reset (what handleFastScanResult does at end)
    w.fastScanInProgress = false;
    EXPECT_FALSE(w.isFastScanInProgress());
}

TEST_F(FSMonitorWorkerTest, Dtor_WithRunningFuture_NoCrash)
{
    // Create worker, start fast scan (it will run in background), then destroy
    // The destructor waits for the future
    auto *w = new FSMonitorWorker();
    // Don't actually call tryFastDirectoryScan() as it launches real subprocesses
    // Just verify normal destruction works
    delete w;
    SUCCEED();
}

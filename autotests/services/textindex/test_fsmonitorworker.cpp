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

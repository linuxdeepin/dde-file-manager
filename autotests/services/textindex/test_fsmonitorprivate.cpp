// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fsmonitorprivate.cpp
 * @brief Unit tests for FSMonitorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/fsmonitor.h"

#include <QTest>

using namespace src;

class FSMonitorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FSMonitorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FSMonitorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FSMonitorPrivateTest, handleDirectoriesBatch)
{
    // Test method: void handleDirectoriesBatch((const QStringList &paths))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->handleDirectoriesBatch(_arg0));
}

TEST_F(FSMonitorPrivateTest, handleFileDeleted)
{
    // Test method: void handleFileDeleted((const QString &path, const QString &name))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileDeleted(_arg0, _arg1));
}

TEST_F(FSMonitorPrivateTest, init)
{
    // Test method: bool init((const QStringList &rootPaths))
    QStringList _arg0{};
    auto result = obj->init(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FSMonitorPrivateTest, removeWatchForDirectory)
{
    // Test method: void removeWatchForDirectory((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeWatchForDirectory(_arg0));
}

TEST_F(FSMonitorPrivateTest, setupWorkerThread)
{
    // Test method: void setupWorkerThread(())
    EXPECT_NO_FATAL_FAILURE(obj->setupWorkerThread());
}

TEST_F(FSMonitorPrivateTest, shouldExcludePath)
{
    // Test method: bool shouldExcludePath((const QString &path))
    QString _arg0{};
    auto result = obj->shouldExcludePath(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FSMonitorPrivateTest, showHidden)
{
    // Test bool getter: showHidden()
    bool result = obj->showHidden();
    EXPECT_FALSE(result);

}

TEST_F(FSMonitorPrivateTest, startMonitoring)
{
    // Test bool getter: startMonitoring()
    bool result = obj->startMonitoring();
    EXPECT_FALSE(result);

}

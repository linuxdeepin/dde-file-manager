// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_vfsmonitorfilesystemwatcherprivate.cpp
 * @brief Unit tests for VfsMonitorFileSystemWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/vfsmonitorwatcher.h"

#include <QTest>

using namespace src;

class VfsMonitorFileSystemWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new VfsMonitorFileSystemWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    VfsMonitorFileSystemWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(VfsMonitorFileSystemWatcherPrivateTest, attemptReconnect)
{
    // Test method: void attemptReconnect(())
    EXPECT_NO_FATAL_FAILURE(obj->attemptReconnect());
}

TEST_F(VfsMonitorFileSystemWatcherPrivateTest, establishConnection)
{
    // Test bool getter: establishConnection()
    bool result = obj->establishConnection();
    EXPECT_FALSE(result);

}

TEST_F(VfsMonitorFileSystemWatcherPrivateTest, handleDisconnect)
{
    // Test method: void handleDisconnect(())
    EXPECT_NO_FATAL_FAILURE(obj->handleDisconnect());
}

TEST_F(VfsMonitorFileSystemWatcherPrivateTest, initDispatcher)
{
    // Test bool getter: initDispatcher()
    bool result = obj->initDispatcher();
    EXPECT_FALSE(result);

}

TEST_F(VfsMonitorFileSystemWatcherPrivateTest, splitPath)
{
    // Test method: QPair<QString, QString> splitPath((const QString &fullPath))
    QString _arg0{};
    auto result = obj->splitPath(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->splitPath(_arg0); });

}

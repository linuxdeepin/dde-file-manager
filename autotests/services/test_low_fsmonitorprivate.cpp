// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_fsmonitorprivate.cpp
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

TEST_F(FSMonitorPrivateTest, addDirectoryRecursively)
{
    // Test method: void addDirectoryRecursively((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->addDirectoryRecursively(_arg0));
}

TEST_F(FSMonitorPrivateTest, addWatchForDirectory)
{
    // Test method: bool addWatchForDirectory((const QString &path))
    QString _arg0{};
    auto result = obj->addWatchForDirectory(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FSMonitorPrivateTest, getMaxUserWatches)
{
    // Test getter: int getMaxUserWatches()
    auto result = obj->getMaxUserWatches();
    EXPECT_EQ(result, 0);

}

TEST_F(FSMonitorPrivateTest, handleFastScanCompleted)
{
    // Test method: void handleFastScanCompleted((bool success))
    EXPECT_NO_FATAL_FAILURE(obj->handleFastScanCompleted(false));
}

TEST_F(FSMonitorPrivateTest, handleFileClosed)
{
    // Test method: void handleFileClosed((const QString &path, const QString &name))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileClosed(_arg0, _arg1));
}

TEST_F(FSMonitorPrivateTest, handleFileCreated)
{
    // Test method: void handleFileCreated((const QString &path, const QString &name))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileCreated(_arg0, _arg1));
}

TEST_F(FSMonitorPrivateTest, handleFileMoved)
{
    // Test method: void handleFileMoved((const QString &fromPath, const QString &fromName,
                                       const QString &toPath, const QString &toName))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileMoved(_arg0, _arg1, _arg2, _arg3));
}

TEST_F(FSMonitorPrivateTest, isDirectory)
{
    // Test method: bool isDirectory((const QString &path, const QString &name))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->isDirectory(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FSMonitorPrivateTest, isSymbolicLink)
{
    // Test method: bool isSymbolicLink((const QString &path))
    QString _arg0{};
    auto result = obj->isSymbolicLink(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FSMonitorPrivateTest, isWithinWatchLimit)
{
    // Test bool getter: isWithinWatchLimit()
    bool result = obj->isWithinWatchLimit();
    EXPECT_FALSE(result);

}

TEST_F(FSMonitorPrivateTest, setupVfsMonitorConnections)
{
    // Test method: void setupVfsMonitorConnections(())
    EXPECT_NO_FATAL_FAILURE(obj->setupVfsMonitorConnections());
}

TEST_F(FSMonitorPrivateTest, setupWatcherConnections)
{
    // Test method: void setupWatcherConnections(())
    EXPECT_NO_FATAL_FAILURE(obj->setupWatcherConnections());
}

TEST_F(FSMonitorPrivateTest, stopMonitoring)
{
    // Test method: void stopMonitoring(())
    EXPECT_NO_FATAL_FAILURE(obj->stopMonitoring());
}

TEST_F(FSMonitorPrivateTest, travelRootDirectories)
{
    // Test method: void travelRootDirectories(())
    EXPECT_NO_FATAL_FAILURE(obj->travelRootDirectories());
}

TEST_F(FSMonitorPrivateTest, FSMonitorPrivate_Destructor)
{
    // Test method:  ~FSMonitorPrivate(())
    EXPECT_NO_FATAL_FAILURE({ FSMonitorPrivate *tmp = new FSMonitorPrivate(); delete tmp; });
}

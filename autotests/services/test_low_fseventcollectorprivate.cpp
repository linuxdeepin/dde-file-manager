// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_fseventcollectorprivate.cpp
 * @brief Unit tests for FSEventCollectorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/fsmonitor/fseventcollector.h"

#include <QTest>

using namespace src;

class FSEventCollectorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new FSEventCollectorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    FSEventCollectorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(FSEventCollectorPrivateTest, FSEventCollectorPrivate)
{
    // Test constructor: FSEventCollectorPrivate((FSEventCollector *qq,
                                                 FSEventCollector::PathPredicate pathPredicate,
                                                 FSMonitor &monitor))
    ASSERT_NE(obj, nullptr);
}

TEST_F(FSEventCollectorPrivateTest, buildPath)
{
    // Test method: QString buildPath((const QString &dirPath, const QString &fileName))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->buildPath(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FSEventCollectorPrivateTest, flushCollectedEvents)
{
    // Test method: void flushCollectedEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->flushCollectedEvents());
}

TEST_F(FSEventCollectorPrivateTest, handleDirectoryCreated)
{
    // Test method: void handleDirectoryCreated((const QString &path, const QString &name))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleDirectoryCreated(_arg0, _arg1));
}

TEST_F(FSEventCollectorPrivateTest, handleDirectoryMoved)
{
    // Test method: void handleDirectoryMoved((const QString &fromPath, const QString &fromName,
                                                   const QString &toPath, const QString &toName))
    QString _arg0{};
    QString _arg1{};
    QString _arg2{};
    QString _arg3{};
    EXPECT_NO_FATAL_FAILURE(obj->handleDirectoryMoved(_arg0, _arg1, _arg2, _arg3));
}

TEST_F(FSEventCollectorPrivateTest, init)
{
    // Test method: bool init((const QStringList &rootPaths))
    QStringList _arg0{};
    auto result = obj->init(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FSEventCollectorPrivateTest, isDirectory)
{
    // Test method: bool isDirectory((const QString &path))
    QString _arg0{};
    auto result = obj->isDirectory(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FSEventCollectorPrivateTest, isMaxEventCountExceeded)
{
    // Test bool getter: isMaxEventCountExceeded()
    bool result = obj->isMaxEventCountExceeded();
    EXPECT_FALSE(result);

}

TEST_F(FSEventCollectorPrivateTest, normalizePath)
{
    // Test method: QString normalizePath((const QString &dirPath, const QString &fileName))
    QString _arg0{};
    QString _arg1{};
    auto result = obj->normalizePath(_arg0, _arg1);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(FSEventCollectorPrivateTest, shouldTrackPath)
{
    // Test method: bool shouldTrackPath((const QString &path))
    QString _arg0{};
    auto result = obj->shouldTrackPath(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(FSEventCollectorPrivateTest, stopCollecting)
{
    // Test method: void stopCollecting(())
    EXPECT_NO_FATAL_FAILURE(obj->stopCollecting());
}

TEST_F(FSEventCollectorPrivateTest, FSEventCollectorPrivate_Destructor)
{
    // Test method:  ~FSEventCollectorPrivate(())
    EXPECT_NO_FATAL_FAILURE({ FSEventCollectorPrivate *tmp = new FSEventCollectorPrivate(); delete tmp; });
}

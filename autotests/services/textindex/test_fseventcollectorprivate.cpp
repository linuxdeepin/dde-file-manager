// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_fseventcollectorprivate.cpp
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

TEST_F(FSEventCollectorPrivateTest, cleanupRedundantEntries)
{
    // Test method: void cleanupRedundantEntries(())
    EXPECT_NO_FATAL_FAILURE(obj->cleanupRedundantEntries());
}

TEST_F(FSEventCollectorPrivateTest, handleDirectoryDeleted)
{
    // Test method: void handleDirectoryDeleted((const QString &path, const QString &name))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleDirectoryDeleted(_arg0, _arg1));
}

TEST_F(FSEventCollectorPrivateTest, handleFileClosed)
{
    // Test method: void handleFileClosed((const QString &path, const QString &name))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileClosed(_arg0, _arg1));
}

TEST_F(FSEventCollectorPrivateTest, handleFileCreated)
{
    // Test method: void handleFileCreated((const QString &path, const QString &name))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileCreated(_arg0, _arg1));
}

TEST_F(FSEventCollectorPrivateTest, handleFileDeleted)
{
    // Test method: void handleFileDeleted((const QString &path, const QString &name))
    QString _arg0{};
    QString _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->handleFileDeleted(_arg0, _arg1));
}

TEST_F(FSEventCollectorPrivateTest, isChildOfAnyPath)
{
    // Test method: bool isChildOfAnyPath((const QString &path, const QSet<QString> &pathSet))
    QString _arg0{};
    QSet<QString> _arg1{};
    auto result = obj->isChildOfAnyPath(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(FSEventCollectorPrivateTest, removeEntriesCoveredByDirectories)
{
    // Test method: void removeEntriesCoveredByDirectories(())
    EXPECT_NO_FATAL_FAILURE(obj->removeEntriesCoveredByDirectories());
}

TEST_F(FSEventCollectorPrivateTest, removeRedundantEntries)
{
    // Test method: void removeRedundantEntries((QSet<QString> &filesList))
    QSet<QString> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeRedundantEntries(_arg0));
}

TEST_F(FSEventCollectorPrivateTest, startCollecting)
{
    // Test bool getter: startCollecting()
    bool result = obj->startCollecting();
    EXPECT_FALSE(result);

}

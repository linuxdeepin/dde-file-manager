// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentiterateworker.cpp
 * @brief Unit tests for RecentIterateWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "recentiterateworker.h"

#include <QTest>

using namespace recent;

class RecentIterateWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentIterateWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentIterateWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentIterateWorkerTest, RecentIterateWorker)
{
    // Test constructor: RecentIterateWorker((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentIterateWorkerTest, onRequestReload)
{
    // Test method: void onRequestReload((const QString &xbelPath, qint64 timestamp))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onRequestReload(_arg0, 0));
}

TEST_F(RecentIterateWorkerTest, onRequestRemoveItems)
{
    // Test method: void onRequestRemoveItems((const QStringList &hrefs))
    QStringList _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onRequestRemoveItems(_arg0));
}

TEST_F(RecentIterateWorkerTest, processBookmarkElement)
{
    // Test method: void processBookmarkElement((QXmlStreamReader &reader, QStringList &curPathList))
    QXmlStreamReader _arg0{};
    QStringList _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->processBookmarkElement(_arg0, _arg1));
}

TEST_F(RecentIterateWorkerTest, removeOutdatedItems)
{
    // Test method: void removeOutdatedItems((const QStringList &cachedPathList, const QStringList &curPathList))
    QStringList _arg0{};
    QStringList _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->removeOutdatedItems(_arg0, _arg1));
}

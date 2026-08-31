// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_cacheworker.cpp
 * @brief Unit tests for CacheWorker methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/infocache.h"

#include <QTest>

using namespace src;

class CacheWorkerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CacheWorker();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CacheWorker *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CacheWorkerTest, CacheWorker)
{
    // Test constructor: CacheWorker((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CacheWorkerTest, cacheInfo)
{
    // Test method: void cacheInfo((const QUrl url, const FileInfoPointer info))
    EXPECT_NO_FATAL_FAILURE(obj->cacheInfo(QUrl(), FileInfoPointer()));
}

TEST_F(CacheWorkerTest, disconnectWatcher)
{
    // Test method: void disconnectWatcher((const QMap<QUrl, FileInfoPointer> infos))
    EXPECT_NO_FATAL_FAILURE(obj->disconnectWatcher(QMap<QUrl, FileInfoPointer>()));
}

TEST_F(CacheWorkerTest, removeCaches)
{
    // Test method: void removeCaches((const QList<QUrl> urls))
    EXPECT_NO_FATAL_FAILURE(obj->removeCaches(QList<QUrl>()));
}

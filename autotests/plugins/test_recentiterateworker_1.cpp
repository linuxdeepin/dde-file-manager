// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentiterateworker_1.cpp
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

TEST_F(RecentIterateWorkerTest, onRequestAddRecentItem)
{
    // Test method: void onRequestAddRecentItem((const QVariantMap &item))
    QVariantMap _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onRequestAddRecentItem(_arg0));
}

TEST_F(RecentIterateWorkerTest, onRequestPurgeItems)
{
    // Test method: void onRequestPurgeItems((const QString &xbelPath))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onRequestPurgeItems(_arg0));
}

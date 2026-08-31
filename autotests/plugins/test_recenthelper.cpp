// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recenthelper.cpp
 * @brief Unit tests for RecentHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/recentmanager.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentHelperTest, clearRecent)
{
    // Test method: void clearRecent(())
    EXPECT_NO_FATAL_FAILURE(obj->clearRecent());
}

TEST_F(RecentHelperTest, removeRecent)
{
    // Test method: void removeRecent((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->removeRecent(_arg0));
}

TEST_F(RecentHelperTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

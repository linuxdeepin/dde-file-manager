// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentdaemon_1.cpp
 * @brief Unit tests for RecentDaemon methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "recentdaemon.h"

#include <QTest>

using namespace recent;

class RecentDaemonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentDaemon();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentDaemon *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentDaemonTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(RecentDaemonTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

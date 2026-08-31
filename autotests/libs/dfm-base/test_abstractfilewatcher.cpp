// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_abstractfilewatcher.cpp
 * @brief Unit tests for TestWatcherPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "libs/dfm-base/test_abstractfilewatcher.h"

#include <QTest>

using namespace autotests;

class TestWatcherPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TestWatcherPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TestWatcherPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TestWatcherPrivateTest, TestWatcherPrivate)
{
    // Test constructor: TestWatcherPrivate(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(TestWatcherPrivateTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(TestWatcherPrivateTest, stop)
{
    // Test bool getter: stop()
    bool result = obj->stop();
    EXPECT_FALSE(result);

}

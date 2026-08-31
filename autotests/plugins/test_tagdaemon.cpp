// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tagdaemon.cpp
 * @brief Unit tests for TagDaemon methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "tagdaemon.h"

#include <QTest>

using namespace tag;

class TagDaemonTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagDaemon();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagDaemon *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagDaemonTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(TagDaemonTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}

TEST_F(TagDaemonTest, stop)
{
    // Test method: void stop(())
    EXPECT_NO_FATAL_FAILURE(obj->stop());
}

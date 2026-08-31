// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_schemenode.cpp
 * @brief Unit tests for SchemeNode methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/urlroute.h"

#include <QTest>

using namespace include;

class SchemeNodeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SchemeNode();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SchemeNode *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SchemeNodeTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(SchemeNodeTest, isEmpty)
{
    // Test bool getter: isEmpty()
    bool result = obj->isEmpty();
    EXPECT_TRUE(result);

}

TEST_F(SchemeNodeTest, rootPath)
{
    // Test getter: QString rootPath()
    auto result = obj->rootPath();
    EXPECT_TRUE(result.isEmpty());

}

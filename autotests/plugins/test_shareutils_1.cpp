// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shareutils_1.cpp
 * @brief Unit tests for ShareUtils methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/shareutils.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareUtilsTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareUtils();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareUtils *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareUtilsTest, ShareUtils)
{
    // Test constructor: ShareUtils((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShareUtilsTest, displayName)
{
    // Test getter: QString displayName()
    auto result = obj->displayName();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(ShareUtilsTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(ShareUtilsTest, instance)
{
    // Test getter: ShareUtils instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(ShareUtilsTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

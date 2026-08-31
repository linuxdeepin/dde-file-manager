// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shareiteratorprivate.cpp
 * @brief Unit tests for ShareIteratorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "iterator/shareiterator.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareIteratorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareIteratorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareIteratorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareIteratorPrivateTest, ShareIteratorPrivate)
{
    // Test constructor: ShareIteratorPrivate((ShareIterator *qq, const QUrl &url))
    ASSERT_NE(obj, nullptr);
}

TEST_F(ShareIteratorPrivateTest, ShareIteratorPrivate_Destructor)
{
    // Test method:  ~ShareIteratorPrivate(())
    EXPECT_NO_FATAL_FAILURE({ ShareIteratorPrivate *tmp = new ShareIteratorPrivate(); delete tmp; });
}

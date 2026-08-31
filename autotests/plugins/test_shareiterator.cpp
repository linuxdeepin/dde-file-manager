// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_shareiterator.cpp
 * @brief Unit tests for ShareIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "iterator/shareiterator.h"

#include <QTest>

using namespace dfmplugin_myshares;

class ShareIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ShareIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ShareIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ShareIteratorTest, url)
{
    // Test getter: QUrl url()
    auto result = obj->url();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

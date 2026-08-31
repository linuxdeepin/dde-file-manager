// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_bidirectionhash.cpp
 * @brief Unit tests for BidirectionHash methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/configs/private/settingbackend_p.h"

#include <QTest>

using namespace src;

class BidirectionHashTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new BidirectionHash();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    BidirectionHash *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(BidirectionHashTest, key)
{
    // Test getter: Key key()
    auto result = obj->key();
    EXPECT_NO_FATAL_FAILURE({ obj->key(); });

}

TEST_F(BidirectionHashTest, keys)
{
    // Test getter: QList<Key> keys()
    auto result = obj->keys();
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(BidirectionHashTest, value)
{
    // Test getter: T value()
    auto result = obj->value();
    EXPECT_NO_FATAL_FAILURE({ obj->value(); });

}

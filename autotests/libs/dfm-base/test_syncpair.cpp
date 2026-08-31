// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_syncpair.cpp
 * @brief Unit tests for SyncPair methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/base/configs/configsyncdefs.h"

#include <QTest>

using namespace src;

class SyncPairTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SyncPair();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SyncPair *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SyncPairTest, isValid)
{
    // Test bool getter: isValid()
    bool result = obj->isValid();
    EXPECT_FALSE(result);

}

TEST_F(SyncPairTest, serialize)
{
    // Test getter: QString serialize()
    auto result = obj->serialize();
    EXPECT_TRUE(result.isEmpty());

}

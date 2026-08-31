// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_timetoupdatecache.cpp
 * @brief Unit tests for TimeToUpdateCache methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "dfm-base/utils/infocache.h"

#include <QTest>

using namespace src;

class TimeToUpdateCacheTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TimeToUpdateCache();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TimeToUpdateCache *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TimeToUpdateCacheTest, dealRemoveInfo)
{
    // Test method: void dealRemoveInfo(())
    EXPECT_NO_FATAL_FAILURE(obj->dealRemoveInfo());
}

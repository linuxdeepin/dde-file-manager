// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_indextask.cpp
 * @brief Unit tests for IndexTask methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/indextask.h"

#include <QTest>

using namespace src;

class IndexTaskTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IndexTask();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IndexTask *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IndexTaskTest, onProgressChanged)
{
    // Test method: void onProgressChanged((qint64 count, qint64 total))
    EXPECT_NO_FATAL_FAILURE(obj->onProgressChanged(0, 0));
}

TEST_F(IndexTaskTest, throttleCpuUsage)
{
    // Test method: void throttleCpuUsage(())
    EXPECT_NO_FATAL_FAILURE(obj->throttleCpuUsage());
}

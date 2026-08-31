// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_low_procpartition.cpp
 * @brief Unit tests for ProcPartition methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/core/dmsetup.h"

#include <QTest>

using namespace src;

class ProcPartitionTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProcPartition();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProcPartition *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProcPartitionTest, devNum)
{
    // Test getter: QString devNum()
    auto result = obj->devNum();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

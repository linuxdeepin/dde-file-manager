// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dm_setup_helper.cpp
 * @brief Unit tests for dm_setup_helper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/core/dmsetup.h"

#include <QTest>

using namespace src;

class dm_setup_helperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new dm_setup_helper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    dm_setup_helper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(dm_setup_helperTest, procPartitions)
{
    // Test getter: QList<dm_setup_helper::ProcPartition> procPartitions()
    auto result = obj->procPartitions();
    EXPECT_TRUE(result.isEmpty());

}

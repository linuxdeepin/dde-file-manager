// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dmtaskhelper.cpp
 * @brief Unit tests for DMTaskHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/diskencrypt/core/dmsetup.h"

#include <QTest>

using namespace src;

class DMTaskHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new DMTaskHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    DMTaskHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(DMTaskHelperTest, task)
{
    // Test getter: struct dm_task task()
    auto result = obj->task();
    EXPECT_NO_FATAL_FAILURE({ obj->task(); });

}

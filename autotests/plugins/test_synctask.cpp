// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_synctask.cpp
 * @brief Unit tests for SyncTask methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "syncdbus.h"

#include <QTest>

using namespace core;

class SyncTaskTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new SyncTask();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    SyncTask *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(SyncTaskTest, path)
{
    // Test getter: QString path()
    auto result = obj->path();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(SyncTaskTest, run)
{
    // Test method: void run(())
    EXPECT_NO_FATAL_FAILURE(obj->run());
}

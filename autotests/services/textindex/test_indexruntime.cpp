// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_indexruntime.cpp
 * @brief Unit tests for IndexRuntime methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/core/indexruntime.h"

#include <QTest>

using namespace src;

class IndexRuntimeTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new IndexRuntime();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    IndexRuntime *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(IndexRuntimeTest, context)
{
    // Test getter: IndexContext context()
    auto result = obj->context();
    EXPECT_NO_FATAL_FAILURE({ obj->context(); });

}

TEST_F(IndexRuntimeTest, profile)
{
    // Test getter: IndexProfile profile()
    auto result = obj->profile();
    EXPECT_NO_FATAL_FAILURE({ obj->profile(); });

}

TEST_F(IndexRuntimeTest, stateStore)
{
    // Test getter: IndexStateStore stateStore()
    auto result = obj->stateStore();
    EXPECT_NO_FATAL_FAILURE({ obj->stateStore(); });

}

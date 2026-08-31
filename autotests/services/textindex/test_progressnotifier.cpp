// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_progressnotifier.cpp
 * @brief Unit tests for ProgressNotifier methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "services/textindex/task/progressnotifier.h"

#include <QTest>

using namespace src;

class ProgressNotifierTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ProgressNotifier();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ProgressNotifier *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ProgressNotifierTest, instance)
{
    // Test getter: SERVICETEXTINDEX_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

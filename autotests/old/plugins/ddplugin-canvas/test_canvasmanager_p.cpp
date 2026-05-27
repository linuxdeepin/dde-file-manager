// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

// Basic functionality test for canvasmanager_p
class TestCanvasmanager_p : public testing::Test
{
public:
    void SetUp() override
    {
        // Setup test environment
    }

    void TearDown() override
    {
        stub.clear();
    }

protected:
    stub_ext::StubExt stub;
};

/**
 * @brief Basic test to ensure test framework works
 */
TEST_F(TestCanvasmanager_p, BasicTest_Framework_Works)
{
    // This test ensures the test framework is working
    EXPECT_TRUE(true);
}

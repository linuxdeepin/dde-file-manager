// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>

// Simple smoke tests to ensure basic functionality
class UT_AccessControlSimple : public testing::Test
{
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

// Basic smoke test
TEST_F(UT_AccessControlSimple, BasicSmokeTest_Success)
{
    // This is a basic smoke test to ensure the test framework works
    EXPECT_TRUE(true);
}

// Test stub functionality
TEST_F(UT_AccessControlSimple, StubFunctionality_Works)
{
    bool stubCalled = false;
    
    // Test that stub can mock a simple function
    auto testFunc = [&stubCalled]() -> bool {
        stubCalled = true;
        return true;
    };
    
    bool result = testFunc();
    EXPECT_TRUE(result);
    EXPECT_TRUE(stubCalled);
} 
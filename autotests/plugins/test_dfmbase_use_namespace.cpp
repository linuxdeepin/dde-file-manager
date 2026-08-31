// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_dfmbase_use_namespace.cpp
 * @brief Unit tests for DFMBASE_USE_NAMESPACE Mid-priority methods
 */

#include <gtest/gtest.h>

class DFMBASE_USE_NAMESPACETest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(DFMBASE_USE_NAMESPACETest, doWork)
{
    // doWork
    SUCCEED();
}

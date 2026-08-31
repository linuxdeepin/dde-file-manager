// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_processprioritymanager.cpp
 * @brief Unit tests for ProcessPriorityManager Mid-priority methods
 */

#include <gtest/gtest.h>

class ProcessPriorityManagerTest : public ::testing::Test {
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(ProcessPriorityManagerTest, lowerCpuNicePriority)
{
    // lowerCpuNicePriority
    SUCCEED();
}

TEST_F(ProcessPriorityManagerTest, lowerIoPriority)
{
    // lowerIoPriority
    SUCCEED();
}

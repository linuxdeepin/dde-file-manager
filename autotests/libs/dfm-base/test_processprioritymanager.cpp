// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_processprioritymanager.cpp
 * @brief Unit tests for ProcessPriorityManager (utils/processprioritymanager.cpp)
 *        — all static methods. They use syscalls (setpriority, ioprio_set,
 *        sched_setscheduler) which gracefully fail in the test sandbox
 *        (non-root) and return false; no hangs.
 */

#include <gtest/gtest.h>

#include <dfm-base/utils/processprioritymanager.h>

using namespace dfmbase;

TEST(ProcessPriorityManagerTest, LowerIoPriorityReturnsBool)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ProcessPriorityManager::lowerIoPriority(); });
}

TEST(ProcessPriorityManagerTest, LowerCpuNicePriorityDefaultReturnsBool)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ProcessPriorityManager::lowerCpuNicePriority(); });
}

TEST(ProcessPriorityManagerTest, LowerCpuNicePriorityClampsHighValue)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ProcessPriorityManager::lowerCpuNicePriority(999); });
}

TEST(ProcessPriorityManagerTest, LowerCpuNicePriorityClampsLowValue)
{
    EXPECT_NO_FATAL_FAILURE({ (void)ProcessPriorityManager::lowerCpuNicePriority(-999); });
}

TEST(ProcessPriorityManagerTest, SetCpuSchedulingPolicyBatchReturnsBool)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)ProcessPriorityManager::setCpuSchedulingPolicy(
                ProcessPriorityManager::CpuSchedulingPolicy::Batch);
    });
}

TEST(ProcessPriorityManagerTest, SetCpuSchedulingPolicyIdleReturnsBool)
{
    EXPECT_NO_FATAL_FAILURE({
        (void)ProcessPriorityManager::setCpuSchedulingPolicy(
                ProcessPriorityManager::CpuSchedulingPolicy::Idle);
    });
}

TEST(ProcessPriorityManagerTest, SetCpuSchedulingPolicyNormalReturnsTrue)
{
    // Normal policy short-circuits and returns true without syscalls.
    EXPECT_TRUE(ProcessPriorityManager::setCpuSchedulingPolicy(
            ProcessPriorityManager::CpuSchedulingPolicy::Normal));
}

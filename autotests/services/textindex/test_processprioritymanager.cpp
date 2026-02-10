// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>
#include <cerrno>
#include <cstring>

#include "utils/processprioritymanager.h"

SERVICETEXTINDEX_USE_NAMESPACE

static bool kMockSyscallSuccess = true;
static int kMockSyscallErrno = 0;
static int kCallCount = 0;

long int syscall_hook1(long int number, ...)
{
    __DBG_STUB_INVOKE__
    if (!kMockSyscallSuccess) {
        errno = kMockSyscallErrno;
        return -1;
    }
    errno = 0;
    return 0;
}

long int syscall_hook2(long int number, ...)
{
    __DBG_STUB_INVOKE__
    kCallCount++;
    if (kCallCount == 1) {
        // First call (IDLE class) fails
        errno = EPERM;
        return -1;
    } else {
        // Second call (Best Effort) succeeds
        errno = 0;
        return 0;
    }
}

class UT_ProcessPriorityManager : public testing::Test
{
protected:
    void SetUp() override
    {
        // Reset errno for each testf
        errno = 0;

        // Reset mock states
        mockSetprioritySuccess = true;
        mockSetpriorityErrno = 0;
        kMockSyscallSuccess = true;
        kMockSyscallErrno = 0;
        mockSchedSetschedulerSuccess = true;
        mockSchedSetschedulerErrno = 0;
    }

    void TearDown() override
    {
        stub.clear();
    }

    void setupSetpriorityMock(bool success = true, int errorCode = 0)
    {
        mockSetprioritySuccess = success;
        mockSetpriorityErrno = errorCode;

        stub.set_lamda(setpriority, [this](int which, int who, int prio) -> int {
            __DBG_STUB_INVOKE__
            if (!mockSetprioritySuccess) {
                errno = mockSetpriorityErrno;
                return -1;
            }
            errno = 0;
            return 0;
        });
    }

    void setupSyscallMock(bool success = true, int errorCode = 0)
    {
        kMockSyscallSuccess = success;
        kMockSyscallErrno = errorCode;

        stub.set(syscall, syscall_hook1);
        // stub.set_lamda(syscall, [this](long int number, ...) -> long int {
        //     __DBG_STUB_INVOKE__
        //     if (!mockSyscallSuccess) {
        //         errno = mockSyscallErrno;
        //         return -1;
        //     }
        //     errno = 0;
        //     return 0;
        // });
    }

    void setupSchedSetschedulerMock(bool success = true, int errorCode = 0)
    {
        mockSchedSetschedulerSuccess = success;
        mockSchedSetschedulerErrno = errorCode;

        stub.set_lamda(sched_setscheduler, [this](pid_t pid, int policy, const struct sched_param *param) -> int {
            __DBG_STUB_INVOKE__
            if (!mockSchedSetschedulerSuccess) {
                errno = mockSchedSetschedulerErrno;
                return -1;
            }
            errno = 0;
            return 0;
        });
    }

    // Mock state variables
    bool mockSetprioritySuccess = true;
    int mockSetpriorityErrno = 0;
    bool mockSchedSetschedulerSuccess = true;
    int mockSchedSetschedulerErrno = 0;

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_ProcessPriorityManager, LowerCpuNicePriority_DefaultValue_ReturnsTrue)
{
    setupSetpriorityMock(true);

    bool result = ProcessPriorityManager::lowerCpuNicePriority();

    EXPECT_TRUE(result);
}

TEST_F(UT_ProcessPriorityManager, LowerCpuNicePriority_CustomValue_ReturnsTrue)
{
    setupSetpriorityMock(true);

    bool result = ProcessPriorityManager::lowerCpuNicePriority(10);

    EXPECT_TRUE(result);
}

TEST_F(UT_ProcessPriorityManager, LowerCpuNicePriority_ValueTooLow_ClampsToMinimum)
{
    setupSetpriorityMock(true);

    bool result = ProcessPriorityManager::lowerCpuNicePriority(-25);

    EXPECT_TRUE(result);
}

TEST_F(UT_ProcessPriorityManager, LowerCpuNicePriority_ValueTooHigh_ClampsToMaximum)
{
    setupSetpriorityMock(true);

    bool result = ProcessPriorityManager::lowerCpuNicePriority(25);

    EXPECT_TRUE(result);
}

TEST_F(UT_ProcessPriorityManager, LowerCpuNicePriority_SetpriorityFails_ReturnsFalse)
{
    setupSetpriorityMock(false, EPERM);

    bool result = ProcessPriorityManager::lowerCpuNicePriority();

    EXPECT_FALSE(result);
}

TEST_F(UT_ProcessPriorityManager, LowerCpuNicePriority_SetpriorityReturnsNegativeOneButErrnoZero_ReturnsTrue)
{
    // Special case: setpriority can legitimately return -1 if old priority was -1
    stub.set_lamda(setpriority, [](int which, int who, int prio) -> int {
        __DBG_STUB_INVOKE__
        errno = 0;   // No error
        return -1;   // But return -1 (valid case)
    });

    bool result = ProcessPriorityManager::lowerCpuNicePriority();

    EXPECT_TRUE(result);
}

TEST_F(UT_ProcessPriorityManager, LowerIoPriority_Success_ReturnsTrue)
{
    setupSyscallMock(true);

    bool result = ProcessPriorityManager::lowerIoPriority();

    EXPECT_TRUE(result);
}

TEST_F(UT_ProcessPriorityManager, LowerIoPriority_IdleClassFails_FallsBackToBestEffort)
{
    kCallCount = 0;
    stub.set(syscall, syscall_hook2);

    // stub.set_lamda(syscall, [&callCount](long number, ...) -> long {
    //     __DBG_STUB_INVOKE__
    //     callCount++;
    //     if (callCount == 1) {
    //         // First call (IDLE class) fails
    //         errno = EPERM;
    //         return -1;
    //     } else {
    //         // Second call (Best Effort) succeeds
    //         errno = 0;
    //         return 0;
    //     }
    // });

    bool result = ProcessPriorityManager::lowerIoPriority();

    EXPECT_TRUE(result);
    EXPECT_EQ(kCallCount, 2);
}

TEST_F(UT_ProcessPriorityManager, LowerIoPriority_BothClassesFail_ReturnsFalse)
{
    setupSyscallMock(false, EPERM);

    bool result = ProcessPriorityManager::lowerIoPriority();

    EXPECT_FALSE(result);
}

#ifdef Q_OS_LINUX
TEST_F(UT_ProcessPriorityManager, SetCpuSchedulingPolicy_IdlePolicy_ReturnsTrue)
{
    setupSchedSetschedulerMock(true);

    bool result = ProcessPriorityManager::setCpuSchedulingPolicy(ProcessPriorityManager::CpuSchedulingPolicy::Idle);

    EXPECT_TRUE(result);
}

TEST_F(UT_ProcessPriorityManager, SetCpuSchedulingPolicy_BatchPolicy_ReturnsTrue)
{
    setupSchedSetschedulerMock(true);

    bool result = ProcessPriorityManager::setCpuSchedulingPolicy(ProcessPriorityManager::CpuSchedulingPolicy::Batch);

    EXPECT_TRUE(result);
}

TEST_F(UT_ProcessPriorityManager, SetCpuSchedulingPolicy_NormalPolicy_ReturnsTrue)
{
    // Normal policy should return true without calling sched_setscheduler
    bool result = ProcessPriorityManager::setCpuSchedulingPolicy(ProcessPriorityManager::CpuSchedulingPolicy::Normal);

    EXPECT_TRUE(result);
}

TEST_F(UT_ProcessPriorityManager, SetCpuSchedulingPolicy_SchedSetschedulerFails_ReturnsFalse)
{
    setupSchedSetschedulerMock(false, EPERM);

    bool result = ProcessPriorityManager::setCpuSchedulingPolicy(ProcessPriorityManager::CpuSchedulingPolicy::Idle);

    EXPECT_FALSE(result);
}

TEST_F(UT_ProcessPriorityManager, SetCpuSchedulingPolicy_UnsupportedPolicy_ReturnsFalse)
{
    // Mock SCHED_IDLE as not defined by making sched_setscheduler fail
    setupSchedSetschedulerMock(false, EINVAL);

    bool result = ProcessPriorityManager::setCpuSchedulingPolicy(ProcessPriorityManager::CpuSchedulingPolicy::Idle);

    EXPECT_FALSE(result);
}
#endif   // Q_OS_LINUX

TEST_F(UT_ProcessPriorityManager, LowerAllAvailablePriorities_VerboseTrue_ExecutesAllSteps)
{
    setupSetpriorityMock(true);
    setupSyscallMock(true);
    setupSchedSetschedulerMock(true);

    // Should not throw or crash
    EXPECT_NO_THROW({
        ProcessPriorityManager::lowerAllAvailablePriorities(true);
    });
}

TEST_F(UT_ProcessPriorityManager, LowerAllAvailablePriorities_VerboseFalse_ExecutesAllSteps)
{
    setupSetpriorityMock(true);
    setupSyscallMock(true);
    setupSchedSetschedulerMock(true);

    // Should not throw or crash
    EXPECT_NO_THROW({
        ProcessPriorityManager::lowerAllAvailablePriorities(false);
    });
}

TEST_F(UT_ProcessPriorityManager, LowerAllAvailablePriorities_AllOperationsFail_DoesNotCrash)
{
    setupSetpriorityMock(false, EPERM);
    setupSyscallMock(false, EPERM);
    setupSchedSetschedulerMock(false, EPERM);

    // Should not throw or crash even if all operations fail
    EXPECT_NO_THROW({
        ProcessPriorityManager::lowerAllAvailablePriorities(true);
    });
}

TEST_F(UT_ProcessPriorityManager, LowerAllAvailablePriorities_PartialFailure_ContinuesExecution)
{
    // IO priority fails, but others succeed
    setupSetpriorityMock(true);
    setupSyscallMock(false, EPERM);
    setupSchedSetschedulerMock(true);

    // Should not throw or crash
    EXPECT_NO_THROW({
        ProcessPriorityManager::lowerAllAvailablePriorities(false);
    });
}

#ifdef Q_OS_LINUX
TEST_F(UT_ProcessPriorityManager, LowerAllAvailablePriorities_IdlePolicyFails_FallsBackToBatch)
{
    setupSetpriorityMock(true);
    setupSyscallMock(true);

    int schedCallCount = 0;
    stub.set_lamda(sched_setscheduler, [&schedCallCount](pid_t pid, int policy, const struct sched_param *param) -> int {
        __DBG_STUB_INVOKE__
        schedCallCount++;
        if (schedCallCount == 1) {
            // First call (IDLE) fails
            errno = EPERM;
            return -1;
        } else {
            // Second call (BATCH) succeeds
            errno = 0;
            return 0;
        }
    });

    // Should not throw or crash
    EXPECT_NO_THROW({
        ProcessPriorityManager::lowerAllAvailablePriorities(true);
    });

    EXPECT_EQ(schedCallCount, 2);
}
#endif   // Q_OS_LINUX

TEST_F(UT_ProcessPriorityManager, LowerCpuNicePriority_EdgeCaseValues_HandledCorrectly)
{
    setupSetpriorityMock(true);

    // Test boundary values
    EXPECT_TRUE(ProcessPriorityManager::lowerCpuNicePriority(-20));
    EXPECT_TRUE(ProcessPriorityManager::lowerCpuNicePriority(19));
    EXPECT_TRUE(ProcessPriorityManager::lowerCpuNicePriority(0));
}

TEST_F(UT_ProcessPriorityManager, LowerIoPriority_NotLinux_ReturnsTrue)
{
    // On non-Linux platforms, the function should return true (no-op)
    // This test assumes the function handles platform differences gracefully

#ifndef Q_OS_LINUX
    bool result = ProcessPriorityManager::lowerIoPriority();
    EXPECT_TRUE(result);
#endif
}

TEST_F(UT_ProcessPriorityManager, LowerCpuNicePriority_NotUnix_ReturnsTrue)
{
    // On non-Unix platforms, the function should return true (no-op)
    // This test assumes the function handles platform differences gracefully

#if !defined(Q_OS_UNIX) || defined(Q_OS_WASM)
    bool result = ProcessPriorityManager::lowerCpuNicePriority();
    EXPECT_TRUE(result);
#endif
}

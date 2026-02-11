// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "processprioritymanager.h"

#ifdef Q_OS_LINUX
// SYS_ioprio_set might not be defined in <sys/syscall.h> on all toolchains/glibc versions.
// If it's not, you might need to define it based on your architecture.
// Common values: __NR_ioprio_set. For x86_64 it's 251, for i386 it's 289.
// However, modern systems should usually have it. We rely on <sys/syscall.h> for SYS_ioprio_set.
#    ifndef SYS_ioprio_set
// Provide a fallback or error if absolutely necessary and known for the target arch
// For example, for x86_64:
// #if defined(__x86_64__) && !defined(SYS_ioprio_set)
// #define SYS_ioprio_set 251
// #endif
// For this example, we'll assume it's defined if ioprio syscalls are generally available.
#    endif
#endif   // Q_OS_LINUX

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace {   // Anonymous namespace for helpers local to this translation unit
const char *getPolicySpelling(ProcessPriorityManager::CpuSchedulingPolicy policy)
{
    switch (policy) {
    case ProcessPriorityManager::CpuSchedulingPolicy::Batch:
        return "BATCH";
    case ProcessPriorityManager::CpuSchedulingPolicy::Idle:
        return "IDLE";
    case ProcessPriorityManager::CpuSchedulingPolicy::Normal:
        return "NORMAL";   // Though Normal case returns early
    default:
        return "UnknownPolicy";   // Should not happen if switch in main func is exhaustive
    }
}
}   // anonymous namespace

bool ProcessPriorityManager::lowerIoPriority()
{
#if defined(Q_OS_LINUX) && defined(SYS_ioprio_set)
    // Try to set I/O scheduling to IDLE class
    // int ioprio = (IoPrio::IOPRIO_CLASS_IDLE << IoPrio::IOPRIO_CLASS_SHIFT);
    // Or directly use the integer value if enums are defined with those exact values:
    int ioprio_idle_val = (IoPrio::IOPRIO_CLASS_IDLE << IoPrio::IOPRIO_CLASS_SHIFT);

    if (syscall(SYS_ioprio_set, IoPrio::IOPRIO_WHO_PROCESS, 0, ioprio_idle_val) < 0) {
        fmWarning() << "ProcessPriorityManager: Cannot set I/O scheduling to IDLE ("
                    << strerror(errno) << "). Trying Best Effort (lowest).";
        // Fallback: Try to set I/O scheduling to Best Effort class, lowest priority (7)
        int ioprio_be_lowest_val = (IoPrio::IOPRIO_CLASS_BE << IoPrio::IOPRIO_CLASS_SHIFT) | 7;
        if (syscall(SYS_ioprio_set, IoPrio::IOPRIO_WHO_PROCESS, 0, ioprio_be_lowest_val) < 0) {
            fmWarning() << "ProcessPriorityManager: Cannot set I/O scheduling to Best Effort ("
                        << strerror(errno) << ").";
            return false;
        }
        fmInfo() << "ProcessPriorityManager: I/O priority set to Best Effort (lowest).";
    } else {
        fmInfo() << "ProcessPriorityManager: I/O priority set to IDLE.";
    }
    return true;
#else
    // fmDebug() << "ProcessPriorityManager: I/O priority setting (ioprio_set) not supported on this platform.";
    return true;   // Not an error, just not applicable
#endif
}

bool ProcessPriorityManager::lowerCpuNicePriority(int niceValue)
{
#if defined(Q_OS_UNIX) && !defined(Q_OS_WASM)   // WASM doesn't have this
    if (niceValue < -20) niceValue = -20;
    if (niceValue > 19) niceValue = 19;

    errno = 0;   // POSIX requires setting errno to 0 before call for some functions
    if (setpriority(PRIO_PROCESS, 0, niceValue) == -1) {
        // Check errno because setpriority can legitimately return -1 on success
        // if the process's old priority was -1.
        if (errno != 0) {
            fmWarning() << "ProcessPriorityManager: Cannot lower CPU nice priority to" << niceValue
                        << "(" << strerror(errno) << ").";
            return false;
        }
    }
    fmInfo() << "ProcessPriorityManager: CPU nice priority set to" << niceValue;
    return true;
#else
    // fmDebug() << "ProcessPriorityManager: CPU nice priority setting (setpriority) not supported on this platform.";
    Q_UNUSED(niceValue);
    return true;   // Not an error, just not applicable
#endif
}

#ifdef Q_OS_LINUX
bool ProcessPriorityManager::setCpuSchedulingPolicy(CpuSchedulingPolicy policy)
{
    struct sched_param param;
    memset(&param, 0, sizeof(param));   // Must be 0 for SCHED_BATCH/IDLE
    param.sched_priority = 0;

    int schedPolicyValue = -1;
    switch (policy) {
#    ifdef SCHED_BATCH
    case CpuSchedulingPolicy::Batch:
        schedPolicyValue = SCHED_BATCH;
        // policyName will be derived later using getPolicySpelling(policy)
        break;
#    endif
#    ifdef SCHED_IDLE
    case CpuSchedulingPolicy::Idle:
        schedPolicyValue = SCHED_IDLE;
        // policyName will be derived later using getPolicySpelling(policy)
        break;
#    endif
    case CpuSchedulingPolicy::Normal:
        qInfo() << "ProcessPriorityManager: CPU scheduling policy Normal (SCHED_OTHER) requested. No change applied by this function.";
        return true;
    default:
        qWarning() << "ProcessPriorityManager: Unknown or unsupported CPU scheduling policy requested ("
                   << static_cast<int>(policy) << ").";   // Log the int value of the policy
        return false;
    }

    // If we reach here, policy was either Batch or Idle.
    const char *requestedPolicyName = getPolicySpelling(policy);

    if (schedPolicyValue != -1) {
        // A schedPolicyValue was assigned (i.e., SCHED_BATCH or SCHED_IDLE was defined and matched)
        if (sched_setscheduler(0, schedPolicyValue, &param) == -1) {
            qWarning() << "ProcessPriorityManager: Cannot set CPU scheduling policy to" << requestedPolicyName
                       << "(" << strerror(errno) << ").";
            return false;
        }
        qInfo() << "ProcessPriorityManager: CPU scheduling policy set to" << requestedPolicyName << ".";
        return true;
    } else {
        // schedPolicyValue is still -1.
        // This means the requested policy (Batch or Idle) was not available
        // because its SCHED_XXX macro was not defined at compile time.
        qDebug() << "ProcessPriorityManager: CPU scheduling policy" << requestedPolicyName
                 << "is not defined/supported on this system build (its SCHED_XXX macro is not defined).";
        return false;
    }
}
#endif   // Q_OS_LINUX

void ProcessPriorityManager::lowerAllAvailablePriorities(bool verboseLog)
{
    if (verboseLog) {
        fmInfo() << "ProcessPriorityManager: Attempting to lower all available priorities...";
    }

    bool ioLowered = lowerIoPriority();
    if (verboseLog) {
        if (ioLowered)
            fmInfo() << "  - I/O priority lowering attempted/succeeded or N/A.";
        else
            fmWarning() << "  - I/O priority lowering FAILED.";
    }

#ifdef Q_OS_LINUX
    bool schedIdleSet = setCpuSchedulingPolicy(CpuSchedulingPolicy::Idle);
    if (schedIdleSet) {
        if (verboseLog) fmInfo() << "  - CPU scheduling policy set to IDLE.";
    } else {
        if (verboseLog) fmWarning() << "  - Failed to set CPU scheduling policy to IDLE. Trying BATCH...";
        bool schedBatchSet = setCpuSchedulingPolicy(CpuSchedulingPolicy::Batch);
        if (schedBatchSet) {
            if (verboseLog) fmInfo() << "  - CPU scheduling policy set to BATCH.";
        } else {
            if (verboseLog) fmWarning() << "  - Failed to set CPU scheduling policy to BATCH.";
        }
    }
#else
    if (verboseLog) fmDebug() << "  - CPU scheduling policy (SCHED_IDLE/BATCH) not available on this platform.";
#endif

    bool niceLowered = lowerCpuNicePriority();   // Default to 19
    if (verboseLog) {
        if (niceLowered)
            fmInfo() << "  - CPU nice priority lowering attempted/succeeded or N/A.";
        else
            fmWarning() << "  - CPU nice priority lowering FAILED.";
    }

    if (verboseLog) fmInfo() << "ProcessPriorityManager: Priority lowering attempts complete.";
}

SERVICETEXTINDEX_END_NAMESPACE

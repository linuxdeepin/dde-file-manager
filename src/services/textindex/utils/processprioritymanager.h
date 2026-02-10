// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef PROCESSPRIORITYMANAGER_H
#define PROCESSPRIORITYMANAGER_H

#include "service_textindex_global.h"

#include <QtGlobal>   // For Q_OS_LINUX etc.

// _GNU_SOURCE for syscall and sched_setscheduler extensions
#ifndef _GNU_SOURCE
#    define _GNU_SOURCE
#endif

#include <QDebug>   // For logging

#ifdef Q_OS_LINUX
#    include <sys/time.h>
#    include <sys/resource.h>
#    include <unistd.h>
#    include <sys/syscall.h>
#    include <cerrno>
#    include <cstring>   // For strerror, memset
#    include <sched.h>
#endif   // Q_OS_LINUX

SERVICETEXTINDEX_BEGIN_NAMESPACE

class ProcessPriorityManager
{
public:
    // Make the class non-instantiable, all methods are static
    ProcessPriorityManager() = delete;

    /**
     * @brief Tries to lower the I/O priority of the current process to IDLE.
     * On Linux, this uses ioprio_set.
     * @return True if successful or not applicable, false on failure.
     */
    static bool lowerIoPriority();

    /**
     * @brief Lowers the CPU scheduling priority (nice value) of the current process.
     * @param niceValue The nice value to set (typically 1 to 19 for lower priority).
     *                  19 is the lowest standard priority.
     * @return True if successful or not applicable, false on failure.
     */
    static bool lowerCpuNicePriority(int niceValue = 19);

#ifdef Q_OS_LINUX
    enum class CpuSchedulingPolicy {
        Normal,   // Default SCHED_OTHER (not actively set by this class)
        Batch,   // SCHED_BATCH for non-interactive, CPU-intensive tasks
        Idle   // SCHED_IDLE for very low priority tasks
    };

    /**
     * @brief Sets the CPU scheduling policy for the current process (Linux-specific).
     * @param policy The desired scheduling policy (Batch or Idle).
     * @return True if successful, false on failure or if not supported.
     */
    static bool setCpuSchedulingPolicy(CpuSchedulingPolicy policy);
#endif   // Q_OS_LINUX

    /**
     * @brief Attempts to lower all available priorities for the current process.
     * This will try to:
     * 1. Set CPU scheduling to Idle, then Batch (Linux-specific).
     * 2. Lower I/O priority (Linux-specific).
     * 3. Lower CPU nice value.
     * It logs successes and failures but doesn't return a single success/failure status
     * as some operations might succeed while others fail or are not applicable.
     * Call individual methods if you need specific success/failure feedback.
     * @param verboseLog If true, logs details of each step.
     */
    static void lowerAllAvailablePriorities(bool verboseLog = false);

private:
#ifdef Q_OS_LINUX
    // Constants for ioprio_set, defined locally if not available from system headers
    // These are typically defined in <linux/ioprio.h>, but we ensure they exist.
    struct IoPrio
    {
        enum {
            IOPRIO_CLASS_NONE = 0,
            IOPRIO_CLASS_RT = 1,
            IOPRIO_CLASS_BE = 2,
            IOPRIO_CLASS_IDLE = 3,
        };
        enum {
            IOPRIO_WHO_PROCESS = 1,
            IOPRIO_WHO_PGRP = 2,
            IOPRIO_WHO_USER = 3,
        };
        static const int IOPRIO_CLASS_SHIFT = 13;
        static const int IOPRIO_PRIO_MASK = (1 << IOPRIO_CLASS_SHIFT) - 1;
    };
#endif   // Q_OS_LINUX
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // PROCESSPRIORITYMANAGER_H

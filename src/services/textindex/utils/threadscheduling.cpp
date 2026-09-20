// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "threadscheduling.h"

#include <sched.h>

#include <cerrno>
#include <cstring>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace ThreadScheduling {

bool setSelfSchedulerIdle(QString *errorMsg)
{
    sched_param param {};
    if (sched_setscheduler(0, SCHED_IDLE, &param) != 0) {
        if (errorMsg) {
            *errorMsg = QString("sched_setscheduler(SCHED_IDLE) failed: %1")
                                .arg(QString::fromUtf8(std::strerror(errno)));
        }
        return false;
    }
    return true;
}

}   // namespace ThreadScheduling

SERVICETEXTINDEX_END_NAMESPACE

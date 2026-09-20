// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef THREADSCHEDULING_H
#define THREADSCHEDULING_H

#include "service_textindex_global.h"

#include <QString>

SERVICETEXTINDEX_BEGIN_NAMESPACE

namespace ThreadScheduling {

/**
 * @brief 把调用线程标记为 SCHED_IDLE（Linux 无特权即可设置）。
 *
 * SCHED_IDLE 线程拥有最低的调度权重：只要系统里还有普通线程想用 CPU，
 * 它就不参与竞争，仅在网络/磁盘空闲时推进。用于重索引 worker 线程，
 * 使同进程内的实时性线程（如 VfsMonitor 的 socket 读线程）在任务运行
 * 期间（尤其整个服务被 systemd CPUQuota 限流的场景）仍能分到配额。
 *
 * sched_setscheduler(0, ...) 只作用于调用线程本身，对进程其他线程无影响；
 * 设置随线程生命周期保持，线程退出后自然失效。
 *
 * @param[out] errorMsg 失败时的错误信息（errno 文本）
 * @return true 表示当前线程已处于 SCHED_IDLE
 */
bool setSelfSchedulerIdle(QString *errorMsg);

}   // namespace ThreadScheduling

SERVICETEXTINDEX_END_NAMESPACE

#endif   // THREADSCHEDULING_H

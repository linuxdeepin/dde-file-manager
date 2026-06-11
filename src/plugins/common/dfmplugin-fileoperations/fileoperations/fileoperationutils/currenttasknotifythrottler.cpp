// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "currenttasknotifythrottler.h"

#include <QMutexLocker>

DPFILEOPERATIONS_USE_NAMESPACE

CurrentTaskNotifyThrottler::CurrentTaskNotifyThrottler(qint64 intervalMs)
    : intervalMs(intervalMs)
{
}

std::optional<CurrentTaskNotifyThrottler::Task> CurrentTaskNotifyThrottler::submit(const QUrl &sourceUrl, const QUrl &targetUrl)
{
    const Task task { sourceUrl, targetUrl };
    QMutexLocker locker(&mutex);

    if (!timer.isValid()) {
        // The first visible task switch should never be hidden behind throttling,
        // otherwise the dialog can stay on "In data statistics ...".
        timer.start();
        lastEmitElapsed = 0;
        lastEmittedTask = task;
        pendingTask.reset();
        return task;
    }

    if ((lastEmittedTask && *lastEmittedTask == task)
        || (pendingTask && *pendingTask == task)) {
        return std::nullopt;
    }

    const qint64 elapsed = timer.elapsed();
    if (elapsed - lastEmitElapsed >= intervalMs) {
        lastEmitElapsed = elapsed;
        lastEmittedTask = task;
        pendingTask.reset();
        return task;
    }

    // Inside the throttle window we keep only the latest task switch so the UI
    // eventually catches up without replaying every intermediate file.
    pendingTask = task;
    return std::nullopt;
}

std::optional<CurrentTaskNotifyThrottler::Task> CurrentTaskNotifyThrottler::takeReadyTask()
{
    QMutexLocker locker(&mutex);
    if (!timer.isValid() || !pendingTask) {
        return std::nullopt;
    }

    const qint64 elapsed = timer.elapsed();
    if (elapsed - lastEmitElapsed < intervalMs) {
        return std::nullopt;
    }

    return takePendingTaskUnlocked(elapsed);
}

std::optional<CurrentTaskNotifyThrottler::Task> CurrentTaskNotifyThrottler::flush()
{
    QMutexLocker locker(&mutex);
    if (!pendingTask) {
        return std::nullopt;
    }

    if (!timer.isValid()) {
        timer.start();
    }

    return takePendingTaskUnlocked(timer.elapsed());
}

std::optional<CurrentTaskNotifyThrottler::Task> CurrentTaskNotifyThrottler::takePendingTaskUnlocked(qint64 elapsed)
{
    if (!pendingTask) {
        return std::nullopt;
    }

    // Promote the coalesced task to the latest emitted state and clear the
    // backlog in one step so callers observe a single trailing update.
    const Task task = *pendingTask;
    pendingTask.reset();
    lastEmitElapsed = elapsed;

    if (lastEmittedTask && *lastEmittedTask == task) {
        return std::nullopt;
    }

    lastEmittedTask = task;
    return task;
}

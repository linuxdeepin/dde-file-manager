// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CURRENTTASKNOTIFYTHROTTLER_H
#define CURRENTTASKNOTIFYTHROTTLER_H

#include "dfmplugin_fileoperations_global.h"

#include <QElapsedTimer>
#include <QMutex>
#include <QUrl>

#include <optional>

DPFILEOPERATIONS_BEGIN_NAMESPACE

class CurrentTaskNotifyThrottler
{
public:
    struct Task
    {
        QUrl sourceUrl;
        QUrl targetUrl;

        bool operator==(const Task &other) const
        {
            return sourceUrl == other.sourceUrl && targetUrl == other.targetUrl;
        }
    };

public:
    explicit CurrentTaskNotifyThrottler(qint64 intervalMs = 100);

    // Submit a task switch.
    // Returns a task only when the caller should emit immediately (leading edge).
    std::optional<Task> submit(const QUrl &sourceUrl, const QUrl &targetUrl);
    // Returns the latest coalesced task after the throttle window expires.
    // Callers typically poll this from an existing progress heartbeat.
    std::optional<Task> takeReadyTask();
    // Force delivery of the latest coalesced task, used when the worker is ending.
    std::optional<Task> flush();

private:
    std::optional<Task> takePendingTaskUnlocked(qint64 elapsed);

private:
    QMutex mutex;
    QElapsedTimer timer;
    qint64 intervalMs { 0 };
    qint64 lastEmitElapsed { 0 };
    std::optional<Task> lastEmittedTask;
    std::optional<Task> pendingTask;
};

DPFILEOPERATIONS_END_NAMESPACE

#endif   // CURRENTTASKNOTIFYTHROTTLER_H

// SPDX-FileCopyrightText: 2024 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TASKSTATE_H
#define TASKSTATE_H

#include "service_textindex_global.h"

#include <QMutex>
#include <QMutexLocker>
#include <QAtomicInteger>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class TaskState
{
public:
    TaskState() : m_running(false) { }

    bool isRunning() const
    {
        return m_running.loadAcquire();
    }

    void start()
    {
        m_running.storeRelease(true);
    }

    void stop()
    {
        m_running.storeRelease(false);
    }

private:
    QAtomicInteger<bool> m_running;
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // TASKSTATE_H

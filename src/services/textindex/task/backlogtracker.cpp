// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backlogtracker.h"

#include "state/indexstatestore.h"

#include <dconfig.h>

#include <memory>

SERVICETEXTINDEX_USE_NAMESPACE
DCORE_USE_NAMESPACE

namespace {
constexpr qint64 kDefaultBacklogThreshold = 5000;
constexpr qint64 kMinimumBacklogThreshold = 200;
constexpr qint64 kMaximumBacklogThreshold = 100000;
constexpr char kAnythingConfig[] = "org.deepin.anything";
constexpr char kPendingEventsKey[] = "pending_events_trigger_updating";
}

FilenameBacklogTracker::FilenameBacklogTracker(const IndexStateStore *stateStore)
    : m_stateStore(stateStore)
{
}

void FilenameBacklogTracker::onStartup()
{
    if (m_stateStore && m_stateStore->isCleanState() && m_stateStore->isBacklogExceeded())
        m_stateStore->setBacklogExceeded(false);
}

void FilenameBacklogTracker::update(qint64 queuedWork, qint64 runningWork, bool hasRunningTask)
{
    if (!m_stateStore)
        return;

    const qint64 pending = qMax<qint64>(0, queuedWork) + qMax<qint64>(0, runningWork);
    const bool exceeded = m_stateStore->isBacklogExceeded();
    if (!exceeded && pending >= threshold()) {
        m_stateStore->setBacklogExceeded(true);
        fmInfo() << "[FilenameBacklogTracker] Backlog exceeded threshold:" << pending;
    } else if (exceeded && pending == 0 && !hasRunningTask) {
        m_stateStore->setBacklogExceeded(false);
        fmInfo() << "[FilenameBacklogTracker] Backlog drained";
    }
}

void FilenameBacklogTracker::onFullScanFinished()
{
    if (m_stateStore)
        m_stateStore->setBacklogExceeded(false);
}

qint64 FilenameBacklogTracker::threshold() const
{
    // 阈值在运行中极少变化，且本函数在任务入队/完成/进度里程碑时被频繁调用，
    // 缓存避免每次都走 DConfig::create 的 DBus IPC 开销。
    if (m_cachedThreshold >= 0)
        return m_cachedThreshold;

    qint64 computed = kDefaultBacklogThreshold;
    const std::unique_ptr<DConfig> config(DConfig::create(kAnythingConfig, kAnythingConfig, QString(), nullptr));
    if (config && config->isValid()) {
        const qint64 configured = config->value(kPendingEventsKey, kDefaultBacklogThreshold).toLongLong();
        if (configured >= kMinimumBacklogThreshold && configured <= kMaximumBacklogThreshold)
            computed = configured;
    }

    m_cachedThreshold = computed;
    return m_cachedThreshold;
}

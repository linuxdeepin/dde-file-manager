// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKLOGTRACKER_H
#define BACKLOGTRACKER_H

#include "service_textindex_global.h"

#include <QtGlobal>

SERVICETEXTINDEX_BEGIN_NAMESPACE

class IndexStateStore;

class BacklogTracker
{
public:
    virtual ~BacklogTracker() = default;
    virtual void onStartup() = 0;
    virtual void update(qint64 queuedWork, qint64 runningWork, bool hasRunningTask) = 0;
    virtual void onFullScanFinished() = 0;
};

class FilenameBacklogTracker final : public BacklogTracker
{
public:
    explicit FilenameBacklogTracker(const IndexStateStore *stateStore);

    void onStartup() override;
    void update(qint64 queuedWork, qint64 runningWork, bool hasRunningTask) override;
    void onFullScanFinished() override;

private:
    qint64 threshold() const;

    const IndexStateStore *m_stateStore { nullptr };

    // 首次计算后缓存，避免每次评估积压都经 DConfig::create 触发 DBus IPC。
    // 线程约束：threshold() 仅经 update() ← TaskManager::updateBacklogState()
    // 被调用，其全部触发路径（DBus 方法入口、QueuedConnection 槽、主线程定时
    // 器）均在主线程执行，此缓存为单线程访问，无需同步；若未来引入跨线程
    // 调用方，需先补充同步。
    mutable qint64 m_cachedThreshold { -1 };
};

SERVICETEXTINDEX_END_NAMESPACE

#endif   // BACKLOGTRACKER_H

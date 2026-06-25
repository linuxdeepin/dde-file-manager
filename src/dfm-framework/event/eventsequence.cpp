// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/event/eventsequence.h>
#include <dfm-framework/lifecycle/lifecycle.h>

DPF_USE_NAMESPACE

bool EventSequence::traversal()
{
    return traversal(QVariantList());
}

bool EventSequence::traversal(const QVariantList &params)
{
    // Short-circuit during process shutdown: hook handlers may touch
    // Q_GLOBAL_STATIC singletons (Settings, Application) that are already
    // destroyed in unspecified atexit order. Hook chains originated from
    // plugin dtor / stop() paths are semantically meaningless at this point.
    if (Q_UNLIKELY(LifeCycle::isShuttingDown())) {
        qCDebug(logDPF) << "EventSequence: dropping traversal during shutdown";
        return false;
    }

    for (auto seq : list) {
        if (seq.handler(params))
            return true;
    }
    return false;
}

bool EventSequenceManager::unfollow(const QString &space, const QString &topic)
{
    Q_ASSERT(topic.startsWith(kHookStrategePrefix));
    return unfollow(EventConverter::convert(space, topic));
}

bool EventSequenceManager::unfollow(EventType type)
{
    QWriteLocker guard(&rwLock);
    if (sequenceMap.contains(type))
        return sequenceMap.remove(type) > 0;

    return false;
}

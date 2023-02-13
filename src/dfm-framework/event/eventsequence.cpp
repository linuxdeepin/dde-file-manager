// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <dfm-framework/event/eventsequence.h>

DPF_USE_NAMESPACE

bool EventSequence::traversal()
{
    return traversal(QVariantList());
}

bool EventSequence::traversal(const QVariantList &params)
{
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

// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef WATCHERCACHE_P_H
#define WATCHERCACHE_P_H

#include <dfm-base/utils/watchercache.h>
#include <dfm-base/utils/threadcontainer.h>

#include <QMutex>
#include <QMap>

namespace dfmbase {
class WatcherCachePrivate
{
    friend class WatcherCache;
    WatcherCache *const q;
    DThreadHash<QUrl, QSharedPointer<AbstractFileWatcher>> watchers;
    DThreadList<QString> disableCahceSchemes;

public:
    explicit WatcherCachePrivate(WatcherCache *qq);
    virtual ~WatcherCachePrivate();
};
}

#endif   // WATCHERCACHE_P_H

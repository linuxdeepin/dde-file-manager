// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DABSTRACTFILEWATCHER_P_H
#define DABSTRACTFILEWATCHER_P_H

#include "durl.h"
#include "dabstractfilewatcher.h"

class DAbstractFileWatcherPrivate
{
public:
    explicit DAbstractFileWatcherPrivate(DAbstractFileWatcher *qq);
    virtual ~DAbstractFileWatcherPrivate() {}
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool handleGhostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType1 signal, const DUrl &arg1);
    virtual bool handleGhostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType2 signal, const DUrl &arg1, const DUrl &arg2);
    virtual bool handleGhostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType3 signal, const DUrl &arg1, int isExternalSource);

    DAbstractFileWatcher *q_ptr;

    DUrl url;
    bool started = false;
    static QList<DAbstractFileWatcher *> watcherList;

    Q_DECLARE_PUBLIC(DAbstractFileWatcher)
};

#endif // DABSTRACTFILEWATCHER_P_H

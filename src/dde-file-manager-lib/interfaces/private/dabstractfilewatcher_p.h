/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
 *             hujianzhong<hujianzhong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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

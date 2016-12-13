/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#ifndef DABSTRACTFILEWATCHER_P_H
#define DABSTRACTFILEWATCHER_P_H

#include "durl.h"
#include "dabstractfilewatcher.h"

class DAbstractFileWatcherPrivate
{
public:
    DAbstractFileWatcherPrivate(DAbstractFileWatcher *qq);

    virtual bool start() = 0;
    virtual bool stop() = 0;

    DAbstractFileWatcher *q_ptr;

    DUrl url;
    bool started = false;
    static QList<DAbstractFileWatcher*> watcherList;

    Q_DECLARE_PUBLIC(DAbstractFileWatcher)
};

#endif // DABSTRACTFILEWATCHER_P_H

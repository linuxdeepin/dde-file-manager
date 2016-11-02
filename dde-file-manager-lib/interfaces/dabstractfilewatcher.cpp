/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "dabstractfilewatcher.h"
#include "private/dabstractfilewatcher_p.h"

#include <QEvent>
#include <QDebug>

DAbstractFileWatcherPrivate::DAbstractFileWatcherPrivate(DAbstractFileWatcher *qq)
    : q_ptr(qq)
{

}

DAbstractFileWatcher::DAbstractFileWatcher(const DUrl &url, QObject *parent)
    : DAbstractFileWatcher(*new DAbstractFileWatcherPrivate(this), url, parent)
{

}

DAbstractFileWatcher::~DAbstractFileWatcher()
{
    Q_D(const DAbstractFileWatcher);

    Q_ASSERT_X(!d->started, "DAbstractFileWatcher::~DAbstractFileWatcher()", "Watcher is started, Plase call stopWatcher()");
}

DUrl DAbstractFileWatcher::fileUrl() const
{
    Q_D(const DAbstractFileWatcher);

    return d->url;
}

bool DAbstractFileWatcher::startWatcher()
{
    Q_D(DAbstractFileWatcher);

    if (d->started)
        return true;

    if (start()) {
        d->started = true;

        return true;
    }

    return false;
}

bool DAbstractFileWatcher::stopWatcher()
{
    Q_D(DAbstractFileWatcher);

    if (!d->started)
        return false;

    if (stop()) {
        d->started = false;

        return true;
    }

    return false;
}

DAbstractFileWatcher::DAbstractFileWatcher(DAbstractFileWatcherPrivate &dd,
                                           const DUrl &url, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    Q_ASSERT(url.isValid());

    d_ptr->url = url;
}

bool DAbstractFileWatcher::event(QEvent *event)
{
    if (event->type() == QEvent::DeferredDelete) {
        stopWatcher();
    }

    return QObject::event(event);
}

#include "moc_dabstractfilewatcher.cpp"

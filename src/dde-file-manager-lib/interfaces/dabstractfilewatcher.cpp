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

#include "dabstractfilewatcher.h"
#include "private/dabstractfilewatcher_p.h"

#include <QEvent>
#include <QDebug>

QList<DAbstractFileWatcher *> DAbstractFileWatcherPrivate::watcherList;
DAbstractFileWatcherPrivate::DAbstractFileWatcherPrivate(DAbstractFileWatcher *qq)
    : q_ptr(qq)
{

}

bool DAbstractFileWatcherPrivate::handleGhostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType1 signal, const DUrl &arg1)
{

    if (url == targetUrl || url == arg1) {
        (q_ptr->*signal)(arg1);

        return true;
    }

    return false;
}

bool DAbstractFileWatcherPrivate::handleGhostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType3 signal, const DUrl &arg1, int isExternalSource)
{

    if (url == targetUrl || url == arg1) {
        (q_ptr->*signal)(arg1, isExternalSource);

        return true;
    }

    return false;
}

bool DAbstractFileWatcherPrivate::handleGhostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType2 signal, const DUrl &arg1, const DUrl &arg2)
{

    if (url == targetUrl || url == arg1 || url == arg2) {
        (q_ptr->*signal)(arg1, arg2);

        return true;
    }

    return false;
}

DAbstractFileWatcher::~DAbstractFileWatcher()
{
    stopWatcher();
    DAbstractFileWatcherPrivate::watcherList.removeOne(this);
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

    if (d->start()) {
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

    if (d->stop()) {
        d->started = false;

        return true;
    }

    return false;
}

bool DAbstractFileWatcher::restartWatcher()
{
    bool ok = stopWatcher();
    return ok && startWatcher();
}

void DAbstractFileWatcher::setEnabledSubfileWatcher(const DUrl &subfileUrl, bool enabled)
{
    Q_UNUSED(subfileUrl)
    Q_UNUSED(enabled)
}

bool DAbstractFileWatcher::ghostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType1 signal, const DUrl &arg1)
{
    if (!signal)
        return false;

    bool ok = false;

    for (DAbstractFileWatcher *watcher : DAbstractFileWatcherPrivate::watcherList) {
        if (watcher->d_func()->handleGhostSignal(targetUrl, signal, arg1))
            ok = true;
    }

    return ok;
}

bool DAbstractFileWatcher::ghostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType3 signal, const DUrl &arg1, const int isExternalSource)
{
    if (!signal)
        return false;

    bool ok = false;

    for (DAbstractFileWatcher *watcher : DAbstractFileWatcherPrivate::watcherList) {
        if (watcher->d_func()->handleGhostSignal(targetUrl, signal, arg1, isExternalSource))
            ok = true;
    }

    return ok;
}

bool DAbstractFileWatcher::ghostSignal(const DUrl &targetUrl, DAbstractFileWatcher::SignalType2 signal, const DUrl &arg1, const DUrl &arg2)
{
    if (!signal)
        return false;

    bool ok = false;

    for (DAbstractFileWatcher *watcher : DAbstractFileWatcherPrivate::watcherList) {
        if (watcher->d_func()->handleGhostSignal(targetUrl, signal, arg1, arg2))
            ok = true;
    }

    return ok;
}

DAbstractFileWatcher::DAbstractFileWatcher(DAbstractFileWatcherPrivate &dd,
                                           const DUrl &url, QObject *parent)
    : QObject(parent)
    , d_ptr(&dd)
{
    Q_ASSERT(url.isValid());

//    qDebug() << "add file watcher for: " << url;

    d_ptr->url = url;
    DAbstractFileWatcherPrivate::watcherList << this;
}

#include "moc_dabstractfilewatcher.cpp"

/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
 *
 * Maintainer: liuyangming<liuyangming@uniontech.com>
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
#include "trashfilewatcher.h"
#include "utils/trashhelper.h"
#include "private/trashfilewatcher_p.h"
#include "dfm-base/base/schemefactory.h"

#include "base/urlroute.h"

#include <dfm-io/core/dwatcher.h>
#include <dfmio_global.h>
#include <dfmio_register.h>
#include <dfm-io/core/diofactory.h>

#include <QEvent>
#include <QDir>
#include <QDebug>
#include <QApplication>

DFMBASE_USE_NAMESPACE
DPTRASH_BEGIN_NAMESPACE

TrashFileWatcherPrivate::TrashFileWatcherPrivate(const QUrl &fileUrl, TrashFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool TrashFileWatcherPrivate::start()
{
    return proxy && proxy->startWatcher();
}

bool TrashFileWatcherPrivate::stop()
{
    return proxy && proxy->stopWatcher();
}

void TrashFileWatcherPrivate::initFileWatcher()
{
    const QUrl &watchUrl = QUrl::fromLocalFile(TrashHelper::toLocalFile(path).path());

    proxy = WatcherFactory::create<AbstractFileWatcher>(watchUrl);
    if (!proxy) {
        qWarning("watcher create failed.");
        abort();
    }
}

void TrashFileWatcherPrivate::initConnect()
{
    connect(proxy.data(), &AbstractFileWatcher::fileDeleted, static_cast<TrashFileWatcher *>(q), &TrashFileWatcher::onFileDeleted);
    connect(proxy.data(), &AbstractFileWatcher::fileAttributeChanged, static_cast<TrashFileWatcher *>(q), &TrashFileWatcher::onFileAttributeChanged);
    connect(proxy.data(), &AbstractFileWatcher::subfileCreated, static_cast<TrashFileWatcher *>(q), &TrashFileWatcher::onSubfileCreated);
}

TrashFileWatcher::TrashFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new TrashFileWatcherPrivate(url, this), parent)
{
    dptr = static_cast<TrashFileWatcherPrivate *>(d.data());
    dptr->initFileWatcher();
    dptr->initConnect();
}

TrashFileWatcher::~TrashFileWatcher()
{
}

void TrashFileWatcher::onFileDeleted(const QUrl &url)
{
    QUrl newUrl = TrashHelper::fromLocalFile(url.path());

    emit fileDeleted(newUrl);
}

void TrashFileWatcher::onFileAttributeChanged(const QUrl &url)
{
    QUrl newUrl = TrashHelper::fromLocalFile(url.path());
    emit fileAttributeChanged(newUrl);
}

void TrashFileWatcher::onSubfileCreated(const QUrl &url)
{
    QUrl newUrl = TrashHelper::fromLocalFile(url.path());
    emit subfileCreated(newUrl);
}

DPTRASH_END_NAMESPACE

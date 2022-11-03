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
#include <dfm-io/dfmio_global.h>
#include <dfm-io/dfmio_register.h>
#include <dfm-io/core/diofactory.h>

#include <QEvent>
#include <QDir>
#include <QDebug>
#include <QApplication>

DFMBASE_USE_NAMESPACE
namespace dfmplugin_trash {

TrashFileWatcherPrivate::TrashFileWatcherPrivate(const QUrl &fileUrl, TrashFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool TrashFileWatcherPrivate::start()
{
    if (watcher.isNull())
        return false;
    started = watcher->start();
    if (!started)
        qWarning() << "watcher start failed, error: " << watcher->lastError().errorMsg();
    return started;
}

bool TrashFileWatcherPrivate::stop()
{
    if (watcher.isNull())
        return false;
    started = watcher->stop();
    return started;
}

void TrashFileWatcherPrivate::initFileWatcher()
{
    const QUrl &trashUrl = url;
    QSharedPointer<DIOFactory> factory = produceQSharedIOFactory(trashUrl.scheme(), static_cast<QUrl>(trashUrl));
    if (!factory) {
        qWarning("create factory failed.");
        abort();
    }

    watcher = factory->createWatcher();
    if (!watcher) {
        qWarning("watcher create failed.");
        abort();
    }
}

void TrashFileWatcherPrivate::initConnect()
{
    connect(watcher.data(), &DWatcher::fileChanged, q, &AbstractFileWatcher::fileAttributeChanged);
    connect(watcher.data(), &DWatcher::fileDeleted, q, &AbstractFileWatcher::fileDeleted);
    connect(watcher.data(), &DWatcher::fileAdded, q, &AbstractFileWatcher::subfileCreated);
    connect(watcher.data(), &DWatcher::fileRenamed, q, &AbstractFileWatcher::fileRename);
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

}

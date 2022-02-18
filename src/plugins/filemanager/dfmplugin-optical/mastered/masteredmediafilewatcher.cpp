/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "masteredmediafilewatcher.h"
#include "masteredmediafilewatcher_p.h"
#include "utils/opticalhelper.h"

#include "dfm-base/base/schemefactory.h"

DFMBASE_USE_NAMESPACE
DPOPTICAL_USE_NAMESPACE

MasteredMediaFileWatcherPrivate::MasteredMediaFileWatcherPrivate(const QUrl &fileUrl, MasteredMediaFileWatcher *qq)
    : AbstractFileWatcherPrivate(fileUrl, qq)
{
}

bool MasteredMediaFileWatcherPrivate::start()
{
    return (proxyOnDisk ? proxyOnDisk->startWatcher() : true) && proxyStaging && proxyStaging->startWatcher();
}

bool MasteredMediaFileWatcherPrivate::stop()
{
    return (proxyOnDisk ? proxyOnDisk->startWatcher() : true) && proxyStaging && proxyStaging->stopWatcher();
}

MasteredMediaFileWatcher::MasteredMediaFileWatcher(const QUrl &url, QObject *parent)
    : AbstractFileWatcher(new MasteredMediaFileWatcherPrivate(url, this), parent)
{
    dptr = dynamic_cast<MasteredMediaFileWatcherPrivate *>(d.data());
    QUrl urlStaging { OpticalHelper::localStagingFile(url) };
    if (!urlStaging.isValid() || urlStaging.isEmpty())
        return;

    dptr->proxyStaging = WacherFactory::create<AbstractFileWatcher>(urlStaging);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileAttributeChanged,
            this, &MasteredMediaFileWatcher::onFileAttributeChanged);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileDeleted,
            this, &MasteredMediaFileWatcher::onFileDeleted);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::fileRename,
            this, &MasteredMediaFileWatcher::onFileRename);
    connect(dptr->proxyStaging.data(), &AbstractFileWatcher::subfileCreated,
            this, &MasteredMediaFileWatcher::onSubfileCreated);

    dptr->proxyOnDisk.clear();

    // TODO(zhangs): proxyOnDisk wathcer
}

void MasteredMediaFileWatcher::onFileDeleted(const QUrl &url)
{
    QUrl realUrl { OpticalHelper::tansToBurnFile(url) };
    emit fileDeleted(realUrl);
}

void MasteredMediaFileWatcher::onFileAttributeChanged(const QUrl &url)
{
    QUrl realUrl { OpticalHelper::tansToBurnFile(url) };
    emit fileAttributeChanged(realUrl);
}

void MasteredMediaFileWatcher::onFileRename(const QUrl &fromUrl, const QUrl &toUrl)
{
    QUrl realFromUrl { OpticalHelper::tansToBurnFile(fromUrl) };
    QUrl realToUrl { OpticalHelper::tansToBurnFile(toUrl) };

    emit fileRename(realFromUrl, realToUrl);
}

void MasteredMediaFileWatcher::onSubfileCreated(const QUrl &url)
{
    QUrl realUrl { OpticalHelper::tansToBurnFile(url) };
    emit subfileCreated(realUrl);
}

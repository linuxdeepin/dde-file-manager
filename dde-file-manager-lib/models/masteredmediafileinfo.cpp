/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
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
#include "masteredmediafileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "app/define.h"
#include "deviceinfo/udisklistener.h"
#include "dfileservices.h"
#include <QRegularExpression>
#include "controllers/masteredmediacontroller.h"

MasteredMediaFileInfo::MasteredMediaFileInfo(const DUrl &url)
    : DAbstractFileInfo(url)
{
    QRegularExpression re("^(.*)/(disk_files|staging_files)/(.*)$");
    QString device(url.path());
    auto rem=re.match(device);
    if (rem.hasMatch()) {
        auto dev = deviceListener->getDeviceByDevicePath(rem.captured(1));
        //udisks should be used in order to detect blank media properly
        if (dev->getMountPointUrl().scheme() == BURN_SCHEME && rem.captured(2) == "disk_files") {
            //blank media contains no files on disc, duh!
            return;
        }
        if(rem.captured(2) == "disk_files")
            m_parentUrl = DUrl(dev->getMountPoint() + rem.captured(3));
        else
            m_parentUrl = DUrl(MasteredMediaController::getStagingFolder(url));
        setProxy(DFileService::instance()->createFileInfo(Q_NULLPTR, m_parentUrl));
    }
}

bool MasteredMediaFileInfo::exists() const
{
    Q_D(const DAbstractFileInfo);

    return !d->proxy || d->proxy->exists();
}

bool MasteredMediaFileInfo::isReadable() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return true;

    return d->proxy->isReadable();
}

bool MasteredMediaFileInfo::isDir() const
{
    Q_D(const DAbstractFileInfo);

    return !d->proxy || d->proxy->isDir();
}

int MasteredMediaFileInfo::filesCount() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return 0;

    return d->proxy->filesCount();
}

DUrl MasteredMediaFileInfo::parentUrl() const
{
    return m_parentUrl;
}

bool MasteredMediaFileInfo::canRedirectionFileUrl() const
{
    return !isDir();
}

DUrl MasteredMediaFileInfo::redirectedFileUrl() const
{
    Q_D(const DAbstractFileInfo);
    return d->proxy->fileUrl();
}

bool MasteredMediaFileInfo::canIteratorDir() const
{
    Q_D(const DAbstractFileInfo);

    return !d->proxy;
}

QString MasteredMediaFileInfo::toLocalFile() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->toLocalFile();
}

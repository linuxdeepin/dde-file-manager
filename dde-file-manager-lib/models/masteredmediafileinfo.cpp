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
#include "disomaster.h"
#include "dblockdevice.h"
#include "ddiskdevice.h"
#include "ddiskmanager.h"
#include <QRegularExpression>
#include "controllers/masteredmediacontroller.h"

MasteredMediaFileInfo::MasteredMediaFileInfo(const DUrl &url)
    : DAbstractFileInfo(url)
{
    QRegularExpression re("^(.*)/(disk_files|staging_files)/(.*)$");
    QString device(url.path());
    auto rem = re.match(device);
    if (rem.hasMatch()) {
        QString udiskspath = rem.captured(1);
        udiskspath.replace("/dev/", "/org/freedesktop/UDisks2/block_devices/");
        QSharedPointer<DBlockDevice> blkdev(DDiskManager::createBlockDevice(udiskspath));
        QSharedPointer<DDiskDevice> diskdev(DDiskManager::createDiskDevice(blkdev->drive()));
        if (diskdev->opticalBlank() && rem.captured(2) == "disk_files") {
            //blank media contains no files on disc, duh!
            return;
        }
        if(rem.captured(2) == "disk_files") {
            Q_ASSERT(blkdev->mountPoints().size() > 0);
            QString mntpoint = QString(blkdev->mountPoints().front());
            if (*mntpoint.rbegin() != '/') {
                mntpoint += '/';
            }

            m_backerUrl = DUrl(mntpoint + rem.captured(3));
            m_backerUrl.setScheme(FILE_SCHEME);
        } else {
            m_backerUrl = DUrl(MasteredMediaController::getStagingFolder(url));
        }
        setProxy(DFileService::instance()->createFileInfo(Q_NULLPTR, m_backerUrl));
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

bool MasteredMediaFileInfo::isWritable() const
{
    return canDrop();
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

QFileInfo MasteredMediaFileInfo::toQFileInfo() const
{
    return QFileInfo(m_backerUrl.path());
}

DUrl MasteredMediaFileInfo::parentUrl() const
{
    DUrl ret = this->DAbstractFileInfo::parentUrl();
    ret.setPath(ret.path().replace("staging_files", "disk_files"));
    if (!ret.path().endsWith('/'))
        ret.setPath(ret.path() + '/');
    return ret;
}

QVector<MenuAction> MasteredMediaFileInfo::menuActionList(MenuType type) const
{
    QVector<MenuAction> ret = DAbstractFileInfo::menuActionList(type);
    ret.removeAll(MenuAction::TagInfo);
    ret.removeAll(MenuAction::TagFilesUseColor);
    return ret;
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

    return "";
}

bool MasteredMediaFileInfo::canDrop() const
{
    QRegularExpression re("^(.*)/(disk_files|staging_files)/(.*)$");
    auto rem = re.match(this->fileUrl().path());
    if (rem.hasMatch()) {
        return ISOMaster->getDevicePropertyCached(rem.captured(1)).avail > 0;
    }
    else {
        return false;
    }
}

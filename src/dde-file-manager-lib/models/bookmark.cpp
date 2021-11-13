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

#include "bookmark.h"
#include "dfileservices.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "dstorageinfo.h"
#include "private/dabstractfileinfo_p.h"

#include <QIcon>
#include <QUrlQuery>

BookMark::BookMark(const DUrl &url)
    : DAbstractFileInfo(url)
{
    DUrl target = url.bookmarkTargetUrl();

    if (target.isValid()) {
        setProxy(DFileService::instance()->createFileInfo(nullptr, target));
    }
}

BookMark::BookMark(const QString &name, const DUrl &sourceUrl)
    : BookMark(DUrl::fromBookMarkFile(sourceUrl, name))
{

}

BookMark::~BookMark()
{

}

DUrl BookMark::sourceUrl() const
{
    return fileUrl().bookmarkTargetUrl();
}

QString BookMark::getName() const
{
    return fileUrl().bookmarkName();
}

QString BookMark::getMountPoint() const
{
    return udisksMountPoint;
}

/*!
 * \brief Check if bookmark target file exist
 *
 * This is a known interface misunderstand, it should be implemented as check
 * if a bookmark is exist. This behavior should be fixed later.
 *
 * \return bookmark target file exist or not.
 */
bool BookMark::exists() const
{
    // Check if it is a local file first
    if (!mountPoint.isEmpty() && !locateUrl.isEmpty()) {
        DUrl mountPointUrl(mountPoint);
        QString mountPointPath = mountPointUrl.path();

        if (mountPointUrl.scheme() == DEVICE_SCHEME && mountPointPath.startsWith("/dev")) {
            QStringList paths = DDiskManager::resolveDeviceNode(mountPointUrl.path(),{});
            if (paths.isEmpty()) {
                return false;
            }
            udisksDBusPath = paths.first();
            QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(udisksDBusPath));
            udisksMountPoint.clear();
            for (auto& mp : blDev->mountPoints()) {
                QString mps(mp);
                if (sourceUrl().path().startsWith(mps)) {
                    udisksMountPoint = mps;
                    break;
                }
            }
        }
    }

    if (!udisksMountPoint.isEmpty()) {
        const DAbstractFileInfoPointer &sourceInfo = DFileService::instance()->createFileInfo(nullptr, sourceUrl());
        return sourceInfo->exists();
    }

    // not a local file, assume it's exist.
    return true;
}

QString BookMark::fileDisplayName() const
{
    return getName();
}

bool BookMark::canRedirectionFileUrl() const
{
    //书签指向目录就是挂载目录本身时locateUrl为空，属于正常逻辑，这里无需判断locateUrl
    if (!mountPoint.isEmpty() /*&& !locateUrl.isEmpty()*/ && udisksMountPoint.isEmpty() && !udisksDBusPath.isEmpty()) {
        QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(udisksDBusPath));
        udisksMountPoint = blDev->mount({});
    }

    return fileUrl() != DUrl(BOOKMARK_ROOT);
}

DUrl BookMark::redirectedFileUrl() const
{
    //书签指向目录就是挂载目录本身时locateUrl为空，属于正常逻辑，这里无需判断locateUrl
    if (!mountPoint.isEmpty() /*&& !locateUrl.isEmpty()*/) {
        DUrl mountPointUrl(mountPoint);
        QString schemeStr = mountPointUrl.scheme();
        //为防止locateUrl传入QUrl被转码，locateUrl统一保存为base64,这里需要从base64转回来。
        QByteArray ba = QByteArray::fromBase64(locateUrl.toLocal8Bit());

        if (!udisksDBusPath.isEmpty() && !udisksMountPoint.isEmpty()) {
//            return DUrl::fromLocalFile(udisksMountPoint + locateUrl);
            return DUrl::fromLocalFile(udisksMountPoint + QString(ba));
        }

        if (schemeStr == SMB_SCHEME || schemeStr == FTP_SCHEME || schemeStr == SFTP_SCHEME) {
//            mountPointUrl.setPath(mountPointUrl.path() + locateUrl);
            mountPointUrl.setPath(mountPointUrl.path() + QString(ba));
            return mountPointUrl;
        }
    }

    return sourceUrl();
}

DUrl BookMark::parentUrl() const
{
    return DUrl(BOOKMARK_ROOT);
}

DUrl BookMark::getUrlByNewFileName(const QString &name) const
{
    DUrl new_url = fileUrl();
    new_url.setBookmarkName(name);

    return new_url;
}

QDateTime BookMark::created() const
{
    return m_created;
}

QDateTime BookMark::lastModified() const
{
    return m_lastModified;
}

bool BookMark::canDrop() const
{
    Q_D(const DAbstractFileInfo);
    if (d->proxy && d->proxy->isDir() && !d->proxy->isWritable())
        return false;
    return DAbstractFileInfo::canDrop();
}

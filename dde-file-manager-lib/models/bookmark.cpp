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

#include "bookmark.h"
#include "dfileservices.h"
#include "ddiskmanager.h"
#include "dblockdevice.h"
#include "dstorageinfo.h"

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
            mountPointPath.replace("dev", "org/freedesktop/UDisks2/block_devices");
            udisksDBusPath = mountPointPath;
            QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(mountPointPath));
            udisksMountPoint = blDev->mountPoints().isEmpty() ? QString() : blDev->mountPoints().first();
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
    if (!mountPoint.isEmpty() && !locateUrl.isEmpty() && udisksMountPoint.isEmpty() && !udisksDBusPath.isEmpty()) {
        QScopedPointer<DBlockDevice> blDev(DDiskManager::createBlockDevice(udisksDBusPath));
        udisksMountPoint = blDev->mount({});
    }

    return fileUrl() != DUrl(BOOKMARK_ROOT);
}

DUrl BookMark::redirectedFileUrl() const
{
    if (!mountPoint.isEmpty() && !locateUrl.isEmpty()) {
        DUrl mountPointUrl(mountPoint);
        QString schemeStr = mountPointUrl.scheme();

        if (!udisksDBusPath.isEmpty() && !udisksMountPoint.isEmpty()) {
            return DUrl::fromLocalFile(udisksMountPoint + locateUrl);
        }

        if (schemeStr == SMB_SCHEME || schemeStr == FTP_SCHEME || schemeStr == SFTP_SCHEME) {
            mountPointUrl.setPath(mountPointUrl.path() + locateUrl);
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

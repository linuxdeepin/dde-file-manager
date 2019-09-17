/*
 * Copyright (C) 2019 Deepin Technology Co., Ltd.
 *
 * Author:     Gary Wang <wzc782970009@gmail.com>
 *
 * Maintainer: Gary Wang <wangzichong@deepin.com>
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
#include "vaultfileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "dfileservices.h"
#include "controllers/vaultcontroller.h"

#include <QStandardPaths>

class VaultFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    VaultFileInfoPrivate(const DUrl &url, VaultFileInfo *qq) : DAbstractFileInfoPrivate(url, qq, true) {}
};

VaultFileInfo::VaultFileInfo(const DUrl &url)
    : DAbstractFileInfo(url)
{
    if (url.host() == "files") {
        // normal file map to..
        DUrl actualUrl = DUrl::fromLocalFile(VaultController::vaultToLocal(url));
        setProxy(DAbstractFileInfoPointer(DFileService::instance()->createFileInfo(nullptr, actualUrl)));
    }
}

bool VaultFileInfo::exists() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return d->proxy->exists();
    }

    return true;
}

DUrl VaultFileInfo::parentUrl() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return VaultController::localUrlToVault(d->proxy->parentUrl());
    }

    qDebug() << "FIXME: implement parentUrl() for VaultFileInfo";
    return DUrl();
}

QString VaultFileInfo::iconName() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return d->proxy->iconName();
    }

    return "drive-harddisk-encrypted"; // it's not really a harddisk.. maybe we should use a dfm scope vault icon.
}

QString VaultFileInfo::genericIconName() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return d->proxy->genericIconName();
    }

    return "drive-harddisk-encrypted";
}

DUrl VaultFileInfo::mimeDataUrl() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return d->proxy->mimeDataUrl();
    }

    return DAbstractFileInfo::mimeDataUrl();
}

bool VaultFileInfo::canRedirectionFileUrl() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return d->proxy->isDir() ? false : d->proxy->canRedirectionFileUrl();
    }

    return false;
}

DUrl VaultFileInfo::redirectedFileUrl() const
{
    Q_D(const VaultFileInfo);
    if (d->proxy) {
        return d->proxy->fileUrl();
    }

    return DAbstractFileInfo::redirectedFileUrl();
}

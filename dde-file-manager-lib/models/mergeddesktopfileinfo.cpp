/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
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

#include "mergeddesktopfileinfo.h"
#include "controllers/mergeddesktopcontroller.h"
#include "dfileservices.h"

#include "private/dabstractfileinfo_p.h"
#include "private/mergeddesktop_common_p.h"

#include <QIcon>
#include <QStandardPaths>

class VirtualEntryInfo : public DAbstractFileInfo
{
public:
    VirtualEntryInfo(const DUrl &url) : DAbstractFileInfo(url) {}

    QString iconName() const override
    {
        switch (MergedDesktopController::entryTypeByName(fileName())) {
        case DMD_APPLICATION:
            return QStringLiteral("folder-applications-stack");
        case DMD_DOCUMENT:
            return QStringLiteral("folder-documents-stack");
        case DMD_MUSIC:
            return QStringLiteral("folder-music-stack");
        case DMD_PICTURE:
            return QStringLiteral("folder-images-stack");
        case DMD_VIDEO:
            return QStringLiteral("folder-video-stack");
        case DMD_OTHER:
            return QStringLiteral("folder-stack");
        default:
            qWarning() << "VirtualEntryInfo::iconName() no matched branch (it can be a bug!)";
            qWarning() << "Url: " << fileUrl();
        }
        return QStringLiteral("folder-stack");
    }

    bool exists() const override
    {
        return true;
    }

    bool isDir() const override
    {
        return true;
    }

    bool isVirtualEntry() const override
    {
        return true;
    }

    bool canShare() const override
    {
        return false;
    }

    bool isReadable() const override
    {
        return true;
    }

    bool isWritable() const override
    {
        return true;
    }

    QString fileName() const override
    {
        QString path = fileUrl().path();

        if (path.startsWith(VIRTUALENTRY_PATH)) {
            if (path != VIRTUALENTRY_PATH) {
                return DAbstractFileInfo::fileName();
            } else {
                return "Entry";
            }
        } else if (path.startsWith(VIRTUALFOLDER_PATH)) {
            if (path != VIRTUALFOLDER_PATH) {
                return DAbstractFileInfo::fileName();
            } else {
                return "Folder";
            }
        } else if (path.startsWith(MERGEDDESKTOP_PATH)) {
            return "Merged Desktop";
        }

        return DAbstractFileInfo::fileName() + "(?)";
    }

    Qt::ItemFlags fileItemDisableFlags() const override
    {
        return Qt::ItemIsDragEnabled;
    }

    DAbstractFileInfo::CompareFunction compareFunByColumn(int) const override
    {
        return nullptr;
    }
};

class MergedDesktopFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    MergedDesktopFileInfoPrivate(const DUrl &url, MergedDesktopFileInfo *qq)
        : DAbstractFileInfoPrivate(url, qq, true) {}

    DUrl m_parentUrl;
};

MergedDesktopFileInfo::MergedDesktopFileInfo(const DUrl &url, const DUrl &parentUrl)
    : DAbstractFileInfo(*new MergedDesktopFileInfoPrivate(url, this))
{
    Q_D(MergedDesktopFileInfo);
    d->m_parentUrl = parentUrl;

    QString path = url.path();
    QString fileName = url.fileName();
    QString desktopPath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    QString realPath = desktopPath + QDir::separator() + fileName;

    if (path.startsWith(VIRTUALENTRY_PATH)) {
        if (path.split('/', QString::SkipEmptyParts).count() != 2) {
            setProxy(DAbstractFileInfoPointer(DFileService::instance()->createFileInfo(nullptr, DUrl::fromLocalFile(realPath))));
        } else {
            setProxy(DAbstractFileInfoPointer(new VirtualEntryInfo(url)));
        }
    } else if (path.startsWith(VIRTUALFOLDER_PATH)) {
        if (path != VIRTUALFOLDER_PATH) {
            setProxy(DAbstractFileInfoPointer(DFileService::instance()->createFileInfo(nullptr, DUrl::fromLocalFile(realPath))));
        } else {
            setProxy(DAbstractFileInfoPointer(new VirtualEntryInfo(url)));
        }
    } else if (path.startsWith(MERGEDDESKTOP_PATH)) {
        if (path == MERGEDDESKTOP_PATH) {
            setProxy(DAbstractFileInfoPointer(DFileService::instance()->createFileInfo(nullptr, DUrl::fromLocalFile(desktopPath))));
        } else {
            setProxy(DAbstractFileInfoPointer(new VirtualEntryInfo(url)));
        }
    }
}

DUrl MergedDesktopFileInfo::parentUrl() const
{
    Q_D(const MergedDesktopFileInfo);
    return d->m_parentUrl;
}

QString MergedDesktopFileInfo::iconName() const
{
    Q_D(const MergedDesktopFileInfo);
    if (d->proxy) {
        return d->proxy->iconName();
    }

    return DAbstractFileInfo::iconName();
}

QString MergedDesktopFileInfo::genericIconName() const
{
    Q_D(const MergedDesktopFileInfo);
    if (d->proxy) {
        return d->proxy->genericIconName();
    }

    return DAbstractFileInfo::genericIconName();
}

DUrl MergedDesktopFileInfo::mimeDataUrl() const
{
    Q_D(const MergedDesktopFileInfo);
    if (d->proxy) {
        return d->proxy->mimeDataUrl();
    }

    return DAbstractFileInfo::mimeDataUrl();
}

bool MergedDesktopFileInfo::canRedirectionFileUrl() const
{
    Q_D(const MergedDesktopFileInfo);
    if (d->proxy) {
        return true;
    }

    return DAbstractFileInfo::canRedirectionFileUrl();
}

DUrl MergedDesktopFileInfo::redirectedFileUrl() const
{
    Q_D(const MergedDesktopFileInfo);
    if (d->proxy) {
        return d->proxy->fileUrl();
    }

    return DAbstractFileInfo::redirectedFileUrl();
}

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

#include "private/dabstractfileinfo_p.h"

class MergedDesktopFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    MergedDesktopFileInfoPrivate(const DUrl &url, MergedDesktopFileInfo *qq)
        : DAbstractFileInfoPrivate(url, qq, true)
    {
    }
};

MergedDesktopFileInfo::MergedDesktopFileInfo(const DUrl &url)
    : DAbstractFileInfo(*new MergedDesktopFileInfoPrivate(url, this))
{

}

bool MergedDesktopFileInfo::isDir() const
{
    QString path = fileUrl().path();
    if (path == "/" || path == "/folder/" || path.startsWith("/entry") || path == "/mergeddesktop/") {
        return true;
    }

    // TODO: redir to real url?
    return DAbstractFileInfo::isDir();
}

bool MergedDesktopFileInfo::isVirtualEntry() const
{
    return true;
}

Qt::ItemFlags MergedDesktopFileInfo::fileItemDisableFlags() const
{
    if (isVirtualEntry()) {
        return Qt::ItemIsDragEnabled;
    }

    return DAbstractFileInfo::fileItemDisableFlags();
}

QString MergedDesktopFileInfo::fileName() const
{
    QString path = fileUrl().path();

    if (path.startsWith("/entry/")) {
        if (path != "/entry/") {
            return DAbstractFileInfo::fileName();
        } else {
            return "Entry";
        }
    } else if (path.startsWith("/folder/")) {
        if (path != "/folder/") {
            return DAbstractFileInfo::fileName();
        } else {
            return "Folder";
        }
    } else if (path.startsWith("/mergeddesktop/")) {
        return "Merged Desktop";
    }

    return "what";
}

QString MergedDesktopFileInfo::iconName() const
{
    if (fileUrl().path().startsWith("/entry/") && fileUrl().path() != "/entry/") {
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
            qWarning() << "MergedDesktopFileInfo::iconName() no matched branch, it must be a bug!";
        }
    }

    return DAbstractFileInfo::iconName();
}

DAbstractFileInfo::CompareFunction MergedDesktopFileInfo::compareFunByColumn(int) const
{
    return nullptr;
}

bool MergedDesktopFileInfo::exists() const
{
    return true;
}

bool MergedDesktopFileInfo::isReadable() const
{
    return true;
}

bool MergedDesktopFileInfo::isWritable() const
{
    return true;
}

bool MergedDesktopFileInfo::canShare() const
{
    return false;
}

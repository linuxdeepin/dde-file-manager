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

#include "arrangeddesktopfileinfo.h"
#include "controllers/arrangeddesktopcontroller.h"

#include "private/dabstractfileinfo_p.h"

class ArrangedDesktopFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    ArrangedDesktopFileInfoPrivate(const DUrl &url, ArrangedDesktopFileInfo *qq)
        : DAbstractFileInfoPrivate(url, qq, true)
    {
    }
};

ArrangedDesktopFileInfo::ArrangedDesktopFileInfo(const DUrl &url)
    : DAbstractFileInfo(*new ArrangedDesktopFileInfoPrivate(url, this))
{

}

bool ArrangedDesktopFileInfo::isDir() const
{
    if (fileUrl().path() == "/" || fileUrl().path() == "/folder/" || fileUrl().path().startsWith("/entry/")) {
        return true;
    }

    // TODO: redir to real url?
    return DAbstractFileInfo::isDir();
}

QString ArrangedDesktopFileInfo::fileName() const
{
    if (fileUrl().path().startsWith("/entry/") && fileUrl().path() != "/entry/") {
        return DAbstractFileInfo::fileName();
    }

    return "what";
}

QString ArrangedDesktopFileInfo::iconName() const
{
    if (fileUrl().path().startsWith("/entry/") && fileUrl().path() != "/entry/") {
        switch (ArrangedDesktopController::entryTypeByName(fileName())) {
        case DAD_APPLICATION:
            return QStringLiteral("folder-applications-stack");
        case DAD_DOCUMENT:
            return QStringLiteral("folder-documents-stack");
        case DAD_MUSIC:
            return QStringLiteral("folder-music-stack");
        case DAD_PICTURE:
            return QStringLiteral("folder-images-stack");
        case DAD_VIDEO:
            return QStringLiteral("folder-video-stack");
        case DAD_OTHER:
            return QStringLiteral("folder-stack");
        default:
            qWarning() << "ArrangedDesktopFileInfo::iconName() no matched branch, it must be a bug!";
        }
    }

    return DAbstractFileInfo::iconName();
}

bool ArrangedDesktopFileInfo::exists() const
{
    return true;
}

bool ArrangedDesktopFileInfo::isReadable() const
{
    return true;
}

bool ArrangedDesktopFileInfo::canShare() const
{
    return false;
}

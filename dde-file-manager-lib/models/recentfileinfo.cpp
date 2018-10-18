/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#include "recentfileinfo.h"
#include "dfileservices.h"
#include "dfilesystemmodel.h"
#include "dfileinfo.h"
#include "private/dfileinfo_p.h"

RecentFileInfo::RecentFileInfo(const DUrl &url)
    : DAbstractFileInfo(url)
{
      setProxy(DAbstractFileInfoPointer(new DFileInfo(DUrl::fromLocalFile(url.path()))));
}

bool RecentFileInfo::makeAbsolute()
{
    return true;
}

bool RecentFileInfo::exists() const
{
    return DAbstractFileInfo::exists() || QFile::exists(fileUrl().toLocalFile());
}

bool RecentFileInfo::isDir() const
{
    if (fileUrl().path() == "/") {
        return true;
    }

    return DAbstractFileInfo::isDir();
}

QFileDevice::Permissions RecentFileInfo::permissions() const
{
    QFileDevice::Permissions p = DAbstractFileInfo::permissions();

    p = p &(QFileDevice::WriteGroup | QFileDevice::WriteOwner | QFileDevice::WriteUser | QFileDevice::WriteOther);

    return p;
}

QVector<MenuAction> RecentFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actions;

    actions << MenuAction::DisplayAs;
    actions << MenuAction::SortBy;
    actions << MenuAction::SelectAll;

    return actions;
}

QList<int> RecentFileInfo::userColumnRoles() const
{
    static QList<int> userColumnRoles = QList<int>() << DFileSystemModel::FileUserRole + 1
                                                     << DFileSystemModel::FileSizeRole
                                                     << DFileSystemModel::FileMimeTypeRole;

    return userColumnRoles;
}

QVariant RecentFileInfo::userColumnDisplayName(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return QObject::tr("Path", "SearchFileInfo");

    return DAbstractFileInfo::userColumnDisplayName(userColumnRole);
}

QVariant RecentFileInfo::userColumnData(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1) {
        return fileUrl().fileName();
    }

    return DAbstractFileInfo::userColumnData(userColumnRole);
}

int RecentFileInfo::userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return -1;

    return DAbstractFileInfo::userColumnWidth(userColumnRole, fontMetrics);
}

QString RecentFileInfo::subtitleForEmptyFloder() const
{
    return QObject::tr("Folder is empty");
}

Qt::ItemFlags RecentFileInfo::fileItemDisableFlags() const
{
    return Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
}

DUrl RecentFileInfo::mimeDataUrl() const
{
    return DUrl(fileUrl().fragment());
}

bool RecentFileInfo::canRedirectionFileUrl() const
{
    return fileUrl().hasFragment();
}

DUrl RecentFileInfo::redirectedFileUrl() const
{
    return mimeDataUrl();
}

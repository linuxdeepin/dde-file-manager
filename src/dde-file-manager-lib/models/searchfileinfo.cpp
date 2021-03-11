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

#include "searchfileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "views/dfileview.h"

#include "dfileservices.h"

#include "dfilesystemmodel.h"

#include "app/define.h"

#include <QIcon>
#include <QDateTime>
#include <QUrlQuery>

namespace FileSortFunction {
COMPARE_FUN_DEFINE(absoluteFilePath, FilePath, SearchFileInfo)
}

SearchFileInfo::SearchFileInfo(const DUrl &url)
    : DAbstractFileInfo(url)
{
    d_func()->columnCompact = false;

    if (url.searchedFileUrl().isValid()) {
        m_parentUrl = url;
        m_parentUrl.setSearchedFileUrl(DUrl());

        setProxy(DFileService::instance()->createFileInfo(Q_NULLPTR, url.searchedFileUrl()));
    }
}

bool SearchFileInfo::exists() const
{
    Q_D(const DAbstractFileInfo);

    return !d->proxy || d->proxy->exists();
}

bool SearchFileInfo::isReadable() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return true;

    return d->proxy->isReadable();
}

bool SearchFileInfo::isDir() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return true;

    return d->proxy->isDir();
}

bool SearchFileInfo::isVirtualEntry() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->isVirtualEntry();

    return true;
}

int SearchFileInfo::filesCount() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return 0;

    return d->proxy->filesCount();
}

DUrl SearchFileInfo::parentUrl() const
{
    return m_parentUrl;
}

void SearchFileInfo::setColumnCompact(bool)
{

}

QList<int> SearchFileInfo::userColumnRoles() const
{
    static QList<int> userColumnRoles = QList<int>() << DFileSystemModel::FileDisplayNameRole
//                                                     << DFileSystemModel::FileUserRole + 1
//                                                     << DFileSystemModel::FileUserRole + 2
                                        << DFileSystemModel::FilePathRole
                                        << DFileSystemModel::FileLastModifiedRole
                                        << DFileSystemModel::FileSizeRole
                                        << DFileSystemModel::FileMimeTypeRole;

    return userColumnRoles;
}

QVariant SearchFileInfo::userColumnDisplayName(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return qApp->translate("DFileSystemModel",  "Path");
    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
        return qApp->translate("DFileSystemModel",  "Time modified");

    if (userColumnRole == DFileSystemModel::FilePathRole) {
        return qApp->translate("DFileSystemModel",  "Path");
    }

    return DAbstractFileInfo::userColumnDisplayName(userColumnRole);
}

QVariant SearchFileInfo::userColumnData(int userColumnRole) const
{
    Q_D(const DAbstractFileInfo);

    if (userColumnRole == DFileSystemModel::FileUserRole + 2) {
        return QVariant::fromValue(qMakePair(lastModifiedDisplayName(), qMakePair(sizeDisplayName(), mimeTypeDisplayName())));
    } else if (userColumnRole == DFileSystemModel::FilePathRole) {
        QString file_path;

        const DUrl &fileUrl = d->proxy->fileUrl();

        if (fileUrl.isLocalFile()) {
            file_path = absolutePath();
        } else {
            file_path = d->proxy->parentUrl().toString();
        }

        return file_path;
//        return QVariant::fromValue(QPair<QString, QString>(fileDisplayName(), file_path));
    }

    return DAbstractFileInfo::userColumnData(userColumnRole);
}

QList<int> SearchFileInfo::userColumnChildRoles(int column) const
{
    Q_UNUSED(column)

    QList<int> userColumnRoles{};
//    if (column == 0){
//        userColumnRoles << DFileSystemModel::FileDisplayNameRole
//                        << DFileSystemModel::FilePathRole;
//    }else if (column == 1){
//        userColumnRoles << DFileSystemModel::FileLastModifiedRole
//                        << DFileSystemModel::FileSizeRole
//                        << DFileSystemModel::FileMimeTypeRole;
//    }
    return userColumnRoles;
}

bool SearchFileInfo::columnDefaultVisibleForRole(int role) const
{
    Q_UNUSED(role)

    return true;
//    return (role == DFileSystemModel::FileUserRole + 1 || role == DFileSystemModel::FileUserRole + 2);
}

int SearchFileInfo::userColumnWidth(int userColumnRole, const QFontMetrics &fontMetrics) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return -1;

    return fontMetrics.width("0000/00/00 00:00:00");;
}

//int SearchFileInfo::userRowHeight(const QFontMetrics &fontMetrics) const
//{
//    return fontMetrics.height() * 2 + 10;
//}

MenuAction SearchFileInfo::menuActionByColumnRole(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FilePathRole) {
        return MenuAction::AbsolutePath;
    }
    return DAbstractFileInfo::menuActionByColumnRole(userColumnRole);
}

QList<int> SearchFileInfo::sortSubMenuActionUserColumnRoles() const
{
    QList<int> roles;
    roles << DFileSystemModel::FileDisplayNameRole
          << DFileSystemModel::FilePathRole
          << DFileSystemModel::FileLastModifiedRole
          << DFileSystemModel::FileSizeRole
          << DFileSystemModel::FileMimeTypeRole;
    return roles;
}

bool SearchFileInfo::canRedirectionFileUrl() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return true;

    const DAbstractFileInfoPointer &targetFileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchTargetUrl());

    return targetFileInfo && !targetFileInfo->canIteratorDir();
}

DUrl SearchFileInfo::redirectedFileUrl() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->redirectedFileUrl();

    return fileUrl().searchTargetUrl();
}

bool SearchFileInfo::canIteratorDir() const
{
    Q_D(const DAbstractFileInfo);

    return !d->proxy;
}

QVector<MenuAction> SearchFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    Q_D(const DAbstractFileInfo);

    QVector<MenuAction> actions;

    if (!d->proxy && type == SpaceArea) {
        actions << MenuAction::DisplayAs;
        actions << MenuAction::SortBy;
        actions << MenuAction::SelectAll;

        return actions;
    }

    actions = d->proxy->menuActionList(type);
    actions.insert(1, MenuAction::OpenFileLocation);

    return actions;
}

QSet<MenuAction> SearchFileInfo::disableMenuActionList() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return QSet<MenuAction>();

    QSet<MenuAction> actions = d->proxy->disableMenuActionList();

    actions << MenuAction::DecompressHere;

    if (path().isEmpty())
        actions << MenuAction::Property << MenuAction::OpenInTerminal;

    return actions;
}

bool SearchFileInfo::isEmptyFloder(const QDir::Filters &filters) const
{
    if (path().isEmpty())
        return false;

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchedFileUrl());

    return fileInfo && fileInfo->isEmptyFloder(filters);
}

DAbstractFileInfo::CompareFunction SearchFileInfo::compareFunByColumn(int columnRole) const
{
    if (columnRole == DFileSystemModel::FilePathRole)
        return FileSortFunction::compareFileListByFilePath;

    return DAbstractFileInfo::compareFunByColumn(columnRole);
}

bool SearchFileInfo::hasOrderly() const
{
    return false;
}

DUrl SearchFileInfo::getUrlByNewFileName(const QString &fileName) const
{
    Q_D(const DAbstractFileInfo);

    DUrl url = fileUrl();

    if (d->proxy)
        url.setSearchedFileUrl(d->proxy->getUrlByNewFileName(fileName));

    return url;
}

QString SearchFileInfo::loadingTip() const
{
    return QObject::tr("Searching...");
}

QString SearchFileInfo::subtitleForEmptyFloder() const
{
    return QObject::tr("No results");
}

QString SearchFileInfo::fileDisplayName() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->fileDisplayName();

    if (fileUrl().isSearchFile()) {
        return qApp->translate("Shortcut", "Search");
    }

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchTargetUrl());

    if (fileInfo)
        return fileInfo->fileDisplayName();

    return QString();
}

DUrl SearchFileInfo::mimeDataUrl() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->mimeDataUrl();

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchedFileUrl());

    if (info)
        return info->mimeDataUrl();

    return fileUrl().searchedFileUrl();
}

QString SearchFileInfo::toLocalFile() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->toLocalFile();

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchedFileUrl());

    if (info)
        return info->toLocalFile();

    return fileUrl().searchedFileUrl().toLocalFile();
}

QIcon SearchFileInfo::fileIcon() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->fileIcon();

    const DAbstractFileInfoPointer &info = DFileService::instance()->createFileInfo(Q_NULLPTR, fileUrl().searchedFileUrl());

    if (info)
        return info->fileIcon();


    return QIcon::fromTheme("search");
}

bool SearchFileInfo::canRename() const
{
    Q_D(const DAbstractFileInfo);
    if (d->proxy)
        return d->proxy->canRename();
    return DAbstractFileInfo::canRename();
}

bool SearchFileInfo::canDrop() const
{
    Q_D(const DAbstractFileInfo);
    //tag的虚拟目录不能drop
    if (d->proxy && d->proxy->fileUrl().isTaggedFile() && isVirtualEntry())
        return false;
    return DAbstractFileInfo::canDrop();
}

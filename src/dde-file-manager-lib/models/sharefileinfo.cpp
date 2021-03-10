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

#include "sharefileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "usershare/usersharemanager.h"
#include "dfileservices.h"
#include "singleton.h"

#include "controllers/trashmanager.h"
#include "dfileservices.h"
#include "controllers/pathmanager.h"
#include "controllers/bookmarkmanager.h"

#include "app/define.h"
#include "interfaces/dfmstandardpaths.h"
#include "dfilesystemmodel.h"
#include "singleton.h"

#include <QMimeType>
#include <QIcon>

ShareFileInfo::ShareFileInfo(const DUrl &url):
    DAbstractFileInfo(url)
{
    if (url.path() != "/")
        setProxy(DFileService::instance()->createFileInfo(Q_NULLPTR, DUrl::fromLocalFile(url.path())));
}

ShareFileInfo::~ShareFileInfo()
{

}

bool ShareFileInfo::exists() const
{
    Q_D(const DAbstractFileInfo);

    return !d->proxy || d->proxy->exists();
}

bool ShareFileInfo::isDir() const
{
    Q_D(const DAbstractFileInfo);

    return !d->proxy || d->proxy->isDir();
}

bool ShareFileInfo::canRename() const
{
    return false;
}

bool ShareFileInfo::isReadable() const
{
    return true;
}

bool ShareFileInfo::isWritable() const
{
    return false;
}

QString ShareFileInfo::fileDisplayName() const
{
    Q_D(const DAbstractFileInfo);
    if (systemPathManager->isSystemPath(fileUrl().toString()))
        return systemPathManager->getSystemPathDisplayNameByPath(fileUrl().toString());
    ShareInfo info = userShareManager->getShareInfoByPath(fileUrl().path());
    QString displayName = info.shareName();
    if (!displayName.isEmpty()) {
        return displayName;
    } else {
        if (d->proxy) {
            return d->proxy->fileDisplayName();
        }
    }
    return QString("");
}

//QFileDevice::Permissions ShareFileInfo::vpermissions() const
//{
//    QFileDevice::Permissions p = AbstractFileInfo::permissions();

//    p &= ~QFileDevice::WriteOwner;
//    p &= ~QFileDevice::WriteUser;
//    p &= ~QFileDevice::WriteGroup;
//    p &= ~QFileDevice::WriteOther;

//    return p;
//}

QVector<MenuAction> ShareFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;

    if(type == SpaceArea) {
        actionKeys << MenuAction::DisplayAs
                   << MenuAction::SortBy;
    } else if (type == SingleFile){
        if(isDir()){
            bool useRemoveBookmarkAction = false;
            DUrl schemeAlteredUrl = fileUrl();
            schemeAlteredUrl.setScheme(FILE_SCHEME);
            // FIXME: reimplement BookMark::exist() 's behavior and use it for check bookmark existance.
            //        after doing this, don't forget to remove the "bookmarkmanager.h" header file include.
            // if (DFileService::instance()->createFileInfo(nullptr, DUrl::fromBookMarkFile(fileUrl(), QString()))) {
            if (Singleton<BookMarkManager>::instance()->checkExist(DUrl::fromBookMarkFile(schemeAlteredUrl, QString()))) {
                useRemoveBookmarkAction = true;
            }
            actionKeys << MenuAction::Open
                       << MenuAction::OpenInNewWindow
                       << MenuAction::OpenInNewTab
                       << MenuAction::Separator
                       << (useRemoveBookmarkAction ? MenuAction::BookmarkRemove : MenuAction::AddToBookMark)
                       << MenuAction::UnShare;
        }
        actionKeys << MenuAction::Separator
                   << MenuAction::Property;

    }else if (type == MultiFiles || type == MultiFilesSystemPathIncluded){
        actionKeys << MenuAction::Open
                   << MenuAction::Separator
                   << MenuAction::Property;
    }

    return actionKeys;
}

QSet<MenuAction> ShareFileInfo::disableMenuActionList() const
{
    QSet<MenuAction> list;

    return list;
}

//QVariant ShareFileInfo::userColumnData(int userColumnRole) const
//{
//    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
//        return displayDeletionDate;

//    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
//        return originalFilePath;

//    return AbstractFileInfo::userColumnData(userColumnRole);
//}

//QVariant ShareFileInfo::userColumnDisplayName(int userColumnRole) const
//{
//    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
//        return QObject::tr("Time deleted");

//    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
//        return QObject::tr("Path", "TrashFileInfo");

//    return AbstractFileInfo::userColumnDisplayName(userColumnRole);
//}

//int ShareFileInfo::userColumnWidth(int userColumnRole) const
//{
//    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
//        return -1;

//    return AbstractFileInfo::userColumnWidth(userColumnRole);
//}

//AbstractFileInfo::sortFunction ShareFileInfo::sortFunByColumn(int columnRole) const
//{
//    if (columnRole == DFileSystemModel::FileUserRole + 1)
//        return FileSortFunction::sortFileListByDeletionDate;
//    else if (columnRole == DFileSystemModel::FileUserRole + 2)
//        return FileSortFunction::sortFileListBySourceFilePath;
//    else
//        return AbstractFileInfo::sortFunByColumn(columnRole);
//}

bool ShareFileInfo::columnDefaultVisibleForRole(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileLastModifiedRole)
        return false;

    return DAbstractFileInfo::columnDefaultVisibleForRole(userColumnRole);
}

MenuAction ShareFileInfo::menuActionByColumnRole(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return MenuAction::DeletionDate;

    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
        return MenuAction::SourcePath;

    return DAbstractFileInfo::menuActionByColumnRole(userColumnRole);
}

bool ShareFileInfo::canIteratorDir() const
{
    return false;
}

bool ShareFileInfo::makeAbsolute()
{
    return true;
}

DUrl ShareFileInfo::mimeDataUrl() const
{
    return DUrl::fromLocalFile(absoluteFilePath());
}

DUrl ShareFileInfo::parentUrl() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return DUrl::fromUserShareFile("/");

    return DUrl();
}

bool ShareFileInfo::isShared() const
{
    return true;
}

bool ShareFileInfo::canTag() const
{
    return false;
}

bool ShareFileInfo::isVirtualEntry() const
{
    Q_D(const DAbstractFileInfo);

    if (fileUrl() == DUrl(USERSHARE_ROOT))
        return true;

    return d->proxy && d->proxy->isVirtualEntry();
}

bool ShareFileInfo::canDrop() const
{
    Q_D(const DAbstractFileInfo);

    //if (fileUrl() == DUrl(USERSHARE_ROOT))
        //return false;

    return d->proxy && d->proxy->canDrop();
}

Qt::ItemFlags ShareFileInfo::fileItemDisableFlags() const
{
    return Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

QList<QIcon> ShareFileInfo::additionalIcon() const
{
    QList<QIcon> icons;
    icons << QIcon::fromTheme("emblem-shared", DFMGlobal::instance()->standardIcon(DFMGlobal::ShareIcon));
    if (isSymLink()) {
        icons << QIcon::fromTheme("emblem-symbolic-link", DFMGlobal::instance()->standardIcon(DFMGlobal::LinkIcon));
    }

    return icons;
}

bool ShareFileInfo::canRedirectionFileUrl() const
{
    Q_D(const DAbstractFileInfo);

    return d->proxy;
}

DUrl ShareFileInfo::redirectedFileUrl() const
{
    Q_D(const DAbstractFileInfo);

    return d->proxy ? d->proxy->fileUrl() : DUrl();
}

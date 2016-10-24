/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "sharefileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "usershare/usersharemanager.h"
#include "glob.h"
#include "dfileservices.h"
#include "widgets/singleton.h"

#include "controllers/trashmanager.h"
#include "dfileservices.h"
#include "controllers/pathmanager.h"

#include "app/define.h"
#include "interfaces/dfmstandardpaths.h"
#include "shutil/iconprovider.h"
#include "dfilesystemmodel.h"
#include "widgets/singleton.h"

#include <QMimeType>
#include <QSettings>

ShareFileInfo::ShareFileInfo(const DUrl &url):
    DAbstractFileInfo(url)
{
    ShareFileInfo::setUrl(url);
}

ShareFileInfo::~ShareFileInfo()
{

}

bool ShareFileInfo::isCanRename() const
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
    if (!displayName.isEmpty())
        return displayName;
    else{
        if (d->proxy){
            return d->proxy->fileDisplayName();
        }
    }
    return QString("");
}

void ShareFileInfo::setUrl(const DUrl &fileUrl)
{
    DAbstractFileInfo::setUrl(fileUrl);

    if (fileUrl.path() != "/")
        setProxy(DFileService::instance()->createFileInfo(DUrl::fromLocalFile(fileUrl.path())));
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
            actionKeys << MenuAction::Open
                       << MenuAction::OpenInNewWindow
                       << MenuAction::OpenInNewTab
                       << MenuAction::UnShare;
        }
        actionKeys << MenuAction::Separator
                   << MenuAction::Property;

    }else if (type == MultiFiles){
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

QAbstractItemView::SelectionMode ShareFileInfo::supportSelectionMode() const
{
    return QAbstractItemView::ExtendedSelection;
}

Qt::ItemFlags ShareFileInfo::fileItemDisableFlags() const
{
    return Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

QList<QIcon> ShareFileInfo::additionalIcon() const
{
    QList<QIcon> icons;
    icons << DFMGlobal::instance()->standardIcon(DFMGlobal::ShareIcon);
    if (isSymLink()) {
        icons << DFMGlobal::instance()->standardIcon(DFMGlobal::LinkIcon);
    }

    return icons;
}

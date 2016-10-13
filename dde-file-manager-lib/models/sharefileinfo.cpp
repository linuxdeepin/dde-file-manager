/**
 * Copyright (C) 2016 Deepin Technology Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 **/

#include "sharefileinfo.h"
#include "fileinfo.h"

#include "usershare/usersharemanager.h"
#include "glob.h"
#include "widgets/singleton.h"

#include "controllers/trashmanager.h"
#include "controllers/fileservices.h"
#include "controllers/pathmanager.h"

#include "app/global.h"
#include "interfaces/dfmstandardpaths.h"
#include "shutil/iconprovider.h"
#include "models/dfilesystemmodel.h"
#include "widgets/singleton.h"

#include <QMimeType>
#include <QSettings>

ShareFileInfo::ShareFileInfo()
{

}

ShareFileInfo::ShareFileInfo(const DUrl &url):
    AbstractFileInfo(url)
{

}

ShareFileInfo::ShareFileInfo(const QString &url):
    AbstractFileInfo(url)
{

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
    return true;
}

QString ShareFileInfo::displayName() const
{
    if (systemPathManager->isSystemPath(fileUrl().toString()))
        return systemPathManager->getSystemPathDisplayNameByPath(fileUrl().toString());

    return userShareManager->getShareNameByPath(fileUrl().path());
}

void ShareFileInfo::setUrl(const DUrl &fileUrl)
{

}

QIcon ShareFileInfo::fileIcon() const
{
    return fileIconProvider->getFileIcon(fileUrl(), mimeTypeName());
}

QMimeType ShareFileInfo::mimeType() const
{
    if (!data->mimeType.isValid()) {
        data->mimeType = FileInfo::mimeType(data->fileInfo.absoluteFilePath());
    }

    return data->mimeType;
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

QVector<MenuAction> ShareFileInfo::menuActionList(AbstractFileInfo::MenuType type) const
{
    QVector<MenuAction> actionKeys;

    if(type == SpaceArea) {
        actionKeys.reserve(7);

        actionKeys << MenuAction::DisplayAs
                   << MenuAction::SortBy;
    } else if (type == SingleFile){
        actionKeys.reserve(12);
        if(isDir()){
            actionKeys << MenuAction::OpenInNewWindow
                       << MenuAction::OpenInNewTab
                       << MenuAction::UnShare;
        }
        actionKeys << MenuAction::Separator
                   << MenuAction::Property;

    }else if (type == MultiFiles){
        actionKeys.reserve(12);
        actionKeys << MenuAction::Separator
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

    return AbstractFileInfo::columnDefaultVisibleForRole(userColumnRole);
}

MenuAction ShareFileInfo::menuActionByColumnRole(int userColumnRole) const
{
    if (userColumnRole == DFileSystemModel::FileUserRole + 1)
        return MenuAction::DeletionDate;

    if (userColumnRole == DFileSystemModel::FileUserRole + 2)
        return MenuAction::SourcePath;

    return AbstractFileInfo::menuActionByColumnRole(userColumnRole);
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
    return DUrl::fromLocalFile(data->fileInfo.absoluteFilePath());
}

bool ShareFileInfo::isShared() const
{
    return true;
}

QAbstractItemView::SelectionMode ShareFileInfo::supportSelectionMode() const
{
    return QAbstractItemView::SingleSelection;
}

Qt::ItemFlags ShareFileInfo::fileItemDisableFlags() const
{
    return Qt::ItemIsDragEnabled | Qt::ItemIsEditable;
}

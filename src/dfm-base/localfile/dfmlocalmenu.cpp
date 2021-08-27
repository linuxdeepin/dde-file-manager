/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liyigang<liyigang@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             yanghao<yanghao@uniontech.com>
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
#include "private/dfmlocalmenu_p.h"
#include "shutil/fileutils.h"
#include "base/dfmschemefactory.h"
#include "base/singleton.hpp"

DFMLocalMenuPrivate::DFMLocalMenuPrivate(const QString &filePath, DFMLocalMenu *qq)
    :m_fileInfo(DFMInfoFactory::instance().create<DFMLocalFileInfo>(filePath)),
      q_ptr(qq)
{

}

DFMLocalMenuPrivate::~DFMLocalMenuPrivate()
{

}

DFMLocalMenu::DFMLocalMenu(const QString &filePath)
     :d_ptr(new DFMLocalMenuPrivate(filePath, this))
{

}

QVector<DFMLocalMenu::MenuAction> DFMLocalMenu::menuActionList(MenuType type) const
{
    Q_D(const DFMLocalMenu);
    QVector<MenuAction> actionKeys;

    if (type == SpaceArea) {
        actionKeys.reserve(9);

        actionKeys << MenuAction::NewFolder
                   << MenuAction::NewDocument
                   << MenuAction::Separator
                   << MenuAction::DisplayAs
                   << MenuAction::SortBy
                   << MenuAction::OpenAsAdmin
                   << MenuAction::OpenInTerminal
                   << MenuAction::Separator
                   << MenuAction::Paste
                   << MenuAction::SelectAll
                   << MenuAction::Separator
                   << MenuAction::Property;
    } else if (type == SingleFile) {

        if (d->m_fileInfo->isDir() /*todo:待DFMLocalFileInfo提供对应接口&& systemPathManager->isSystemPath(d->m_fileInfo->filePath())*/) {
            actionKeys << MenuAction::Open
                       << MenuAction::OpenInNewWindow
                       << MenuAction::OpenInNewTab
                       << MenuAction::OpenAsAdmin
                       << MenuAction::Separator
                       << MenuAction::Copy
                       << MenuAction::Separator
                       << MenuAction::Compress
                       << MenuAction::Separator;
            /*todo:待DFMLocalFileInfo提供对应接口
            if (canShare() && !isShared()) {
                actionKeys << MenuAction::Share
                           << MenuAction::Separator;
            } else if (isShared()) {
                actionKeys << MenuAction::UnShare
                           << MenuAction::Separator;
            }
            */
            actionKeys << MenuAction::CreateSymlink
                       << MenuAction::SendToDesktop
                       << MenuAction::Separator
                       << MenuAction::OpenInTerminal
                       << MenuAction::Separator;

            actionKeys  << MenuAction::Property;

        } else {
            actionKeys << MenuAction::Open;
            /*todo:待DFMLocalFileInfo提供对应接口
            if (!isVirtualEntry()) {
                actionKeys << MenuAction::OpenWith;
            }
            */

            if (d->m_fileInfo->isDir()) {
                actionKeys << MenuAction::OpenInNewWindow
                           << MenuAction::OpenInNewTab
                           << MenuAction::OpenAsAdmin;
            } else {
                /*todo:待DFMLocalFileInfo提供对应接口
                QSet<QString> mountable = {"application/x-cd-image", "application/x-iso9660-image"};
                if (mountable.contains(mimeTypeName())) {
                    actionKeys << MenuAction::MountImage;
                }*/
            }
            actionKeys << MenuAction::Separator
                       << MenuAction::Cut
                       << MenuAction::Copy
                       << MenuAction::Rename;
            /*todo:待DFMLocalFileInfo提供对应接口
            const_cast<DAbstractFileInfo *>(this)->checkMountFile();
            if (isGvfsMountFile() || deviceListener->isInRemovableDeviceFolder(absoluteFilePath())) {
                if (!isVirtualEntry()) {
                    actionKeys << MenuAction::CompleteDeletion;
                }
            } else {
                actionKeys << MenuAction::Delete;
            }
            */
            actionKeys << MenuAction::Separator;

            if (d->m_fileInfo->isDir()) {
                actionKeys << MenuAction::Compress;

                actionKeys << MenuAction::Separator;
                /*todo:待DFMLocalFileInfo提供对应接口
                if (canShare() && !isShared()) {
                    actionKeys << MenuAction::Share
                               << MenuAction::Separator;
                } else if (isShared()) {
                    actionKeys << MenuAction::UnShare
                               << MenuAction::Separator;
                }*/

            } else if (d->m_fileInfo->isFile()) {
                if (!FileUtils::isArchive(d->m_fileInfo->absoluteFilePath())) {
                    actionKeys << MenuAction::Compress
                               << MenuAction::Separator;
                }
            }

            if (d->m_fileInfo->isFile()) {
                if (FileUtils::isArchive(d->m_fileInfo->absoluteFilePath())) {
                    actionKeys << MenuAction::Decompress
                               << MenuAction::DecompressHere
                               << MenuAction::Separator;
                }
            }

            actionKeys << MenuAction::CreateSymlink
                       << MenuAction::SendToDesktop;
            /*todo:待DFMLocalFileInfo提供对应接口
            if (deviceListener->getCanSendDisksByUrl(d->m_fileInfo->absoluteFilePath()).count() > 0
                    || bluetoothManager->model()->adapters().count() > 0) {
                actionKeys << MenuAction::SendToRemovableDisk;
            }

            DDiskManager diskm;
            for (auto &devs : diskm.diskDevices()) {
                QScopedPointer<DDiskDevice> dev(DDiskManager::createDiskDevice(devs));
                if (dev->mediaCompatibility().join(' ').contains("_r")) {
                    actionKeys << MenuAction::StageFileForBurning;
                    break;
                }
            }
            */
            if (d->m_fileInfo->isDir()) {
                // FIXME: reimplement BookMark::exist() 's behavior and use it for check bookmark existance.
                //        after doing this, don't forget to remove the "bookmarkmanager.h" header file include.
                // if (DFileService::instance()->createFileInfo(nullptr, DUrl::fromBookMarkFile(fileUrl(), QString()))) {

                /*todo:待提供对应接口
                if (!d->m_fileInfo->isSymLink()) {
                    if (Singleton<BookMarkManager>::instance()->checkExist(DUrl::fromBookMarkFile(fileUrl(), QString()))) {
                        actionKeys << MenuAction::BookmarkRemove;
                    } else {
                        actionKeys << MenuAction::AddToBookMark;
                    }
                }
                */
                actionKeys << MenuAction::Separator
                           << MenuAction::OpenInTerminal
                           << MenuAction::Separator;
            } else if (d->m_fileInfo->isFile()) {
                /*todo:待提供对应接口
                if (mimeTypeName().startsWith("image") && d->m_fileInfo->isReadable()
//                        && !mimeTypeName().endsWith("gif")
                        && !mimeTypeName().endsWith("svg+xml")
                        && !mimeTypeName().endsWith("raf")
                        && !mimeTypeName().endsWith("crw")) {
                    actionKeys << MenuAction::SetAsWallpaper
                               << MenuAction::Separator;
                }
                */
            }

#ifdef SW_LABEL
            qDebug() << LabelMenuItemIds;
            foreach (QString id, LabelMenuItemIds) {
                int index = LabelMenuItemIds.indexOf(id);
                MenuAction actionType = MenuAction(MenuAction::Unknow + index + 1);
                actionKeys << actionType;
                fileMenuManger->setActionString(actionType, LabelMenuItemDatas[id].label);
                fileMenuManger->setActionID(actionType, id);
            }
            actionKeys << MenuAction::Separator;
#endif
            int size { actionKeys.size() };

            if (actionKeys[size - 1] != MenuAction::Separator) {
                actionKeys << MenuAction::Separator;
            }

            ///###: tag protocol
            actionKeys << MenuAction::TagInfo;
            actionKeys << MenuAction::TagFilesUseColor;

            actionKeys  << MenuAction::Property;
        }

    } else if (type == MultiFiles) {
        actionKeys << MenuAction::Open
                   << MenuAction::OpenWith
                   << MenuAction::Separator
                   << MenuAction::Cut
                   << MenuAction::Copy
                   << MenuAction::Rename
                   << MenuAction::Compress
                   << MenuAction::SendToDesktop;

        /*todo:待DFMLocalFileInfo提供对应接口
        if (deviceListener->getCanSendDisksByUrl(d->m_fileInfo->absoluteFilePath()).count() > 0
                || bluetoothManager->model()->adapters().count() > 0) {
            actionKeys << MenuAction::SendToRemovableDisk;
        }

        DDiskManager diskm;
        for (auto &devs : diskm.diskDevices()) {
            QScopedPointer<DDiskDevice> dev(DDiskManager::createDiskDevice(devs));
            if (dev->mediaCompatibility().join(' ').contains("_r")) {
                actionKeys << MenuAction::StageFileForBurning;
                break;
            }
        }
        const_cast<DAbstractFileInfo *>(this)->checkMountFile();
        if (isGvfsMountFile() || deviceListener->isInRemovableDeviceFolder(d->m_fileInfo->absoluteFilePath())) {
            if (!isVirtualEntry()) {
                actionKeys << MenuAction::CompleteDeletion;
            }
        } else {
            actionKeys << MenuAction::Delete;
        }
        */
        actionKeys << MenuAction::Separator
                   << MenuAction::Property;

        ///###: tag protocol.
        actionKeys << MenuAction::TagInfo;
        actionKeys << MenuAction::TagFilesUseColor;

        actionKeys  << MenuAction::Property;

    } else if (type == MultiFilesSystemPathIncluded) {
        actionKeys << MenuAction::Open
                   << MenuAction::Separator
                   << MenuAction::Copy
                   << MenuAction::Compress
                   << MenuAction::SendToDesktop
                   << MenuAction::Separator;

        ///###: tag protocol.
        actionKeys << MenuAction::TagInfo;
        actionKeys << MenuAction::TagFilesUseColor;

        actionKeys << MenuAction::Property;
    }

    /*todo:待DFMLocalFileInfo提供对应接口
    if (isGvfsMountFile())
        actionKeys.removeAll(MenuAction::OpenAsAdmin);
    */

    return actionKeys;
}

QMap<DFMLocalMenu::MenuAction, QVector<DFMLocalMenu::MenuAction> > DFMLocalMenu::subMenuActionList(DFMLocalMenu::MenuType type) const
{
    Q_UNUSED(type)
    /*
    QMap<MenuAction, QVector<MenuAction> > actions;

    QVector<MenuAction> openwithMenuActionKeys;
    actions.insert(MenuAction::OpenWith, openwithMenuActionKeys);


    QVector<MenuAction> docmentMenuActionKeys;
#ifdef DFM_MINIMUM
    docmentMenuActionKeys << MenuAction::NewText;
#else
    docmentMenuActionKeys << MenuAction::NewWord
                          << MenuAction::NewExcel
                          << MenuAction::NewPowerpoint
                          << MenuAction::NewText;
#endif

    static const QList<QAction *> template_file_list = getTemplateFileList();
    static const QVector<MenuAction> action_type_list = getMenuActionTypeListByAction(template_file_list);

    QString urlStr = toLocalFile();
    if (urlStr.isEmpty()) {
#ifdef QT_DEBUG
        qCritical() << fileUrl() << "scheme fileinfo seems doesn't implement toLocalFile() or implemented incorrectly.";
        qCritical() << "This can be a bug and should be fixed!!!!!!!!";
#endif
        urlStr = fileUrl().toLocalFile();
    }

    for (QAction *action : template_file_list) {
        action->setProperty("_fileinfo_path", urlStr);
    }

    docmentMenuActionKeys << action_type_list;
    actions.insert(MenuAction::NewDocument, docmentMenuActionKeys);

    QVector<MenuAction> displayAsMenuActionKeys;

    int support_view_mode = supportViewMode();

    if ((support_view_mode & DListView::IconMode) == DListView::IconMode) {
        displayAsMenuActionKeys << MenuAction::IconView;
    }

    if ((support_view_mode & DListView::ListMode) == DListView::ListMode) {
        displayAsMenuActionKeys << MenuAction::ListView;
    }

    actions.insert(MenuAction::DisplayAs, displayAsMenuActionKeys);

    QVector<MenuAction> sortByMenuActionKeys;
//    sortByMenuActionKeys << MenuAction::Name;

    for (int role : sortSubMenuActionUserColumnRoles()) {
        sortByMenuActionKeys << menuActionByColumnRole(role);
    }

    actions.insert(MenuAction::SortBy, sortByMenuActionKeys);

    if (deviceListener->isMountedRemovableDiskExits()
            || bluetoothManager->model()->adapters().count() > 0) {
        QVector<MenuAction> diskMenuActionKeys;
        actions.insert(MenuAction::SendToRemovableDisk, diskMenuActionKeys);
    }

    DDiskManager diskm;
    for (auto &drvs : diskm.diskDevices()) {
        QScopedPointer<DDiskDevice> drv(DDiskManager::createDiskDevice(drvs));
        if (drv->mediaCompatibility().join(' ').contains("_r")) {
            actions.insert(MenuAction::StageFileForBurning, {});
            break;
        }
    }

    return actions;
    */

    //临时最小文管菜单
    QMap<MenuAction, QVector<MenuAction> > actions;

    QVector<MenuAction> openwithMenuActionKeys;
    actions.insert(MenuAction::OpenWith, openwithMenuActionKeys);


    QVector<MenuAction> docmentMenuActionKeys;
#ifdef DFM_MINIMUM
    docmentMenuActionKeys << MenuAction::NewText;
#else
    docmentMenuActionKeys << MenuAction::NewWord
                          << MenuAction::NewExcel
                          << MenuAction::NewPowerpoint
                          << MenuAction::NewText;
#endif
    actions.insert(MenuAction::NewDocument, docmentMenuActionKeys);
    return actions;
}

QSet<DFMLocalMenu::MenuAction> DFMLocalMenu::disableMenuActionList() const
{
    Q_D(const DFMLocalMenu);

    QSet<MenuAction> list;

    if (!d->m_fileInfo->isWritable()) {
        list << MenuAction::NewFolder
             << MenuAction::NewDocument
             << MenuAction::Paste;
    }
    /*todo:待接口完善
    if (!canRename()) {
        list << MenuAction::Cut << MenuAction::Rename << MenuAction::Delete << MenuAction::CompleteDeletion;
    }

    if (isVirtualEntry()) {
        list << MenuAction::Copy;
    }
   */
    return list;
}

DFMLocalMenu::MenuAction DFMLocalMenu::menuActionByColumnRole(int role) const
{

    /*todo:待接口完善
    switch (role) {
    case DFileSystemModel::FileDisplayNameRole:
    case DFileSystemModel::FileNameRole:
        return MenuAction::Name;
    case DFileSystemModel::FileSizeRole:
        return MenuAction::Size;
    case DFileSystemModel::FileMimeTypeRole:
        return MenuAction::Type;
    case DFileSystemModel::FileCreatedRole:
        return MenuAction::CreatedDate;
    case DFileSystemModel::FileLastModifiedRole:
        return MenuAction::LastModifiedDate;
    case DFileSystemModel::FileLastReadRole:
        return MenuAction::LastRead;
    default:
        return MenuAction::Unknow;
    }
    */
    return MenuAction::Unknow;
}

QList<int> DFMLocalMenu::sortSubMenuActionUserColumnRoles() const
{
    /*todo:先用临时的数据，待接口更新
    Q_D(const DFMLocalMenu);

    if (!d->columnCompact) {
        return userColumnRoles();
    }

    QList<int> roles;

    int column = 0;

    for (int role : userColumnRoles()) {
        const QList<int> child_roles = userColumnChildRoles(column++);

        if (child_roles.isEmpty()) {
            roles << role;
        } else {
            roles << child_roles;
        }
    }

    return roles;
    */
    static QList<int> userColumnRoles = QList<int>() << (Qt::UserRole + 9)/*DFileSystemModel::FileDisplayNameRole*/
                                        << (Qt::UserRole + 6)/*DFileSystemModel::FileLastModifiedRole*/
                                        << (Qt::UserRole + 3)/*DFileSystemModel::FileSizeRole*/
                                        << (Qt::UserRole + 4)/*DFileSystemModel::FileMimeTypeRole*/;

    return userColumnRoles;
}

bool DFMLocalMenu::isAddOemExternalAction()
{
    Q_D(DFMLocalMenu);
    return d->m_isAddOemExternalAction;
}

void DFMLocalMenu::setAddOemExternalAction(bool isAdd)
{
    Q_D(DFMLocalMenu);
    d->m_isAddOemExternalAction = isAdd;
}

void DFMLocalMenu::setIsNeedLoadCustomActions(bool needCustom)
{
    Q_D(DFMLocalMenu);
    d->m_isNeedLoadCustomActions = needCustom;
}

bool DFMLocalMenu::isNeedLoadCustomActions()
{
    Q_D(DFMLocalMenu);
    return d->m_isNeedLoadCustomActions;
}



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
#ifndef LOCALMENU_H
#define LOCALMENU_H

#include "dfm-base/dfm_base_global.h"

#include <QObject>
#include <QMenu>

class LocalMenuPrivate;
class LocalMenu : public QObject
{
    Q_OBJECT


    enum MenuAction {
        Unknow,
        Open,
        OpenDisk,
        OpenInNewWindow,
        OpenInNewTab,
        OpenDiskInNewWindow,
        OpenDiskInNewTab,
        OpenAsAdmin,
        OpenWith,
        OpenWithCustom,
        OpenFileLocation,
        Compress,
        Decompress,
        DecompressHere,
        Cut,
        Copy,
        Paste,
        Rename,
        BookmarkRename,
        BookmarkRemove,
        CreateSymlink,
        SendToDesktop,
        SendToRemovableDisk,
        SendToBluetooth,
        AddToBookMark,
        Delete,
        Property,
        NewFolder,
        NewWindow,
        SelectAll,
        Separator,
        ClearRecent,
        ClearTrash,
        DisplayAs, /// sub menu
        SortBy, /// sub menu
        NewDocument, /// sub menu
        NewWord, /// sub menu
        NewExcel, /// sub menu
        NewPowerpoint, /// sub menu
        NewText, /// sub menu
        OpenInTerminal,
        Restore,
        RestoreAll,
        CompleteDeletion,
        Mount,
        Unmount,
        Eject,
        SafelyRemoveDrive,
        Name,
        Size,
        Type,
        CreatedDate,
        LastModifiedDate,
        LastRead,
        DeletionDate,
        SourcePath,
        AbsolutePath,
        Settings,
        Exit,
        IconView,
        ListView,
        ExtendView,
        SetAsWallpaper,
        ForgetPassword,
        Share,
        UnShare,
        SetUserSharePassword,
        FormatDevice,
        OpticalBlank,
        Vault,
        ConnectToServer,

        ///###: tag protocol.
        TagInfo,
        TagFilesUseColor,
        ChangeTagColor,
        DeleteTags,
        RenameTag,

    #ifdef SW_LABEL
        SetLabel,
        ViewLabel,
        EditLabel,
        PrivateFileToPublic,
    #endif

        // recent
        RemoveFromRecent,

        MountImage,
        StageFileForBurning,

        // Vault
        LockNow,
        AutoLock,
        Never,
        FiveMinutes,
        TenMinutes,
        TwentyMinutes,
        DeleteVault,

        UnLock,
        UnLockByKey,

        UserMenuAction = Unknow + 2000
    };

    enum MenuType {
        SingleFile,
        MultiFiles,
        MultiFilesSystemPathIncluded,
        SpaceArea
    };

    Q_DECLARE_PRIVATE(LocalMenu)
    QSharedPointer<LocalMenuPrivate> d_ptr;
public:
    explicit LocalMenu(const QString &filePath);

    virtual QVector<MenuAction> menuActionList(MenuType type) const;
    virtual QMap<MenuAction, QVector<MenuAction>>subMenuActionList(MenuType type = SingleFile) const;
    virtual QSet<MenuAction> disableMenuActionList() const;
    virtual MenuAction menuActionByColumnRole(int role) const;
    virtual QList<int> sortSubMenuActionUserColumnRoles() const;

    bool isAddOemExternalAction();
    void setAddOemExternalAction(bool isAdd);
    void setIsNeedLoadCustomActions(bool needCustom);
    bool isNeedLoadCustomActions();

};

#endif // LOCALMENU_H

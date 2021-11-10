/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     lanxuesong<lanxuesong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#ifndef DFMROOTFILEINFO_H
#define DFMROOTFILEINFO_H

#include "dfileinfo.h"

#define DISKALIAS_GROUP "LocalDiskAlias"
#define DISKALIAS_ITEMS "Items"
#define DISKALIAS_ITEM_UUID "uuid"
#define DISKALIAS_ITEM_NAME "name"
#define DISKALIAS_ITEM_ALIAS "alias"


class DFMRootFileInfoPrivate;

typedef struct {
    QString driver; //盘符
    QString label;  //标签
    QString uuid;   //id
} DiskInfoStr;

class DFMRootFileInfo : public DAbstractFileInfo
{
public:

    enum ItemType {
        UserDirectory = DAbstractFileInfo::FileType::CustomType + 1,
        GvfsGeneric,
        GvfsSMB,
        GvfsMTP,
        GvfsGPhoto2,
        GvfsFTP,
        GvfsCamera,
        UDisksRoot,
        UDisksData,
        UDisksFixed,
        UDisksRemovable,
        UDisksOptical,
        NotConnRemote,
        AppEntry,
    };

    explicit DFMRootFileInfo(const DUrl &url);
    bool exists() const override;

    QString suffix() const override;
    QString fileDisplayName() const override;

    bool canRename() const override;
    bool canShare() const override;
    bool canFetch() const override;
    bool isReadable() const override;
    bool isWritable() const override;
    bool isExecutable() const override;
    bool isHidden() const override;
    bool isRelative() const override;
    bool isAbsolute() const override;
    bool isShared() const override;
    bool isTaged() const override;
    bool isWritableShared() const override;
    bool isAllowGuestShared() const override;

    bool isFile() const override;
    bool isDir() const override;
    FileType fileType() const override;
    int filesCount() const override;
    QString iconName() const override;

    QVector<MenuAction> menuActionList(MenuType type = SingleFile) const override;

    bool canRedirectionFileUrl() const override;
    DUrl redirectedFileUrl() const override;
    bool canDrop() const override;
    Qt::DropActions supportedDragActions() const override;
    Qt::DropActions supportedDropActions() const override;

    QVariantHash extraProperties() const override;

    void refresh(const bool isForce = false) override;

    bool canSetAlias() const;
    void checkCache();
    QString udisksDispalyAlias();
    QString udisksDisplayName();
    QString getVolTag(); // ....../dev/sr0 -> sr0
    QString getUUID();

    bool checkMpsStr(const QString &path) const override;

    static bool typeCompare(const DAbstractFileInfoPointer &a, const DAbstractFileInfoPointer &b);
    //fix bug 49348 if unable network,ftp smb sort the last
    static bool typeCompareByUrl(const DAbstractFileInfoPointer &a, const DAbstractFileInfoPointer &b);

    static QMap<QString, DiskInfoStr> DiskInfoMap;
private:
    QScopedPointer<DFMRootFileInfoPrivate> d_ptr;
    Q_DECLARE_PRIVATE(DFMRootFileInfo)

    /**
     * @brief loadDiskInfo 加载盘符信息
     */
    void loadDiskInfo();
};

#endif // DFMROOTFILEINFO_H

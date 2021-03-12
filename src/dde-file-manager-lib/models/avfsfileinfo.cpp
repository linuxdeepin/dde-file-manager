/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     dengkeyun<dengkeyun@uniontech.com>
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

#include "avfsfileinfo.h"

#include "dfileinfo.h"
#include "private/dabstractfileinfo_p.h"
#include "dfilesystemmodel.h"
#include "controllers/avfsfilecontroller.h"
#include "shutil/fileutils.h"

#include <QFileInfo>
#include <QStandardPaths>
#include <QIcon>

class AVFSFileInfoPrivate : public DAbstractFileInfoPrivate
{
public:
    AVFSFileInfoPrivate(const DUrl &url, AVFSFileInfo *qq)
        : DAbstractFileInfoPrivate(url, qq, true)
    {
    }
};

AVFSFileInfo::AVFSFileInfo(const DUrl &avfsUrl):
    DAbstractFileInfo(*new AVFSFileInfoPrivate(avfsUrl, this))
{

    setProxy(DAbstractFileInfoPointer(new DFileInfo(realFileUrl(avfsUrl))));
}

bool AVFSFileInfo::canRename() const
{
    return false;
}

bool AVFSFileInfo::isWritable() const
{
    return false;
}

bool AVFSFileInfo::canShare() const
{
    return false;
}

QVector<MenuAction> AVFSFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{

    QVector<MenuAction> actions;
    if (type == DAbstractFileInfo::SingleFile) {
        actions << MenuAction::Open;
        if (!isDir())
            actions << MenuAction::OpenWith;
        actions << MenuAction::Separator;
        actions << MenuAction::Copy
                << MenuAction::Property;

    } else if (type == DAbstractFileInfo::SpaceArea) {
        actions << MenuAction::SortBy
                << MenuAction::DisplayAs
                << MenuAction::Property;
    } else {
        actions << MenuAction::Open
                << MenuAction::Separator
                << MenuAction::Copy
                << MenuAction::Property;
    }
    return actions;
}

bool AVFSFileInfo::canIteratorDir() const
{
    return true;
}

bool AVFSFileInfo::isDir() const
{
    Q_D(const AVFSFileInfo);
    //Temporarily just support one lay arch file parser
    QString realFilePath = realFileUrl(fileUrl()).toLocalFile();
    if (FileUtils::isArchive(realFilePath)) {
        realFilePath += "#/";
        return QFileInfo(realFilePath).isDir();
    }
    return d->proxy->isDir();
}

bool AVFSFileInfo::canManageAuth() const
{
    //压缩包内的文件不提供权限管理
    return false;
}

QString AVFSFileInfo::toLocalFile() const
{
    return fileUrl().path();
}

DUrl AVFSFileInfo::parentUrl() const
{
    DUrl durl = DAbstractFileInfo::parentUrl();
    if (fileType() == FileType::Directory) {
        durl.setScheme(FILE_SCHEME);
    }
    return durl;
}

AVFSFileInfo::AVFSFileInfo(AVFSFileInfoPrivate &dd):
    DAbstractFileInfo(dd)
{
}

DUrl AVFSFileInfo::realFileUrl(const DUrl &avfsUrl)
{
    QString avfsPath = avfsUrl.path();

    if (avfsPath.endsWith("/"))
        avfsPath.chop(1);

    QString mountPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    mountPath = mountPath + "/.avfs";

    QString virtualPath = mountPath + avfsPath;
    QStringList pathItems = virtualPath.split("/");
    QString iterPath = "/";
    int archLength = 0;
    foreach (QString item, pathItems) {
        if (item == "")
            continue;
        iterPath += item;
        if (FileUtils::isArchive(iterPath)) {
            archLength++;
            iterPath += "#/";
        } else
            iterPath += "/";
    }

    if (archLength > 1) {
        if (iterPath.endsWith("#/"))
            iterPath.chop(2);
        else
            iterPath.chop(1);
    } else
        iterPath.chop(1);

    return DUrl::fromLocalFile(iterPath);
}

DUrl AVFSFileInfo::realDirUrl(const DUrl &avfsUrl)
{
    QString avfsPath = avfsUrl.path();

    if (avfsPath.endsWith("/"))
        avfsPath.chop(1);

    QString mountPath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    mountPath = mountPath + "/.avfs";

    QString virtualPath = mountPath + avfsPath;
    QStringList pathItems = virtualPath.split("/");
    QString iterPath = "/";
    foreach (QString item, pathItems) {
        if (item == "")
            continue;
        iterPath += item;
        if (FileUtils::isArchive(iterPath))
            iterPath += "#/";
        else
            iterPath += "/";
    }

    iterPath.chop(1);

    return DUrl::fromLocalFile(iterPath);
}

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

#include "tagfileinfo.h"
#include "dfileservices.h"
#include "dfilesystemmodel.h"
#include "dabstractfileinfo.h"
#include "durl.h"
#include "private/dfileinfo_p.h"
#include "tag/tagmanager.h"
#include "shutil/fileutils.h"

TagFileInfo::TagFileInfo(const DUrl &url)
    : DAbstractFileInfo{ url, false } //###: Do not cache.
{
    ///###: if the true url of file is put into fragment field of Uri. Then setup proxy.
    if (!url.taggedLocalFilePath().isEmpty()) {
        DAbstractFileInfoPointer infoPointer{ DFileService::instance()->createFileInfo(nullptr, DUrl::fromLocalFile(url.fragment(QUrl::FullyDecoded))) };
        this->DAbstractFileInfo::setProxy(infoPointer);
    }
}

bool TagFileInfo::isDir() const
{
    const DAbstractFileInfoPrivate *const d{ d_func() };

    return ((!static_cast<bool>(d->proxy)) || d->proxy->isDir());
}

bool TagFileInfo::makeAbsolute()
{
    return true;
}

bool TagFileInfo::exists() const
{
    const DAbstractFileInfoPrivate *const d{ d_func() };

    if (d->proxy) {
        return d->proxy->exists();
    }

    if (fileUrl() == DUrl(TAG_ROOT)) {
        return true;
    }

    const QMap<QString, QString> &tag_map = TagManager::instance()->getAllTags();

    return tag_map.contains(fileUrl().tagName());
}

bool TagFileInfo::canRename() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy) {
        return d->proxy->canRename();
    }

    return fileUrl() != DUrl(TAG_ROOT);
}

bool TagFileInfo::isTaged() const
{
    return true;
}

bool TagFileInfo::isWritable() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->isWritable();

    return true;
}

bool TagFileInfo::canRedirectionFileUrl() const
{
    const DAbstractFileInfoPrivate *const d{ d_func() };

    return static_cast<bool>(d->proxy);
}

int TagFileInfo::filesCount() const
{
    //Tag数据查询会返回很多无效的Path,需要在计算数量时判断Path是否存在
    //TODO: 这种做法不是最优解，在文管重构时需要重写Tag数据库相关的读写逻辑，保证返回有效Path
    QList<QString> files(TagManager::instance()->getFilesThroughTag(fileUrl().tagName()));

    int fileCount = 0;
    for(const QString& localFilePath : files){
        DUrl url{ fileUrl() };
        url.setTaggedFileUrl(localFilePath);
        DAbstractFileInfoPointer fileInfo{ new TagFileInfo(url) };

        if (fileInfo->exists())
            ++fileCount;
    }

    return fileCount;
}

QFileDevice::Permissions TagFileInfo::permissions() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->permissions();

    return QFile::ReadGroup | QFile::ReadOwner | QFile::ReadUser | QFile::ReadOther
           | QFile::WriteGroup | QFile::WriteOwner | QFile::WriteUser | QFile::WriteOther;
}


DUrl TagFileInfo::redirectedFileUrl() const
{
    const DAbstractFileInfoPrivate *const d{ d_func() };

    //d-proxy可能为空，需要先判断
    if (d->proxy)
        return d->proxy->fileUrl();

    return fileUrl();
}

Qt::ItemFlags TagFileInfo::fileItemDisableFlags() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->fileItemDisableFlags();

    return fileUrl() != DUrl(TAG_ROOT) ? Qt::ItemIsDragEnabled : Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVector<MenuAction> TagFileInfo::menuActionList(DAbstractFileInfo::MenuType type) const
{
    Q_D(const DAbstractFileInfo);

    QVector<MenuAction> actions;

    if (!d->proxy) {
        if (type == SpaceArea) {
            actions << MenuAction::DisplayAs;
            actions << MenuAction::SortBy;
        } else {
            actions << MenuAction::Open;
            actions << MenuAction::OpenInNewWindow;
            actions << MenuAction::OpenInNewTab;
            actions << MenuAction::Rename;
            actions << MenuAction::ChangeTagColor;
        }

        return actions;
    }

    actions = d->proxy->menuActionList(type);
    actions.insert(1, MenuAction::OpenFileLocation);

    return actions;
}

DUrl TagFileInfo::getUrlByNewFileName(const QString &name) const
{
    DUrl new_url = fileUrl();
    const QString &local_file = new_url.taggedLocalFilePath();

    if (local_file.isEmpty())
        return DAbstractFileInfo::getUrlByNewFileName(name);

    QFileInfo file_info(local_file);

    new_url.setTaggedFileUrl(file_info.absoluteDir().absoluteFilePath(name));

    return new_url;
}

bool TagFileInfo::canIteratorDir() const
{
    Q_D(const DAbstractFileInfo);

    if (!d->proxy)
        return true;

    return d->proxy->canIteratorDir();
}

QVariantHash TagFileInfo::extraProperties() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->extraProperties();

    QVariantHash hash;

    if (fileUrl() == DUrl(TAG_ROOT))
        return hash;

    const QString &tag_name = fileUrl().tagName();
    const QColor &tag_color = TagManager::instance()->getTagColor({tag_name}).value(tag_name);

    hash["tag_name_list"] = QStringList{tag_name};
    hash["colored"] = QVariant::fromValue(QList<QColor> {tag_color});

    return hash;
}

QList<int> TagFileInfo::userColumnRoles() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->userColumnRoles();

    if (fileUrl() == DUrl(TAG_ROOT)) {
        return QList<int> {
            DFileSystemModel::FileDisplayNameRole,
            DFileSystemModel::FileSizeRole
        };
    }

    return DAbstractFileInfo::userColumnRoles();
}

DUrl TagFileInfo::mimeDataUrl() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->mimeDataUrl();

    return DUrl();
}

Qt::DropActions TagFileInfo::supportedDragActions() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->supportedDragActions();

    return DAbstractFileInfo::supportedDragActions();
}

Qt::DropActions TagFileInfo::supportedDropActions() const
{
    Q_D(const DAbstractFileInfo);

    if (d->proxy)
        return d->proxy->supportedDropActions();

    if (fileUrl().tagName().isEmpty())
        return Qt::IgnoreAction;

    return Qt::CopyAction;
}

bool TagFileInfo::canDrop() const
{
    return DAbstractFileInfo::canDrop();
}

bool TagFileInfo::isVirtualEntry() const
{
    Q_D(const DAbstractFileInfo);
    return !d->proxy;
}

QString TagFileInfo::sizeDisplayName() const
{
    // 如果是文件夹，不去遍历下面的文件数量，否则大文件数量时界面会很卡
    // 原因是因为model会一直刷新，获取size
    if (isDir()) {
        return QStringLiteral("-");
    }

    return FileUtils::formatSize(size());
}

bool TagFileInfo::canDragCompress() const
{
    // 标记文件不支持追加压缩
    return false;
}

DUrl TagFileInfo::parentUrl() const
{
    DUrl url = fileUrl();

    if (url.taggedLocalFilePath().isEmpty()) {
        return DAbstractFileInfo::parentUrl();
    }

    return DUrl::fromUserTaggedFile(url.tagName(), QString{});
}

QString TagFileInfo::iconName() const
{
    DUrl current_url{ this->fileUrl() };

    if (current_url.isTaggedFile()) {
        return QString{"folder"};
    }

    return DAbstractFileInfo::iconName();
}

DUrl TagFileInfo::goToUrlWhenDeleted() const
{
    const DAbstractFileInfoPrivate *d{ d_func() };

    DUrl current_url{ this->fileUrl() };
    QString parent_url{ current_url.parentUrl().path()};

    ///###: if there current file-info do not have a proxy!
    ///###: it shows that current item is a tag-dir(tag:///tag-name).
    if (current_url.isTaggedFile() && parent_url == QString{"/"} && !d->proxy) {
        return DUrl::fromLocalFile(QDir::homePath());
    }

    return DAbstractFileInfo::goToUrlWhenDeleted();
}



//bool TagFileInfo::columnDefaultVisibleForRole(int role) const
//{
//    if(role == DFileSystemModel::FileLastModifiedRole){
//        return false;
//    }

//    return DAbstractFileInfo::columnDefaultVisibleForRole(role);
//}





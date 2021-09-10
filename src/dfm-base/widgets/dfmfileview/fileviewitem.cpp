/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyu@uniontech.com>
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

#include "fileviewitem.h"
#include "shutil/fileutils.h"
#include "private/fileviewitem_p.h"

DFMBASE_USE_NAMESPACE
FileViewItem::FileViewItem()
    : d(new FileViewItemPrivate(this))
{

}

FileViewItem::FileViewItem(const QUrl &url)
    : d(new FileViewItemPrivate(this))
{
    setUrl(url);
}

FileViewItem &FileViewItem::operator=(const FileViewItem &other)
{
    setData(other.data(ItemNameRole),ItemNameRole);
    setData(other.data(ItemIconRole),ItemIconRole);
    setData(other.data(ItemEditRole),ItemEditRole);
    setData(other.data(ItemToolTipRole),ItemToolTipRole);
    setData(other.data(ItemSizeHintRole),ItemSizeHintRole);
    setData(other.data(ItemBackgroundRole),ItemBackgroundRole);
    setData(other.data(ItemForegroundRole),ItemForegroundRole);
    setData(other.data(ItemCheckStateRole),ItemCheckStateRole);
    setData(other.data(ItemInitialSortOrderRole),ItemInitialSortOrderRole);
    setData(other.data(ItemFontRole),ItemFontRole);
    setData(other.data(ItemTextAlignmentRole),ItemTextAlignmentRole);
    setData(other.data(ItemColorRole),ItemColorRole);
    setData(other.data(ItemUrlRole),ItemUrlRole);
    setData(other.data(ItemFileLastModifiedRole),ItemFileLastModifiedRole);
    setData(other.data(ItemFileSizeRole),ItemFileSizeRole);
    setData(other.data(ItemFileMimeTypeRole),ItemFileMimeTypeRole);
    setData(other.data(ItemFileSuffixRole),ItemFileSuffixRole);
    setData(other.data(ItemFilePathRole),ItemFilePathRole);
    setData(other.data(ItemColumListRole),ItemColumListRole);
    setData(other.data(ItemColumWidthScaleListRole),ItemColumWidthScaleListRole);

    return *this;
}

void FileViewItem::refresh(){
    if(!d->fileinfo.isNull())
        d->fileinfo->refresh();
    d->mimeType = MimeDatabase::mimeTypeForUrl(d->fileinfo->url());
}

QUrl FileViewItem::url() const
{
    return QStandardItem::data(Roles::ItemUrlRole).toUrl();
}

void FileViewItem::setUrl(const QUrl url)
{
    setData(QVariant(url),Roles::ItemUrlRole);
    d->fileinfo = InfoFactory::create<AbstractFileInfo>(url);
    d->mimeType = MimeDatabase::mimeTypeForUrl(url);

    setData(QVariant(QIcon::fromTheme(d->mimeType.iconName())),ItemIconRole);
    setData(QVariant(d->fileinfo->fileName()),ItemNameRole);
}

AbstractFileInfoPointer FileViewItem::fileinfo() const
{
    return d->fileinfo;
}

QMimeType FileViewItem::mimeType() const
{
    return d->mimeType;
}


QVariant FileViewItem::data(int role) const
{
    if (d->fileinfo.isNull())
        return QVariant();

    switch (role) {
    case ItemFileLastModifiedRole:
        return d->fileinfo->lastModified().toString("yyyy/MM/dd HH:mm:ss");
    case ItemIconRole:
        return QStandardItem::data(Roles::ItemIconRole);
    case ItemFileSizeRole:
        if (d->fileinfo->isDir()) {
            int size = qSharedPointerDynamicCast<LocalFileInfo>(d->fileinfo)->countChildFile();
            if (size <= 1) {
                return QObject::tr("%1 item").arg(size);
            } else {
                return QObject::tr("%1 items").arg(size);
            }
        }
        else {
            QSharedPointer<LocalFileInfo> local = qSharedPointerCast<LocalFileInfo>(d->fileinfo);
            if (local)
                return local->sizeFormat();
            return QString::number(d->fileinfo->size());
        }
    case ItemFileMimeTypeRole:
        return mimeType().name();
    case ItemColumListRole:
    {
        QList<int> columrollist;
        columrollist << ItemNameRole << ItemFileLastModifiedRole << ItemFileSizeRole << ItemFileMimeTypeRole;

        return QVariant::fromValue<QList<int>>(columrollist);
    }
    case  ItemFileSuffixRole:
        return d->fileinfo->completeSuffix();
    case ItemColumWidthScaleListRole:
    {
        QList<int> columrolwidthlist;
        columrolwidthlist << 2 << 2 << 1 << 1;
        return QVariant::fromValue<QList<int>>(columrolwidthlist);
    }
    case ItemNameRole:
        return d->fileinfo->fileName();
    case ItemSizeHintRole:
        return QSize(-1, 26);
    default:
        return QVariant();
        //        return QStandardItem::data(role);
    }
}

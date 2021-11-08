/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     huanyu<huanyub@uniontech.com>
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
#include "utils/fileutils.h"
#include "private/fileviewitem_p.h"

DFMBASE_USE_NAMESPACE
FileViewItem::FileViewItem()
    : d(new FileViewItemPrivate(this))
{
    qRegisterMetaType<FileViewItem>("FileViewItem");
}

FileViewItem::FileViewItem(const QUrl &url)
    : d(new FileViewItemPrivate(this))
{
    qRegisterMetaType<FileViewItem>("FileViewItem");
    setUrl(url);
}

FileViewItem::FileViewItem(const FileViewItem &other)
    : d(new FileViewItemPrivate(this))
{
    *this = other;
}

FileViewItem::~FileViewItem()
{
    delete d;
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
    setData(QVariant(url), Roles::ItemUrlRole);

    d->fileinfo = InfoFactory::create<AbstractFileInfo>(url);
    if (!d->fileinfo)
        abort();

    d->mimeType = MimeDatabase::mimeTypeForUrl(url);
    if (!d->mimeType.isValid())
        abort();

    setData(QVariant(QIcon::fromTheme(d->mimeType.iconName())), ItemIconRole);
    setData(QVariant(d->fileinfo->fileName()), ItemNameRole);
}

AbstractFileInfoPointer FileViewItem::fileinfo() const
{
    return d->fileinfo;
}

void FileViewItem::setCornerMark(const QIcon &tl, const QIcon &tr, const QIcon &bl, const QIcon &br)
{
    if (!tl.isNull())
        setData(tl, TopLeft);
    if (!tr.isNull())
        setData(tr, TopRight);
    if (!tl.isNull())
        setData(bl, BottomLeft);
    if (!br.isNull())
        setData(br, BottomRight);
}

void FileViewItem::setCornerMark(FileViewItem::CornerMark flag, const QIcon &icon)
{
    switch (flag) {
    case TopLeft:
        return setData(icon, ItemCornerMarkTLRole);
    case TopRight:
        return setData(icon, ItemCornerMarkTRRole);
    case BottomLeft:
        return setData(icon, ItemCornerMarkBLRole);
    case BottomRight:
        return setData(icon, ItemCornerMarkBRRole);
    }
}

QIcon FileViewItem::cornerMarkTR()
{
    auto variant = QStandardItem::data(ItemCornerMarkTRRole);
    if (variant.canConvert<QIcon>()) {
        return qvariant_cast<QIcon>(variant);
    }
    return QIcon();
}

QIcon FileViewItem::cornerMarkBL()
{
    auto variant = QStandardItem::data(ItemCornerMarkBLRole);
    if (variant.canConvert<QIcon>()) {
        return qvariant_cast<QIcon>(variant);
    }
    return QIcon();
}

QIcon FileViewItem::cornerMarkBR()
{
    auto variant = QStandardItem::data(ItemCornerMarkBRRole);
    if (variant.canConvert<QIcon>()) {
        return qvariant_cast<QIcon>(variant);
    }
    return QIcon();
}

void FileViewItem::setIconLayers(const IconLayers &layers)
{
    QStandardItem::setData(QVariant::fromValue<IconLayers>(layers),
                           ItemIconLayersRole);
}

IconLayers FileViewItem::iconLayers()
{
    QVariant variant = data(ItemIconLayersRole);
    if (!variant.canConvert<IconLayers>())
        return {};
    return qvariant_cast<IconLayers>(variant);
}

QIcon FileViewItem::cornerMarkTL()
{
    auto variant = QStandardItem::data(ItemCornerMarkTLRole);
    if (variant.canConvert<QIcon>()) {
        return qvariant_cast<QIcon>(variant);
    }
    return QIcon();
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
        QList<QPair<int,int>> columrollist;
        columrollist << QPair<int,int>(ItemNameRole, 32) << QPair<int,int>(ItemFileLastModifiedRole, 32)
                << QPair<int,int>(ItemFileSizeRole, 20) << QPair<int,int>(ItemFileMimeTypeRole, 16);

        return QVariant::fromValue<QList<QPair<int,int>>>(columrollist);
    }
    case  ItemFileSuffixRole:
        return d->fileinfo->completeSuffix();
    case ItemNameRole:
        return d->fileinfo->fileName();
    case ItemSizeHintRole:
        return QSize(-1, 26);
    default:
        return QVariant();
    }
}

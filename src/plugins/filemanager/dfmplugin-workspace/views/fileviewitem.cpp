/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
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
#include "events/workspaceeventsequence.h"

#include "dfm-base/dfm_global_defines.h"

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

FileViewItem::FileViewItem()
    : d(new FileViewItemPrivate(this))
{
    qRegisterMetaType<FileViewItem>("FileViewItem");
}

FileViewItem::FileViewItem(FileViewItem *parent)
    : d(new FileViewItemPrivate(this)),
      parent(parent)
{
    qRegisterMetaType<FileViewItem>("FileViewItem");
}

FileViewItem::FileViewItem(FileViewItem *parent, const QUrl &url)
    : d(new FileViewItemPrivate(this)),
      parent(parent)
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
    setData(other.data(kItemNameRole), kItemNameRole);
    setData(other.data(kItemIconRole), kItemIconRole);
    setData(other.data(kItemEditRole), kItemEditRole);
    setData(other.data(kItemToolTipRole), kItemToolTipRole);
    setData(other.data(kItemSizeHintRole), kItemSizeHintRole);
    setData(other.data(kItemBackgroundRole), kItemBackgroundRole);
    setData(other.data(kItemForegroundRole), kItemForegroundRole);
    setData(other.data(kItemCheckStateRole), kItemCheckStateRole);
    setData(other.data(kItemInitialSortOrderRole), kItemInitialSortOrderRole);
    setData(other.data(kItemFontRole), kItemFontRole);
    setData(other.data(kItemTextAlignmentRole), kItemTextAlignmentRole);
    setData(other.data(kItemColorRole), kItemColorRole);
    setData(other.data(kItemUrlRole), kItemUrlRole);
    setData(other.data(kItemFileLastModifiedRole), kItemFileLastModifiedRole);
    setData(other.data(kItemFileSizeRole), kItemFileSizeRole);
    setData(other.data(kItemFileMimeTypeRole), kItemFileMimeTypeRole);
    setData(other.data(kItemFilePathRole), kItemFilePathRole);
    setData(other.data(kItemColumListRole), kItemColumListRole);
    setData(other.data(kItemColumWidthScaleListRole), kItemColumWidthScaleListRole);
    setData(other.data(kItemCornerMarkTLRole), kItemCornerMarkTLRole);
    setData(other.data(kItemCornerMarkTLRole), kItemCornerMarkTLRole);
    setData(other.data(kItemCornerMarkBLRole), kItemCornerMarkBLRole);
    setData(other.data(kItemCornerMarkBRRole), kItemCornerMarkBRRole);
    setData(other.data(kItemIconLayersRole), kItemIconLayersRole);
    setData(other.data(kItemFilePinyinNameRole), kItemFilePinyinNameRole);
    setData(other.data(kItemFileBaseNameRole), kItemFileBaseNameRole);
    setData(other.data(kItemFileSuffixRole), kItemFileSuffixRole);
    setData(other.data(kItemFileNameOfRenameRole), kItemFileNameOfRenameRole);
    setData(other.data(kItemFileBaseNameOfRenameRole), kItemFileBaseNameOfRenameRole);
    setData(other.data(kItemFileSuffixOfRenameRole), kItemFileSuffixOfRenameRole);
    setData(other.data(kItemExtraProperties), kItemExtraProperties);
    setData(other.data(kItemFileIconModelToolTipRole), kItemFileIconModelToolTipRole);
    return *this;
}

void FileViewItem::refresh()
{
    if (!d->fileinfo.isNull())
        d->fileinfo->refresh();
    d->mimeType = MimeDatabase::mimeTypeForUrl(d->fileinfo->url());
}

QUrl FileViewItem::url() const
{
    return QStandardItem::data(kItemUrlRole).toUrl();
}

void FileViewItem::setUrl(const QUrl url)
{
    setData(QVariant(url), kItemUrlRole);

    d->fileinfo = InfoFactory::create<AbstractFileInfo>(url);
    if (!d->fileinfo)
        abort();

    d->mimeType = MimeDatabase::mimeTypeForUrl(url);
    if (!d->mimeType.isValid())
        abort();
    // Todo(yanghao)
    //setData(QVariant(d->fileinfo->fileIcon()), kItemIconRole);
    setData(QVariant(d->fileinfo->fileName()), kItemNameRole);
}

AbstractFileInfoPointer FileViewItem::fileInfo() const
{
    return d->fileinfo;
}

void FileViewItem::setCornerMark(const QIcon &tl, const QIcon &tr, const QIcon &bl, const QIcon &br)
{
    if (!tl.isNull())
        setData(tl, kTopLeft);
    if (!tr.isNull())
        setData(tr, kTopRight);
    if (!tl.isNull())
        setData(bl, kBottomLeft);
    if (!br.isNull())
        setData(br, kBottomRight);
}

void FileViewItem::setCornerMark(FileViewItem::CornerMark flag, const QIcon &icon)
{
    switch (flag) {
    case kTopLeft:
        return setData(icon, kItemCornerMarkTLRole);
    case kTopRight:
        return setData(icon, kItemCornerMarkTRRole);
    case kBottomLeft:
        return setData(icon, kItemCornerMarkBLRole);
    case kBottomRight:
        return setData(icon, kItemCornerMarkBRRole);
    }
}

QIcon FileViewItem::cornerMarkTR()
{
    auto variant = QStandardItem::data(kItemCornerMarkTRRole);
    if (variant.canConvert<QIcon>()) {
        return qvariant_cast<QIcon>(variant);
    }
    return QIcon();
}

QIcon FileViewItem::cornerMarkBL()
{
    auto variant = QStandardItem::data(kItemCornerMarkBLRole);
    if (variant.canConvert<QIcon>()) {
        return qvariant_cast<QIcon>(variant);
    }
    return QIcon();
}

QIcon FileViewItem::cornerMarkBR()
{
    auto variant = QStandardItem::data(kItemCornerMarkBRRole);
    if (variant.canConvert<QIcon>()) {
        return qvariant_cast<QIcon>(variant);
    }
    return QIcon();
}

void FileViewItem::setIconLayers(const IconLayers &layers)
{
    QStandardItem::setData(QVariant::fromValue<IconLayers>(layers),
                           kItemIconLayersRole);
}

IconLayers FileViewItem::iconLayers()
{
    QVariant variant = data(kItemIconLayersRole);
    if (!variant.canConvert<IconLayers>())
        return {};
    return qvariant_cast<IconLayers>(variant);
}

QIcon FileViewItem::cornerMarkTL()
{
    auto variant = QStandardItem::data(kItemCornerMarkTLRole);
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

    QVariant data;
    if (WorkspaceEventSequence::instance()->doFetchCustomRoleData(url(), static_cast<ItemRoles>(role), &data))
        return data;

    switch (role) {
    case kItemFileLastModifiedRole:
        return d->fileinfo->lastModified().toString(dateTimeFormat());
    case kItemIconRole:
        return d->fileinfo->fileIcon();
    case kItemFileSizeRole:
        return d->fileinfo->sizeDisplayName();
    case kItemFileMimeTypeRole:
        return d->fileinfo->mimeTypeDisplayName();
    case kItemColumListRole: {
        QList<QPair<int, int>> columrollist;
        columrollist << QPair<int, int>(kItemFileDisplayNameRole, 32) << QPair<int, int>(kItemFileLastModifiedRole, 32)
                     << QPair<int, int>(kItemFileSizeRole, 20) << QPair<int, int>(kItemFileMimeTypeRole, 16);

        return QVariant::fromValue<QList<QPair<int, int>>>(columrollist);
    }
    case kItemSizeHintRole:
        return QSize(-1, 26);
    case kItemNameRole:
        return d->fileinfo->fileName();
    case kItemFileDisplayNameRole:
        return d->fileinfo->fileDisplayName();
    case kItemFilePinyinNameRole:
        return d->fileinfo->fileDisplayPinyinName();
    case kItemFileBaseNameRole:
        return d->fileinfo->completeBaseName();
    case kItemFileSuffixRole:
        return d->fileinfo->suffix();
    case kItemFileNameOfRenameRole:
        return d->fileinfo->fileNameOfRename();
    case kItemFileBaseNameOfRenameRole:
        return d->fileinfo->baseNameOfRename();
    case kItemFileSuffixOfRenameRole:
        return d->fileinfo->suffixOfRename();
    case kItemUrlRole:
        return d->fileinfo->url();
    default:
        return QVariant();
    }
}

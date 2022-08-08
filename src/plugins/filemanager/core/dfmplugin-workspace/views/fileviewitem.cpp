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

#include "dfm-base/dfm_global_defines.h"

#include <QStandardPaths>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace dfmplugin_workspace;

FileViewItem::FileViewItem()
    : d(new FileViewItemPrivate(this))
{
    qRegisterMetaType<FileViewItem>("FileViewItem");
}

FileViewItem::FileViewItem(FileViewItem *p)
    : d(new FileViewItemPrivate(this)),
      parent(p)
{
    qRegisterMetaType<FileViewItem>("FileViewItem");
}

FileViewItem::FileViewItem(FileViewItem *p, const QUrl &url)
    : d(new FileViewItemPrivate(this)),
      parent(p)
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
    setData(other.data(kItemFileDisplayNameRole), kItemFileDisplayNameRole);
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

    // refresh for GVFS files cost huge time.
    // 20220809 refresh due to fix bug: https://pms.uniontech.com/bug-view-133473.html
    // fix in right menu or short cut naturally
    // todo lanxs
    /*if (!url.path().contains(QRegularExpression(Global::Regex::kGvfsRoot)))
        d->fileinfo->refresh();*/
    setData(QVariant(d->fileinfo->fileName()), kItemFileDisplayNameRole);
}

AbstractFileInfoPointer FileViewItem::fileInfo() const
{
    return d->fileinfo;
}

QVariant FileViewItem::data(int role) const
{
    if (d->fileinfo.isNull())
        return QVariant();

    switch (role) {
    case kItemFileLastModifiedRole:
        return d->fileinfo->lastModified().toString(FileUtils::dateTimeFormat());
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
    case Qt::TextAlignmentRole:
        return Qt::AlignVCenter;
    case kItemFileIconModelToolTipRole: {
        const QString filePath = data(kItemFilePathRole).toString();
        const QString stdDocPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DocumentsLocation);
        const QString stdDownPath = QStandardPaths::writableLocation(QStandardPaths::StandardLocation::DownloadLocation);
        if (filePath == stdDocPath || filePath == stdDownPath || filePath == "/data" + stdDocPath || filePath == "/data" + stdDownPath)
            return QString();

        QString strToolTip = data(kItemFileDisplayNameRole).toString();
        return strToolTip;
    }
    default:
        return QVariant();
    }
}

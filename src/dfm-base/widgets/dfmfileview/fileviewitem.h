/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     yanghao<yanghao@uniontech.com>
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

#ifndef FILEVIEWITEM_H
#define FILEVIEWITEM_H

#include "dfm-base/localfile/localfileinfo.h"
#include "dfm-base/shutil/mimedatabase.h"
#include "dfm-base/base/schemefactory.h"

#include <QStandardItem>
#include <QMetaType>

class FileViewItemPrivate;
class FileViewItem: public QStandardItem
{
    Q_DECLARE_PRIVATE(FileViewItem)
    QSharedPointer<FileViewItemPrivate> d;
public:
    enum Roles {
        ItemNameRole = Qt::DisplayRole,
        ItemIconRole = Qt::DecorationRole,
        ItemEditRole = Qt::EditRole,
        ItemToolTipRole = Qt::ToolTipRole,
        ItemSizeHintRole = Qt::SizeHintRole,
        ItemBackgroundRole = Qt::BackgroundRole,
        ItemForegroundRole = Qt::ForegroundRole,
        ItemCheckStateRole = Qt::CheckStateRole,
        ItemInitialSortOrderRole = Qt::InitialSortOrderRole,
        ItemFontRole = Qt::FontRole,
        ItemTextAlignmentRole = Qt::TextAlignmentRole,
        ItemColorRole = Qt::TextColorRole,
        ItemUrlRole = Qt::UserRole + 1,
        ItemFileLastModifiedRole = Qt::UserRole + 2,
        ItemFileSizeRole = Qt::UserRole + 3,
        ItemFileMimeTypeRole = Qt::UserRole + 4,
        ItemFileSuffixRole = Qt::UserRole + 5,
        ItemFilePathRole = Qt::UserRole + 6,
        ItemColumListRole = Qt::UserRole + 7,
        ItemColumWidthScaleListRole = Qt::UserRole + 8,
    };

    explicit FileViewItem();
    explicit FileViewItem(const QUrl &url);
    FileViewItem &operator=(const FileViewItem &other);
    void refresh();
    QUrl url() const;
    void setUrl(const QUrl url);
    AbstractFileInfoPointer fileinfo() const;
    QMimeType mimeType() const;
    // QStandardItem interface
    virtual QVariant data(int role) const override;
};

Q_DECLARE_METATYPE(FileViewItem);

#endif // DFMFILEVIEWITEM_H

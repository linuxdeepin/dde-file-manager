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

#include "localfile/localfileinfo.h"
#include "shutil/mimedatabase.h"
#include "base/schemefactory.h"

#include <QStandardItem>
#include <QMetaType>

class FileViewItem: public QStandardItem
{
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

    FileViewItem();

    FileViewItem(const QUrl &url);

    FileViewItem &operator=(const FileViewItem &other)
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

    void refresh();

    QUrl url() const;

    void setUrl(const QUrl url);

    template<class T>
    QSharedPointer<T> fileinfo() const
    {
        return qSharedPointerDynamicCast<T>(m_fileinfo);
    }

    QMimeType mimeType() const;

private:
    AbstractFileInfoPointer m_fileinfo;
    QMimeType m_mimeType;

    // QStandardItem interface
public:
    virtual QVariant data(int role) const override;
};

Q_DECLARE_METATYPE(FileViewItem);

#endif // DFMFILEVIEWITEM_H

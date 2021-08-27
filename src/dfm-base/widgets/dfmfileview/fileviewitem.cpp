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
#include "fileviewitem.h"
#include "shutil/fileutils.h"

DFMFileViewItem::DFMFileViewItem()
{

}

DFMFileViewItem::DFMFileViewItem(const QUrl &url)
{
    setUrl(url);
}

void DFMFileViewItem::refresh(){
    if(!m_fileinfo.isNull())
        m_fileinfo->refresh();
    m_mimeType = DMimeDatabase::mimeTypeForUrl(m_fileinfo->url());
}

QUrl DFMFileViewItem::url() const
{
    return QStandardItem::data(Roles::ItemUrlRole).toUrl();
}

void DFMFileViewItem::setUrl(const QUrl url)
{
    setData(QVariant(url),Roles::ItemUrlRole);
    m_fileinfo = DFMInfoFactory::instance().create<DAbstractFileInfo>(url);
    m_mimeType = DMimeDatabase::mimeTypeForUrl(url);

    setData(QVariant(QIcon::fromTheme(m_mimeType.iconName())),ItemIconRole);
    setData(QVariant(m_fileinfo->fileName()),ItemNameRole);
}

QMimeType DFMFileViewItem::mimeType() const
{
    return m_mimeType;
}


QVariant DFMFileViewItem::data(int role) const
{
    if (m_fileinfo.isNull())
        return QVariant();

    switch (role) {
    case ItemFileLastModifiedRole:
        return m_fileinfo->lastModified().toString("yyyy/MM/dd HH:mm:ss");
    case ItemIconRole:
        return QStandardItem::data(Roles::ItemIconRole);
    case ItemFileSizeRole:
        if (m_fileinfo->isDir()) {
            int size = qSharedPointerDynamicCast<DFMLocalFileInfo>(m_fileinfo)->countChildFile();
            if (size <= 1) {
                return QObject::tr("%1 item").arg(size);
            } else {
                return QObject::tr("%1 items").arg(size);
            }
        }
        else
            return FileUtils::formatSize(m_fileinfo->size());
    case ItemFileMimeTypeRole:
        return mimeType().name();
    case ItemColumListRole:
    {
        QList<int> columrollist;
        columrollist << ItemNameRole << ItemFileLastModifiedRole << ItemFileSizeRole << ItemFileMimeTypeRole;

        return QVariant::fromValue<QList<int>>(columrollist);
    }
    case  ItemFileSuffixRole:
        return m_fileinfo->completeSuffix();
    case ItemColumWidthScaleListRole:
    {
        QList<int> columrolwidthlist;
        columrolwidthlist << 2 << 2 << 1 << 1;
        return QVariant::fromValue<QList<int>>(columrolwidthlist);
    }
    case ItemNameRole:
        return m_fileinfo->fileName();
    case ItemSizeHintRole:
        return QSize(-1, 26);
    default:
        return QVariant();
        //        return QStandardItem::data(role);
    }
}

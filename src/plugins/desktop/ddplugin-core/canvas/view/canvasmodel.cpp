/*
 * Copyright (C) 2021 ~ 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     liqiang<liqianga@uniontech.com>
 *
 * Maintainer: liqiang<liqianga@uniontech.com>
 *             wangchunlin<wangchunlin@uniontech.com>
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
#include "canvasmodel.h"
#include "filetreater.h"
#include "dfm-base/interfaces/abstractfileinfo.h"

#include <QDateTime>
#include <QMimeData>

DFMBASE_USE_NAMESPACE
DSB_D_BEGIN_NAMESPACE

CanvasModel::CanvasModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    connection();
}

QModelIndex CanvasModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    if (row < 0 || column < 0 || FileTreaterCt->fileCount() <= row) {
        return QModelIndex();
    }
    auto fileInfo = FileTreaterCt->fileInfo(row);
    if (!fileInfo) {
        return QModelIndex();
    }

    return createIndex(row, column, fileInfo.data());
}

QModelIndex CanvasModel::index(const QString &fileUrl, int column)
{
    if (fileUrl.isEmpty())
        return QModelIndex();

    auto fileInfo = FileTreaterCt->fileInfo(fileUrl);

    return index(fileInfo, column);
}

QModelIndex CanvasModel::index(const DFMLocalFileInfoPointer &fileInfo, int column) const
{
    if (!fileInfo)
        return QModelIndex();

    int row = (0 < FileTreaterCt->fileCount()) ? FileTreaterCt->indexOfChild(fileInfo) : 0;
    return createIndex(row, column, const_cast<LocalFileInfo *>(fileInfo.data()));
}

QModelIndex CanvasModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    return QModelIndex();
}

int CanvasModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 0;

    return FileTreaterCt->fileCount();
}

int CanvasModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QVariant CanvasModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.model() != this) {
        return QVariant();
    }

    auto indexFileInfo = static_cast<LocalFileInfo *>(index.internalPointer());
    if (!indexFileInfo) {
        return QVariant();
    }
    switch (role) {
    case FileTreater::kFileIconRole:
        return indexFileInfo->fileIcon();
    case Qt::EditRole:
    case Qt::DisplayRole:
    case FileTreater::kFileNameRole:
    case FileTreater::kFileDisplayNameRole:
        return indexFileInfo->fileName();
    case FileTreater::kFileLastModifiedRole:
        return indexFileInfo->lastModified().toString();    // todo by file info: lastModifiedDisplayName
    case FileTreater::kFileSizeRole:
        return indexFileInfo->size();   // todo by file info: sizeDisplayName
    case FileTreater::kFileMimeTypeRole:
        return indexFileInfo->fileMimeType().name();    // todo by file info: mimeTypeDisplayName
    case FileTreater::kExtraProperties:
        return indexFileInfo->extraProperties();
    case FileTreater::kFileSuffixRole:
        return indexFileInfo->suffix();
    case FileTreater::kFileNameOfRenameRole:
        return "";  // todo by file info: fileNameOfRename
    case FileTreater::kFileBaseNameOfRenameRole:
        return "";  // todo by file info: baseNameOfRename
    case FileTreater::kFileSuffixOfRenameRole:
        return "";  // todo by file info: suffixOfRename
    default:
        return QString();
    }
}

Qt::ItemFlags CanvasModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = QAbstractItemModel::flags(index);
    if (!index.isValid())
        return flags;

    flags |= Qt::ItemIsDragEnabled;
    flags |= Qt::ItemIsDropEnabled; // todo
    return flags;
}

bool CanvasModel::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return FileTreaterCt->canRefresh();
}

void CanvasModel::fetchMore(const QModelIndex &parent)
{
    Q_UNUSED(parent)

    FileTreaterCt->refresh();
}

bool CanvasModel::isRefreshed() const
{
    return FileTreaterCt->isRefreshed();
}

QUrl CanvasModel::rootUrl() const
{
    return FileTreaterCt->rootUrl();
}

QUrl CanvasModel::url(const QModelIndex &index) const
{
    if (!index.isValid())
        return FileTreaterCt->rootUrl();

    if (auto info = FileTreaterCt->fileInfo(index.row())) {
        return info->url();
    }

    return QUrl();
}

QMimeData *CanvasModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *data = new QMimeData();
    QList<QUrl> urls;

    for (const QModelIndex &idx : indexes)
        urls << url(idx);

    data->setUrls(urls);
    return data;
}

bool CanvasModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_UNUSED(row);
    Q_UNUSED(column);
    if (!parent.isValid()) {
        // copy file to desktop
        qDebug() << "copy file to desktop" << data->urls() << action;
        return true;
    } else {
        auto item = url(parent);
        qDebug() << "drop on item" << item << data->urls() << action;
        return true;
    }

    return true;
}

QStringList CanvasModel::mimeTypes() const
{
    static QStringList types {QLatin1String("text/uri-list")};
    return types;
}

Qt::DropActions CanvasModel::supportedDragActions() const
{
    // todo
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

Qt::DropActions CanvasModel::supportedDropActions() const
{
    // todo
    return Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
}

bool CanvasModel::enableSort() const
{
    return FileTreaterCt->enableSort();
}

void CanvasModel::setEnabledSort(bool enabledSort)
{
    FileTreaterCt->setEnabledSort(enabledSort);
}

void CanvasModel::sort(int column, Qt::SortOrder order)
{
    Q_UNUSED(column)
    Q_UNUSED(order)

    sort();
}

bool CanvasModel::sort()
{
    return FileTreaterCt->sort();
}

Qt::SortOrder CanvasModel::sortOrder() const
{
    return FileTreaterCt->sortOrder();
}

void CanvasModel::setSortOrder(const Qt::SortOrder &order)
{
    FileTreaterCt->setSortOrder(order);
}

int CanvasModel::sortRole() const
{
    return FileTreaterCt->sortRole();
}

void CanvasModel::setSortRole(int role, Qt::SortOrder order)
{
    FileTreaterCt->setSortRole(role, order);
}

void CanvasModel::connection()
{
    connect(FileTreaterCt, &FileTreater::fileCreated, this, &CanvasModel::fileCreated);
    connect(FileTreaterCt, &FileTreater::fileDeleted, this, &CanvasModel::fileDeleted);
    connect(FileTreaterCt, &FileTreater::fileRenamed, this, &CanvasModel::fileRenamed);
    connect(FileTreaterCt, &FileTreater::fileRefreshed, this, &CanvasModel::fileRefreshed);
    connect(FileTreaterCt, &FileTreater::enableSortChanged, this, &CanvasModel::enableSortChanged);
}

DSB_D_END_NAMESPACE
